//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "controller.h"
#include "process.h"

#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iasync.h"
#include "vstgui/standalone/include/icommondirectories.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

#include <cassert>
#include <fstream>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
namespace {

#if WINDOWS
constexpr auto PlatformPathDelimiter = '\\';
constexpr auto EnvPathSeparator = ';';
constexpr auto CMakeExecutableName = "CMake.exe";
#else
constexpr auto PlatformPathDelimiter = '/';
constexpr auto EnvPathSeparator = ':';
constexpr auto CMakeExecutableName = "cmake";
#endif

//------------------------------------------------------------------------
constexpr auto CMakeWebPageURL = "https://cmake.org";
constexpr auto SteinbergSDKWebPageURL = "https://www.steinberg.net/en/company/developers.html";
constexpr auto GitHubSDKWebPageURL = "https://github.com/steinbergmedia/vst3sdk";

//------------------------------------------------------------------------
constexpr auto valueIdWelcomeDownloadSDK = "Welcome Download SDK";
constexpr auto valueIdWelcomeLocateSDK = "Welcome Locate SDK";
constexpr auto valueIdWelcomeDownloadCMake = "Welcome Download CMake";
constexpr auto valueIdWelcomeLocateCMake = "Welcome Locate CMake";
constexpr auto valueIdValidVSTSDKPath = "Valid VST SDK Path";
constexpr auto valueIdValidCMakePath = "Valid CMake Path";

//------------------------------------------------------------------------
void showSimpleAlert (const char* headline, const char* description)
{
	AlertBoxForWindowConfig config;
	config.headline = headline;
	config.description = description;
	config.defaultButton = "OK";
	config.window = IApplication::instance ().getWindows ().front ();
	IApplication::instance ().showAlertBoxForWindow (config);
}

//------------------------------------------------------------------------
void setPreferenceStringValue (Preferences& prefs, const UTF8String& key, const ValuePtr& value)
{
	if (!value)
		return;
	if (auto strValue = value->dynamicCast<IStringValue> ())
		prefs.set (key, strValue->getString ());
	else
		prefs.set (key, value->getConverter ().valueAsString (value->getValue ()));
}

//------------------------------------------------------------------------
UTF8String getModelValueString (VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model,
                                const UTF8String& key)
{
	auto value = model->getValue (key);
	if (auto strValue = value->dynamicCast<IStringValue> ())
	{
		return strValue->getString ();
	}

	return {};
}

class ValueListenerViewController : public DelegationController, public ValueListenerAdapter
{
public:
	ValueListenerViewController (IController* parent, ValuePtr value)
	: DelegationController (parent), value (value)
	{
		value->registerListener (this);
	}

	virtual ~ValueListenerViewController () noexcept { value->unregisterListener (this); }

	const ValuePtr& getValue () const { return value; }

private:
	ValuePtr value {nullptr};
};

//------------------------------------------------------------------------
class ScriptScrollViewController : public ValueListenerViewController
{
public:
	ScriptScrollViewController (IController* parent, ValuePtr value)
	: ValueListenerViewController (parent, value)
	{
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		if (auto sv = dynamic_cast<CScrollView*> (view))
			scrollView = sv;
		return controller->verifyView (view, attributes, description);
	}

	void onEndEdit (IValue&) override
	{
		if (!scrollView)
			return;
		auto containerSize = scrollView->getContainerSize ();
		containerSize.top = containerSize.bottom - 10;
		scrollView->makeRectVisible (containerSize);
	}

	CScrollView* scrollView {nullptr};
};

//------------------------------------------------------------------------
class DimmViewController : public ValueListenerViewController
{
public:
	DimmViewController (IController* parent, ValuePtr value)
	: ValueListenerViewController (parent, value)
	{
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		if (auto name = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*name == "Container")
			{
				dimmView = view;
				onEndEdit (*getValue ());
			}
		}
		return controller->verifyView (view, attributes, description);
	}

	void onEndEdit (IValue& value) override
	{
		if (!dimmView)
			return;
		bool b = value.getValue () > 0.5;
		float alphaValue = b ? 0.f : 1.f;
		dimmView->setAlphaValue (alphaValue);
		dimmView->setMouseEnabled (!b);
	}

	CView* dimmView {nullptr};
};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
void Controller::onSetContentView (IWindow& window, const VSTGUI::SharedPointer<CFrame>& view)
{
	contentView = view;
}

//------------------------------------------------------------------------
Controller::Controller ()
{
	Preferences prefs;
	auto vendorPref = prefs.get (valueIdVendor);
	auto emailPref = prefs.get (valueIdEMail);
	auto urlPref = prefs.get (valueIdURL);
	auto vstSdkPathPref = prefs.get (valueIdVSTSDKPath);
	auto cmakePathPref = prefs.get (valueIdCMakePath);
	auto pluginPathPref = prefs.get (valueIdPluginPath);

	auto envPaths = getEnvPaths ();
	if (!cmakePathPref || cmakePathPref->empty ())
		cmakePathPref = findCMakePath (envPaths);

	model = UIDesc::ModelBindingCallbacks::make ();
	/* UI only */
	model->addValue (Value::makeStringListValue (
	    valueIdTabBar, {"Welcome", "Create Plug-In Project", "Preferences"}));

	model->addValue (Value::make (valueIdCreateProject),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 createProject ();
		                 v.performEdit (0.);
	                 }));

	model->addValue (Value::makeStringValue (valueIdScriptOutput, ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue& v) { onScriptOutput (); }));
	model->addValue (Value::make (valueIdScriptRunning),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue& v) {
		                 onScriptRunning (v.getValue () > 0.5 ? true : false);
	                 }));

	/* Factory Infos */
	model->addValue (Value::makeStringValue (valueIdVendor, vendorPref ? *vendorPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdEMail, emailPref ? *emailPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdURL, urlPref ? *urlPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

	/* Directories */
	model->addValue (Value::make (valueIdChooseVSTSDKPath),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 chooseVSTSDKPath ();
		                 v.performEdit (0.);
	                 }));
	model->addValue (Value::make (valueIdChooseCMakePath),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 chooseCMakePath ();
		                 v.performEdit (0.);
	                 }));
	model->addValue (
	    Value::makeStringValue (valueIdVSTSDKPath, vstSdkPathPref ? *vstSdkPathPref : ""),
	    UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdCMakePath, cmakePathPref ? *cmakePathPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

	/* Plug-In */
	model->addValue (Value::makeStringValue (valueIdPluginName, ""));
	model->addValue (
	    Value::makeStringListValue (valueIdPluginType, {"Audio Effect", "Instrument"}));
	model->addValue (Value::makeStringValue (valueIdPluginBundleID, ""));
	model->addValue (Value::makeStringValue (valueIdPluginFilenamePrefix, ""));
	model->addValue (
	    Value::makeStringValue (valueIdPluginPath, pluginPathPref ? *pluginPathPref : ""),
	    UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

	model->addValue (Value::make (valueIdChoosePluginPath),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 choosePluginPath ();
		                 v.performEdit (0.);
	                 }));

	/* CMake */
	model->addValue (Value::makeStringListValue (valueIdCMakeGenerators, {"", ""}),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

	/* Welcome Page */
	model->addValue (Value::make (valueIdWelcomeDownloadSDK),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 downloadVSTSDK ();
		                 v.performEdit (0.);
	                 }));
	model->addValue (Value::make (valueIdWelcomeLocateSDK),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 chooseVSTSDKPath ();
		                 v.performEdit (0.);
	                 }));
	model->addValue (Value::make (valueIdWelcomeDownloadCMake),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 downloadCMake ();
		                 v.performEdit (0.);
	                 }));
	model->addValue (Value::make (valueIdWelcomeLocateCMake),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 chooseCMakePath ();
		                 verifyCMakeInstallation ();
		                 v.performEdit (0.);
	                 }));

	/* Valid Path values */
	model->addValue (Value::make (valueIdValidVSTSDKPath));
	model->addValue (Value::make (valueIdValidCMakePath));

	// sub controllers
	addCreateViewControllerFunc (
	    "ScriptOutputController",
	    [this] (const auto& name, auto parent, const auto uiDesc) -> IController* {
		    return new ScriptScrollViewController (parent, model->getValue (valueIdScriptOutput));
	    });
	addCreateViewControllerFunc (
	    "DimmViewController_CMake",
	    [this] (const auto& name, auto parent, const auto uiDesc) -> IController* {
		    return new DimmViewController (parent, model->getValue (valueIdValidCMakePath));
	    });
	addCreateViewControllerFunc (
	    "DimmViewController_VSTSDK",
	    [this] (const auto& name, auto parent, const auto uiDesc) -> IController* {
		    return new DimmViewController (parent, model->getValue (valueIdValidVSTSDKPath));
	    });
}

//------------------------------------------------------------------------
void Controller::storePreferences ()
{
	Preferences prefs;
	setPreferenceStringValue (prefs, valueIdVendor, model->getValue (valueIdVendor));
	setPreferenceStringValue (prefs, valueIdEMail, model->getValue (valueIdEMail));
	setPreferenceStringValue (prefs, valueIdURL, model->getValue (valueIdURL));
	setPreferenceStringValue (prefs, valueIdVSTSDKPath, model->getValue (valueIdVSTSDKPath));
	setPreferenceStringValue (prefs, valueIdCMakePath, model->getValue (valueIdCMakePath));
	setPreferenceStringValue (prefs, valueIdPluginPath, model->getValue (valueIdPluginPath));
	setPreferenceStringValue (prefs, valueIdCMakeGenerators,
	                          model->getValue (valueIdCMakeGenerators));
}

//------------------------------------------------------------------------
void Controller::onScriptRunning (bool state)
{
	static constexpr auto valuesToDisable = {
	    valueIdTabBar,
	    valueIdVendor,
	    valueIdEMail,
	    valueIdURL,
	    valueIdVSTSDKPath,
	    valueIdCMakePath,
	    valueIdPluginType,
	    valueIdPluginPath,
	    valueIdPluginName,
	    valueIdPluginBundleID,
	    valueIdPluginFilenamePrefix,
	    valueIdChooseCMakePath,
	    valueIdChooseVSTSDKPath,
	    valueIdChoosePluginPath,
	    valueIdCreateProject,
	    valueIdCMakeGenerators,
	};
	for (const auto& valueID : valuesToDisable)
	{
		if (auto value = model->getValue (valueID))
			value->setActive (!state);
	}
}

//------------------------------------------------------------------------
void Controller::onShow (const IWindow& window)
{
	bool sdkInstallationVerified = verifySDKInstallation ();
	bool cmakeInstallationVerified = verifyCMakeInstallation ();
	Value::performSinglePlainEdit (*model->getValue (valueIdTabBar),
	                               sdkInstallationVerified && cmakeInstallationVerified ? 1 : 0);

	if (cmakeInstallationVerified)
		gatherCMakeInformation ();
}

//------------------------------------------------------------------------
void Controller::gatherCMakeInformation ()
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	if (auto process = Process::create (cmakePathStr.getString ()))
	{
		Process::ArgumentList args;
		args.add ("-E");
		args.add ("capabilities");

		auto scriptRunningValue = model->getValue (valueIdScriptRunning);
		assert (scriptRunningValue);
		Value::performSingleEdit (*scriptRunningValue, 1.);
		auto outputString = std::make_shared<std::string> ();
		auto result = process->run (args, [this, scriptRunningValue, outputString,
		                                   process] (Process::CallbackParams& p) mutable {
			if (!p.buffer.empty ())
			{
				*outputString += std::string (p.buffer.data (), p.buffer.size ());
			}
			if (p.isEOF)
			{
				if (auto capabilities = parseCMakeCapabilities (*outputString))
				{
					auto cmakeGeneratorsValue = model->getValue (valueIdCMakeGenerators);
					assert (cmakeGeneratorsValue);
					cmakeGeneratorsValue->dynamicCast<IStringListValue> ()->updateStringList (
					    capabilities->generators);

					Preferences prefs;
					if (auto generatorPref = prefs.get (valueIdCMakeGenerators))
					{
						auto value =
						    cmakeGeneratorsValue->getConverter ().stringAsValue (*generatorPref);
						cmakeGeneratorsValue->performEdit (value);
					}
					else
					{
						// we should use some defaults here
					}
					cmakeCapabilities = std::move (*capabilities);
				}
				else
				{
					// TODO: show error?
				}
				Value::performSingleEdit (*scriptRunningValue, 0.);
				process.reset ();
			}
		});
		if (!result)
		{
			// TODO: show error!
		}
	}
}

//------------------------------------------------------------------------
template <typename Proc>
void Controller::runFileSelector (const UTF8String& valueId, CNewFileSelector::Style style,
                                  Proc proc) const
{
	auto value = model->getValue (valueId);
	if (!value)
		return;

	auto fileSelector = owned (CNewFileSelector::create (contentView, style));
	if (!fileSelector)
		return;

	Preferences prefs;
	if (auto pathPref = prefs.get (valueId))
		fileSelector->setInitialDirectory (*pathPref);

	fileSelector->run ([proc, value] (CNewFileSelector* fs) {
		if (fs->getNumSelectedFiles () == 0)
			return;
		if (proc (fs->getSelectedFile (0)))
			Value::performStringValueEdit (*value, fs->getSelectedFile (0));
	});
}

//------------------------------------------------------------------------
void Controller::chooseVSTSDKPath ()
{
	runFileSelector (
	    valueIdVSTSDKPath, CNewFileSelector::kSelectDirectory, [this] (const UTF8String& path) {
		    if (!validateVSTSDKPath (path))
		    {
			    showSimpleAlert (
			        "Wrong VST SDK path!",
			        "The selected folder does not look like the root folder of the VST SDK.");
			    return false;
		    }
		    Async::schedule (Async::mainQueue (), [this] () { verifySDKInstallation (); });
		    return true;
	    });
}

//------------------------------------------------------------------------
void Controller::chooseCMakePath ()
{
	runFileSelector (
	    valueIdCMakePath, CNewFileSelector::kSelectFile, [this] (const UTF8String& path) {
		    if (!validateCMakePath (path))
		    {
			    showSimpleAlert ("Wrong CMake path!", "The selected file is not cmake.");
			    return false;
		    }
		    Async::schedule (Async::mainQueue (), [this] () {
			    verifyCMakeInstallation ();
			    gatherCMakeInformation ();
		    });
		    return true;
	    });
}

//------------------------------------------------------------------------
void Controller::choosePluginPath ()
{
	runFileSelector (valueIdPluginPath, CNewFileSelector::kSelectDirectory,
	                 [this] (const UTF8String& path) { return validatePluginPath (path); });
}

//------------------------------------------------------------------------
void Controller::downloadVSTSDK ()
{
	AlertBoxForWindowConfig alert;
	alert.window = IApplication::instance ().getWindows ().front ();
	alert.headline = "Which SDK to download?";
	alert.description = "TODO: description";
	alert.defaultButton = "Commercial";
	alert.secondButton = "Open Source";
	alert.thirdButton = "Cancel";
	alert.callback = [] (AlertResult result) {
		switch (result)
		{
			case AlertResult::DefaultButton:
			{
				openURL (SteinbergSDKWebPageURL);
				break;
			}
			case AlertResult::SecondButton:
			{
				openURL (GitHubSDKWebPageURL);
				break;
			}
			case AlertResult::ThirdButton:
			{
				// Canceled
				break;
			}
			default:
			{
				assert (false);
				break;
			}
		}
	};
	IApplication::instance ().showAlertBoxForWindow (alert);
}

//------------------------------------------------------------------------
void Controller::downloadCMake ()
{
	openURL (CMakeWebPageURL);
}

//------------------------------------------------------------------------
bool Controller::verifySDKInstallation ()
{
	auto sdkPathStr = getModelValueString (model, valueIdVSTSDKPath);
	auto result = !(sdkPathStr.empty () || !validateVSTSDKPath (sdkPathStr));
	Value::performSinglePlainEdit (*model->getValue (valueIdValidVSTSDKPath), result);
	return result;
}

//------------------------------------------------------------------------
bool Controller::verifyCMakeInstallation ()
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	auto result = !(cmakePathStr.empty () || !validateCMakePath (cmakePathStr));
	Value::performSinglePlainEdit (*model->getValue (valueIdValidCMakePath), result);
	return result;
}

//------------------------------------------------------------------------
void Controller::showCMakeNotInstalledWarning ()
{
	AlertBoxForWindowConfig config;
	config.headline = "CMake not found!";
	config.description = "You need to install CMake for your platform to use this application.";
	config.defaultButton = "OK";
	config.secondButton = "Download CMake";
	config.window = IApplication::instance ().getWindows ().front ();
	config.callback = [this] (AlertResult result) {
		if (result == AlertResult::SecondButton)
			downloadCMake ();
	};
	IApplication::instance ().showAlertBoxForWindow (config);
}

//------------------------------------------------------------------------
bool Controller::validateVSTSDKPath (const UTF8String& path)
{
	auto p = path.getString ();
	if (*p.rbegin () != PlatformPathDelimiter)
		p += PlatformPathDelimiter;
	p += "pluginterfaces";
	p += PlatformPathDelimiter;
	p += "vst";
	p += PlatformPathDelimiter;
	p += "vsttypes.h";
	std::ifstream stream (p);
	return stream.is_open ();
}

//------------------------------------------------------------------------
bool Controller::validateCMakePath (const UTF8String& path)
{
	std::ifstream stream (path.getString ());
	return stream.is_open ();
}

//------------------------------------------------------------------------
bool Controller::validatePluginPath (const UTF8String& path)
{
	// TODO: check that the path is valid
	return true;
}

//------------------------------------------------------------------------
void Controller::createProject ()
{
	if (cmakeCapabilities.versionMajor == 0)
	{
		showCMakeNotInstalledWarning ();
		return;
	}
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	auto _sdkPathStr = getModelValueString (model, valueIdVSTSDKPath);
	auto pluginOutputPathStr = getModelValueString (model, valueIdPluginPath);
	auto vendorStr = getModelValueString (model, valueIdVendor);
	auto emailStr = getModelValueString (model, valueIdEMail);
	auto pluginNameStr = getModelValueString (model, valueIdPluginName);
	auto filenamePrefixStr = getModelValueString (model, valueIdPluginFilenamePrefix);
	auto pluginBundleIDStr = getModelValueString (model, valueIdPluginBundleID);

	if (_sdkPathStr.empty () || !validateVSTSDKPath (_sdkPathStr))
	{
		showSimpleAlert ("Cannot create Project", "The VST3 SDK Path is not correct.");
		return;
	}
	auto sdkPathStr = _sdkPathStr.getString ();
	unixfyPath (sdkPathStr);
	if (pluginOutputPathStr.empty ())
	{
		showSimpleAlert ("Cannot create Project", "You need to specify an output directory.");
		return;
	}
	if (pluginNameStr.empty ())
	{
		showSimpleAlert ("Cannot create Project", "You need to specify a name for your plugin.");
		return;
	}
	if (pluginBundleIDStr.empty ())
	{
		showSimpleAlert ("Cannot create Project", "You need to specify a bundle ID.");
		return;
	}

	if (auto scriptPath = IApplication::instance ().getCommonDirectories ().get (
	        CommonDirectoryLocation::AppResourcesPath))
	{
		*scriptPath += "GenerateVST3Plugin.cmake";

		Process::ArgumentList args;
		args.add ("-DSMTG_VST3_SDK_SOURCE_DIR_CLI=\"" + sdkPathStr + "\"");
		args.add ("-DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI=\"" + pluginOutputPathStr.getString () +
		          "\"");
		args.add ("-DSMTG_PLUGIN_NAME_CLI=\"" + pluginNameStr.getString () + "\"");
		args.add ("-DSMTG_PLUGIN_IDENTIFIER_CLI=\"" + pluginBundleIDStr.getString () + "\"");
		args.add ("-DSMTG_VENDOR_NAME_CLI=\"" + vendorStr.getString () + "\"");
		args.add ("-DSMTG_VENDOR_EMAIL_CLI=\"" + emailStr.getString () + "\"");
		args.add ("-DSMTG_PREFIX_FOR_FILENAMES_CLI=\"" + filenamePrefixStr.getString () + "\"");
		args.add ("-P");
		args.add (scriptPath->getString ());

		if (auto process = Process::create (cmakePathStr.getString ()))
		{
			auto scriptRunningValue = model->getValue (valueIdScriptRunning);
			assert (scriptRunningValue);
			Value::performSingleEdit (*scriptRunningValue, 1.);
			auto scriptOutputValue = model->getValue (valueIdScriptOutput);
			assert (scriptOutputValue);
			Value::performStringValueEdit (*scriptOutputValue, "");

			auto projectPath = pluginOutputPathStr + PlatformPathDelimiter + pluginNameStr;
			if (!process->run (args, [this, scriptRunningValue, scriptOutputValue, process,
			                          projectPath] (Process::CallbackParams& p) mutable {
				    if (!p.buffer.empty ())
				    {
					    Value::performStringAppendValueEdit (
					        *scriptOutputValue, std::string (p.buffer.data (), p.buffer.size ()));
				    }
				    if (p.isEOF)
				    {
					    assert (scriptRunningValue);
					    Value::performSingleEdit (*scriptRunningValue, 0.);
					    if (p.resultCode == 0)
						    runProjectCMake (projectPath);
					    process.reset ();
				    }
			    }))
			{
				showSimpleAlert ("Could not execute CMake", "Please verify your path to CMake!");
				assert (scriptRunningValue);
				Value::performSingleEdit (*scriptRunningValue, 0.);
			}
		}
	}
}

//------------------------------------------------------------------------
void Controller::runProjectCMake (const UTF8String& path)
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	auto value = model->getValue (valueIdCMakeGenerators);
	assert (value);
	if (!value)
		return;
	auto generator = value->getConverter ().valueAsString (value->getValue ());
	if (generator.getString ().find (' ') != std::string::npos)
	{
		generator = "\"" + generator + "\"";
	}
	if (auto process = Process::create (cmakePathStr.getString ()))
	{
		auto scriptRunningValue = model->getValue (valueIdScriptRunning);
		assert (scriptRunningValue);
		Value::performSingleEdit (*scriptRunningValue, 1.);
		auto scriptOutputValue = model->getValue (valueIdScriptOutput);

		Process::ArgumentList args;
		args.add ("-G" + generator.getString ());
		args.add ("-S");
		args.addPath (path.getString ());
		args.add ("-B");
		auto buildDir = path;
		buildDir += PlatformPathDelimiter;
		buildDir += "build";
		args.addPath (buildDir.getString ());

		Value::performStringAppendValueEdit (*scriptOutputValue, "\n" + cmakePathStr + " ");
		for (const auto& a : args.args)
			Value::performStringAppendValueEdit (*scriptOutputValue, UTF8String (a) + " ");
		Value::performStringAppendValueEdit (*scriptOutputValue, "\n");

		auto result = process->run (args, [this, scriptRunningValue, scriptOutputValue, buildDir,
		                                   process] (Process::CallbackParams& p) mutable {
			if (!p.buffer.empty ())
			{
				Value::performStringAppendValueEdit (
				    *scriptOutputValue, std::string (p.buffer.data (), p.buffer.size ()));
			}
			if (p.isEOF)
			{
				assert (scriptRunningValue);
				Value::performSingleEdit (*scriptRunningValue, 0.);
				if (p.resultCode == 0)
					openCMakeGeneratedProject (buildDir);
				process.reset ();
			}
		});
		if (!result)
		{
			// TODO: Show error
		}
	}
}

//------------------------------------------------------------------------
void Controller::openCMakeGeneratedProject (const UTF8String& path)
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	if (auto process = Process::create (cmakePathStr.getString ()))
	{
		auto scriptOutputValue = model->getValue (valueIdScriptOutput);
		Process::ArgumentList args;
		args.add ("--open");
		args.addPath (path.getString ());
		auto result = process->run (
		    args, [this, scriptOutputValue, process] (Process::CallbackParams& p) mutable {
			    if (!p.buffer.empty ())
			    {
				    Value::performStringAppendValueEdit (
				        *scriptOutputValue, std::string (p.buffer.data (), p.buffer.size ()));
			    }
			    if (p.isEOF)
			    {
				    process.reset ();
			    }
		    });
		if (!result)
		{
			// TODO: Show error
		}
	}
}

//------------------------------------------------------------------------
void Controller::onScriptOutput ()
{
}

//------------------------------------------------------------------------
auto Controller::getEnvPaths () -> StringList
{
	StringList result;
	if (auto envPath = std::getenv ("PATH"))
	{
		std::istringstream input;
		input.str (envPath);
		std::string el;
		while (std::getline (input, el, EnvPathSeparator))
		{
			if (*el.rbegin () != PlatformPathDelimiter)
				el += PlatformPathDelimiter;
			result.emplace_back (std::move (el));
		}
	}
	return result;
}

//------------------------------------------------------------------------
VSTGUI::Optional<UTF8String> Controller::findCMakePath (const StringList& envPaths)
{
	if (!envPaths.empty ())
	{
		for (auto path : envPaths)
		{
			path += CMakeExecutableName;
			std::ifstream stream (path);
			if (stream.is_open ())
			{
				return {std::move (path)};
			}
		}
	}
#if !WINDOWS
	std::string path = "/usr/local/bin/cmake";
	std::ifstream stream (path);
	if (stream.is_open ())
		return {std::move (path)};
#endif
	return {};
}

//------------------------------------------------------------------------
const IMenuBuilder* Controller::getWindowMenuBuilder (const IWindow& window) const
{
	return this;
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
