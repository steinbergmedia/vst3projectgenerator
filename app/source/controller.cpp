//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "controller.h"
#include "dimmviewcontroller.h"
#include "linkcontroller.h"
#include "process.h"
#include "scriptscrollviewcontroller.h"
#include "version.h"

#include "vstgui/lib/controls/ctextedit.h"
#include "vstgui/lib/controls/itexteditlistener.h"

#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iasync.h"
#include "vstgui/standalone/include/icommondirectories.h"
#include "vstgui/uidescription/cstream.h"

#include <array>
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
constexpr auto SteinbergSDKWebPageURL = "https://www.steinberg.net/vst3sdk";
constexpr auto GitHubSDKWebPageURL = "https://github.com/steinbergmedia/vst3sdk";

//------------------------------------------------------------------------
constexpr auto valueIdWelcomeDownloadSDK = "Welcome Download SDK";
constexpr auto valueIdWelcomeLocateSDK = "Welcome Locate SDK";
constexpr auto valueIdWelcomeDownloadCMake = "Welcome Download CMake";
constexpr auto valueIdWelcomeLocateCMake = "Welcome Locate CMake";
constexpr auto valueIdValidVSTSDKPath = "Valid VST SDK Path";
constexpr auto valueIdValidCMakePath = "Valid CMake Path";

//------------------------------------------------------------------------
const std::initializer_list<IStringListValue::StringType> pluginTypeDisplayStrings = {
    "Audio Effect", "Instrument"};
const std::array<std::string, 2> pluginTypeStrings = {"Fx", "Instrument"};

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
size_t makeValidCppName (std::string& str, bool allowColon = false, char replaceChar = '_')
{
	size_t replaced = 0;
	char numericEndVal = allowColon ? 0x3B : 0x3A;
	std::replace_if (str.begin (), str.end (),
	                 [&] (auto c) {
		                 // allowed: 0...9, A...Z and a...z
		                 auto legal = (c >= 0x30 && c < numericEndVal) || (c >= 0x41 && c < 0x5B) ||
		                              (c >= 0x61 && c < 0x7B) || c == replaceChar;
		                 if (!legal)
			                 replaced++;
		                 return !legal;
	                 },
	                 replaceChar);
	return replaced;
}

//------------------------------------------------------------------------
void makeValidCppValueString (IValue& value, bool allowColon = false)
{
	if (auto strValue = value.dynamicCast<IStringValue> ())
	{
		auto str = strValue->getString ().getString ();
		auto replaced = makeValidCppName (str, allowColon);
		if (replaced)
		{
			value.beginEdit ();
			strValue->setString (UTF8String (std::move (str)));
			value.endEdit ();
		}
	}
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
UTF8String getValueString (IValue& value)
{
	if (auto strValue = value.dynamicCast<IStringValue> ())
		return strValue->getString ();
	return {};
}

//------------------------------------------------------------------------
UTF8String getModelValueString (VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model,
                                const UTF8String& key)
{
	if (auto value = model->getValue (key))
		return getValueString (*value);
	return {};
}

//------------------------------------------------------------------------
class SyncProjectAndClassNameController : public ValueListenerViewController,
                                          public TextEditListenerAdapter,
                                          public ViewListenerAdapter
{
public:
	SyncProjectAndClassNameController (IController* parent, ValuePtr value, ValuePtr secondValue)
	: ValueListenerViewController (parent, value), secondValue (secondValue)
	{
		vstgui_assert (value && secondValue);
	}
	~SyncProjectAndClassNameController () noexcept override
	{
		if (textEdit)
			viewWillDelete (textEdit);
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		if (auto te = dynamic_cast<CTextEdit*> (view))
		{
			vstgui_assert (textEdit == nullptr);
			textEdit = te;
			textEdit->registerViewListener (this);
			textEdit->registerTextEditListener (this);
		}
		return ValueListenerViewController::verifyView (view, attributes, description);
	}

	void viewWillDelete (CView* view) override
	{
		vstgui_assert (view == textEdit);
		textEdit->unregisterTextEditListener (this);
		view->unregisterViewListener (this);
		textEdit = nullptr;
	}

	void onTextEditPlatformControlTookFocus (CTextEdit*) override
	{
		syncValues = getValueString (*getSecondValue ()) == getValueString (*getValue ());
	}
	void onTextEditPlatformControlLostFocus (CTextEdit*) override { syncValues = false; }

	void onEndEdit (IValue& value) override
	{
		if (!syncValues)
			return;
		auto strValue = value.dynamicCast<IStringValue> ();
		auto secondStrValue = secondValue->dynamicCast<IStringValue> ();
		if (secondStrValue->getString () == strValue->getString ())
			return;
		secondValue->beginEdit ();
		secondStrValue->setString (strValue->getString ());
		secondValue->endEdit ();
	}

private:
	ValuePtr getSecondValue () const { return secondValue; }
	ValuePtr secondValue;

	CTextEdit* textEdit {nullptr};

	bool syncValues {false};
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
	auto vendorPref = prefs.get (valueIdVendorName);
	auto emailPref = prefs.get (valueIdVendorEMail);
	auto urlPref = prefs.get (valueIdVendorURL);
	auto namespacePref = prefs.get (valueIdVendorNamespace);
	auto vstSdkPathPref = prefs.get (valueIdVSTSDKPath);
	auto cmakePathPref = prefs.get (valueIdCMakePath);
	auto pluginPathPref = prefs.get (valueIdPluginPath);

	auto envPaths = getEnvPaths ();
	if (!cmakePathPref || cmakePathPref->empty ())
		cmakePathPref = findCMakePath (envPaths);

	model = UIDesc::ModelBindingCallbacks::make ();
	/* UI only */
	UTF8String version ("Version ");
	version += MAJOR_VERSION_STR "." SUB_VERSION_STR;
	if (RELEASE_NUMBER_INT > 0)
		version += "." RELEASE_NUMBER_STR;
	model->addValue (Value::makeStringValue (valueIdAppVersion, version));

	model->addValue (Value::makeStringListValue (
	    valueIdTabBar, {"Welcome", "Create Plug-in Project", "Preferences"}));

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

	model->addValue (Value::make (valueIdCopyScriptOutput),
	                 UIDesc::ValueCalls::onAction ([this] (IValue& v) {
		                 copyScriptOutputToClipboard ();
		                 v.performEdit (0.);
	                 }));

	/* Factory/Vendor Infos */
	model->addValue (Value::makeStringValue (valueIdVendorName, vendorPref ? *vendorPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdVendorEMail, emailPref ? *emailPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdVendorURL, urlPref ? *urlPref : ""),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (
	    Value::makeStringValue (valueIdVendorNamespace, namespacePref ? *namespacePref : ""),
	    UIDesc::ValueCalls::onEndEdit ([this] (IValue& val) {
		    makeValidCppValueString (val, true);
		    storePreferences ();
	    }));

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
	model->addValue (Value::makeStringListValue (valueIdPluginType, pluginTypeDisplayStrings));
	model->addValue (Value::makeStringValue (valueIdPluginBundleID, ""));
	model->addValue (Value::makeStringValue (valueIdPluginFilenamePrefix, ""));
	model->addValue (
	    Value::makeStringValue (valueIdPluginClassName, ""),
	    UIDesc::ValueCalls::onEndEdit ([] (IValue& val) { makeValidCppValueString (val, false); }));
	model->addValue (Value::makeStringValue (valueIdMacOSDeploymentTarget, "10.13"),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
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
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) {
		                 storePreferences ();

		                 auto cmakeGeneratorsValue = model->getValue (valueIdCMakeGenerators);
		                 assert (cmakeGeneratorsValue);

		                 auto generatorStr = cmakeGeneratorsValue->getConverter ().valueAsString (
		                     cmakeGeneratorsValue->getValue ());

		                 fillCmakeSupportedPlatforms (generatorStr.getString ());
	                 }));

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

	// Link List
	model->addValue (
	    Value::makeStringListValue (valueIdLinkList, LinkController::instance ().getTitles ()),
	    UIDesc::ValueCalls::onEndEdit ([] (IValue& v) {
		    auto index = v.getConverter ().normalizedToPlain (v.getValue ());
		    const auto& urlList = LinkController::instance ().getUrls ();
		    if (index >= 0 && index < urlList.size ())
		    {
			    openURL (urlList[static_cast<uint32_t> (index)].getString ());
		    }
		    v.performEdit (0.);
	    }));

	/* Valid Path values */
	model->addValue (Value::make (valueIdValidVSTSDKPath));
	model->addValue (Value::make (valueIdValidCMakePath));

	/* Use VSTGUI by default ON */
	model->addValue (Value::make (valueIdUseVSTGUI, 1));

	/* Cmake */
	/* Supported Platforms */
	model->addValue (Value::makeStringListValue (valueIdCMakeSupportedPlatforms, {"", ""}),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

	/* cmake version */
	model->addValue (Value::makeStringValue (valueIdCMakeVersion, "CMake ?.?.?"));

	// HERE add new values when needed (keep the previous order else the uidesc
	// could not find its values!)

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
	addCreateViewControllerFunc (
	    "DimmViewController_CreateProjectTab",
	    [this] (const auto& name, auto parent, const auto uiDesc) -> IController* {
		    return new DimmViewController (parent, model->getValue (valueIdScriptRunning), 0.5f);
	    });
	addCreateViewControllerFunc (
	    "SyncProjectAndClassNameController",
	    [this] (const auto& name, auto parent, const auto uiDesc) -> IController* {
		    return new SyncProjectAndClassNameController (parent,
		                                                  model->getValue (valueIdPluginName),
		                                                  model->getValue (valueIdPluginClassName));
	    });
}

//------------------------------------------------------------------------
void Controller::storePreferences ()
{
	Preferences prefs;
	setPreferenceStringValue (prefs, valueIdVendorName, model->getValue (valueIdVendorName));
	setPreferenceStringValue (prefs, valueIdVendorEMail, model->getValue (valueIdVendorEMail));
	setPreferenceStringValue (prefs, valueIdVendorURL, model->getValue (valueIdVendorURL));
	setPreferenceStringValue (prefs, valueIdVendorNamespace,
	                          model->getValue (valueIdVendorNamespace));
	setPreferenceStringValue (prefs, valueIdVSTSDKPath, model->getValue (valueIdVSTSDKPath));
	setPreferenceStringValue (prefs, valueIdCMakePath, model->getValue (valueIdCMakePath));
	setPreferenceStringValue (prefs, valueIdPluginPath, model->getValue (valueIdPluginPath));
	setPreferenceStringValue (prefs, valueIdCMakeGenerators,
	                          model->getValue (valueIdCMakeGenerators));
	setPreferenceStringValue (prefs, valueIdCMakeSupportedPlatforms,
	                          model->getValue (valueIdCMakeSupportedPlatforms));

	setPreferenceStringValue (prefs, valueIdMacOSDeploymentTarget,
	                          model->getValue (valueIdMacOSDeploymentTarget));
}

//------------------------------------------------------------------------
void Controller::fillCmakeSupportedPlatforms (const std::string& currentGenerator)
{
	auto cmakeGeneratorsPlatformsValue = model->getValue (valueIdCMakeSupportedPlatforms);
	assert (cmakeGeneratorsPlatformsValue);

	for (auto& item : cmakeCapabilities.generators)
	{
		if (item.name == currentGenerator)
		{
			IStringListValue::StringList list;
			list.emplace_back ("Defaults");
			for (auto& platform : item.platforms)
				list.emplace_back (platform);

			cmakeGeneratorsPlatformsValue->dynamicCast<IStringListValue> ()->updateStringList (
			    list);
			break;
		}
	}
	// reset to default
	cmakeGeneratorsPlatformsValue->performEdit (0);
}

//------------------------------------------------------------------------
void Controller::onScriptRunning (bool state)
{
	static constexpr auto valuesToDisable = {
	    valueIdTabBar,
	    valueIdVendorName,
	    valueIdVendorEMail,
	    valueIdVendorURL,
	    valueIdVendorNamespace,
	    valueIdVSTSDKPath,
	    valueIdCMakePath,
	    valueIdPluginType,
	    valueIdPluginPath,
	    valueIdPluginName,
	    valueIdPluginClassName,
	    valueIdPluginBundleID,
	    valueIdPluginFilenamePrefix,
	    valueIdChooseCMakePath,
	    valueIdChooseVSTSDKPath,
	    valueIdChoosePluginPath,
	    valueIdCreateProject,
	    valueIdCMakeGenerators,
	    valueIdCMakeSupportedPlatforms,
	    valueIdMacOSDeploymentTarget,
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
void Controller::onCMakeCapabilityCheckError ()
{
	Value::performSinglePlainEdit (*model->getValue (valueIdValidCMakePath), 0);
	Async::schedule (Async::mainQueue (), [this] () {
		showSimpleAlert ("Failure",
		                 "Could not check cmake capabilities. Check your cmake executable path!");
		Value::performSinglePlainEdit (*model->getValue (valueIdTabBar), 0);
	});
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
					cmakeCapabilities = std::move (*capabilities);

					auto cmakeVersionValue = model->getValue (valueIdCMakeVersion);
					UTF8String str ("CMake ");
					str += std::to_string (cmakeCapabilities.versionMajor) + "." +
					       std::to_string (cmakeCapabilities.versionMinor) + "." +
					       std::to_string (cmakeCapabilities.versionPatch);
					cmakeVersionValue->beginEdit ();
					cmakeVersionValue->dynamicCast<IStringValue> ()->setString (
					    UTF8String (std::move (str)));
					cmakeVersionValue->endEdit ();

					auto cmakeGeneratorsValue = model->getValue (valueIdCMakeGenerators);
					assert (cmakeGeneratorsValue);
					IStringListValue::StringList list;
					for (auto& item : cmakeCapabilities.generators)
					{
#if WINDOWS
						if (item.name.getString ().find ("Win64") == std::string::npos &&
						    item.name.getString ().find ("ARM") == std::string::npos &&
						    item.name.getString ().find ("IA64") == std::string::npos)
#endif // WINDOWS
							list.emplace_back (item.name);
					}
					cmakeGeneratorsValue->dynamicCast<IStringListValue> ()->updateStringList (list);

					Preferences prefs;
					if (auto generatorPref = prefs.get (valueIdCMakeGenerators))
					{
						auto value =
						    cmakeGeneratorsValue->getConverter ().stringAsValue (*generatorPref);
						cmakeGeneratorsValue->performEdit (value);

						fillCmakeSupportedPlatforms (generatorPref->getString ());

						if (auto supportedPlatformPref = prefs.get (valueIdCMakeSupportedPlatforms))
						{
							if (auto platforms = model->getValue (valueIdCMakeSupportedPlatforms))
							{
								auto v = platforms->getConverter ().stringAsValue (
								    *supportedPlatformPref);
								platforms->performEdit (v);
							}
						}
					}
					else
					{
						// we should use some defaults here
					}
				}
				else
				{
					onCMakeCapabilityCheckError ();
				}
				Value::performSingleEdit (*scriptRunningValue, 0.);
				process.reset ();
			}
		});
		if (!result)
		{
			Value::performSingleEdit (*scriptRunningValue, 0.);
			onCMakeCapabilityCheckError ();
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
	alert.description =
	    R"(You can choose between the "Proprietary Steinberg VST 3" or the "Open-source GPLv3" license (dual-license) depending on how you like to distribute your VST 3 plug-in.)";
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
	auto _sdkPathStr = getModelValueString (model, valueIdVSTSDKPath);
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath).getString ();
	auto _pluginOutputPathStr = getModelValueString (model, valueIdPluginPath);
	auto vendorStr = getModelValueString (model, valueIdVendorName).getString ();
	auto vendorHomePageStr = getModelValueString (model, valueIdVendorURL).getString ();
	auto emailStr = getModelValueString (model, valueIdVendorEMail).getString ();
	auto pluginNameStr = getModelValueString (model, valueIdPluginName).getString ();
	auto filenamePrefixStr = getModelValueString (model, valueIdPluginFilenamePrefix).getString ();
	auto pluginBundleIDStr = getModelValueString (model, valueIdPluginBundleID).getString ();
	auto vendorNamspaceStr = getModelValueString (model, valueIdVendorNamespace).getString ();
	auto pluginClassNameStr = getModelValueString (model, valueIdPluginClassName).getString ();
	auto pluginMacOSDeploymentTargetStr =
	    getModelValueString (model, valueIdMacOSDeploymentTarget).getString ();
	auto pluginTypeValue = model->getValue (valueIdPluginType);
	assert (pluginTypeValue);
	auto pluginTypeIndex = static_cast<size_t> (
	    pluginTypeValue->getConverter ().normalizedToPlain (pluginTypeValue->getValue ()));
	auto pluginTypeStr = pluginTypeStrings[pluginTypeIndex];

	auto pluginUseVSTGUI = model->getValue (valueIdUseVSTGUI)->getValue () != 0;

	if (_sdkPathStr.empty () || !validateVSTSDKPath (_sdkPathStr))
	{
		showSimpleAlert ("Cannot create Project", "The VST3 SDK path is not correct.");
		return;
	}
	auto sdkPathStr = _sdkPathStr.getString ();
	unixfyPath (sdkPathStr);
	if (_pluginOutputPathStr.empty ())
	{
		showSimpleAlert ("Cannot create Project", "You need to specify an output directory.");
		return;
	}
	auto pluginOutputPathStr = _pluginOutputPathStr.getString ();
	unixfyPath (pluginOutputPathStr);
	if (pluginOutputPathStr.find (sdkPathStr) == 0)
	{
		showSimpleAlert ("Cannot create Project",
		                 "Your output directory must be outside of the SDK directory.");
		return;
	}
	if (pluginNameStr.empty ())
	{
		showSimpleAlert ("Cannot create Project", "You need to specify a name for your plug-in.");
		return;
	}
	if (pluginBundleIDStr.empty ())
	{
		showSimpleAlert ("Cannot create Project",
		                 "You need to specify a Bundle ID (e.g. com.company.pluginame).");
		return;
	}

	if (pluginClassNameStr.empty ())
	{
		pluginClassNameStr = pluginNameStr;
		makeValidCppName (pluginClassNameStr);
	}
	auto cmakeProjectName = pluginNameStr;
	makeValidCppName (cmakeProjectName);

	if (auto scriptPath = IApplication::instance ().getCommonDirectories ().get (
	        CommonDirectoryLocation::AppResourcesPath))
	{
		*scriptPath += "GenerateVST3Plugin.cmake";

		Process::ArgumentList args;
		args.add ("-DSMTG_VST3_SDK_SOURCE_DIR_CLI=\"" + sdkPathStr + "\"");
		args.add ("-DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI=\"" + pluginOutputPathStr + "\"");
		args.add ("-DSMTG_PLUGIN_NAME_CLI=\"" + pluginNameStr + "\"");
		args.add ("-DSMTG_PLUGIN_CATEGORY_CLI=\"" + pluginTypeStr + "\"");
		args.add ("-DSMTG_CMAKE_PROJECT_NAME_CLI=\"" + cmakeProjectName + "\"");
		args.add ("-DSMTG_PLUGIN_BUNDLE_NAME_CLI=\"" + pluginNameStr + "\"");
		args.add ("-DSMTG_PLUGIN_IDENTIFIER_CLI=\"" + pluginBundleIDStr + "\"");
		args.add ("-DSMTG_MACOS_DEPLOYMENT_TARGET_CLI=\"" + pluginMacOSDeploymentTargetStr + "\"");
		args.add ("-DSMTG_VENDOR_NAME_CLI=\"" + vendorStr + "\"");
		args.add ("-DSMTG_VENDOR_HOMEPAGE_CLI=\"" + vendorHomePageStr + "\"");
		args.add ("-DSMTG_VENDOR_EMAIL_CLI=\"" + emailStr + "\"");
		args.add ("-DSMTG_PREFIX_FOR_FILENAMES_CLI=\"" + filenamePrefixStr + "\"");
		if (!vendorNamspaceStr.empty ())
			args.add ("-DSMTG_VENDOR_NAMESPACE_CLI=\"" + vendorNamspaceStr + "\"");
		if (!pluginClassNameStr.empty ())
			args.add ("-DSMTG_PLUGIN_CLASS_NAME_CLI=\"" + pluginClassNameStr + "\"");

		if (pluginUseVSTGUI)
			args.add ("-DSMTG_ENABLE_VSTGUI_SUPPORT_CLI=ON");
		else
			args.add ("-DSMTG_ENABLE_VSTGUI_SUPPORT_CLI=OFF");

		args.add ("-P");
		args.addPath (scriptPath->getString ());

		if (auto process = Process::create (cmakePathStr))
		{
			auto scriptRunningValue = model->getValue (valueIdScriptRunning);
			assert (scriptRunningValue);
			Value::performSingleEdit (*scriptRunningValue, 1.);
			auto scriptOutputValue = model->getValue (valueIdScriptOutput);
			assert (scriptOutputValue);

			Value::performStringValueEdit (*scriptOutputValue, cmakePathStr.data ());
			Value::performStringAppendValueEdit (*scriptOutputValue, " " + *scriptPath);
			for (const auto& arg : args.args)
			{
				Value::performStringAppendValueEdit (*scriptOutputValue, " " + arg);
			}

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
void Controller::runProjectCMake (const std::string& path)
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	auto value = model->getValue (valueIdCMakeGenerators);
	assert (value);
	if (!value)
		return;
	auto generator = value->getConverter ().valueAsString (value->getValue ());
	if (auto process = Process::create (cmakePathStr.getString ()))
	{
		auto scriptRunningValue = model->getValue (valueIdScriptRunning);
		assert (scriptRunningValue);
		Value::performSingleEdit (*scriptRunningValue, 1.);
		auto scriptOutputValue = model->getValue (valueIdScriptOutput);

		auto buildDir = path;
		buildDir += PlatformPathDelimiter;
		buildDir += "build";

		// first: delete previous build folder
		Process::ArgumentList prepareArgs;
		prepareArgs.add ("-E");
		prepareArgs.add ("remove_directory");
		prepareArgs.addPath (buildDir);
		process->run (prepareArgs,
		              [process] (Process::CallbackParams& p) mutable { process.reset (); });

		// now build the cmake command
		Process::ArgumentList args;

		// Generator Name
		args.add ("-G");
		args.addPath (generator.getString ());

		// Platform Name
		if (auto platforms = model->getValue (valueIdCMakeSupportedPlatforms))
		{
			auto platform = platforms->getConverter ().valueAsString (platforms->getValue ());
			if (!platform.empty () && platform != "Defaults")
				args.add ("-A " + platform.getString ());
		}

		// Path to Source
		args.add ("-S");
		args.addPath (path);

		// Path to Build
		args.add ("-B");
		args.addPath (buildDir);

		if (auto pluginUseVSTGUI = model->getValue (valueIdUseVSTGUI)->getValue () != 0)
			args.add ("-DSMTG_ENABLE_VSTGUI_SUPPORT=ON");
		else
			args.add ("-DSMTG_ENABLE_VSTGUI_SUPPORT=OFF");

		Value::performStringAppendValueEdit (*scriptOutputValue, "\n" + cmakePathStr + " ");
		for (const auto& a : args.args)
			Value::performStringAppendValueEdit (*scriptOutputValue, UTF8String (a) + " ");
		Value::performStringAppendValueEdit (*scriptOutputValue, "\n");

		bool result = false;
		if ((process = Process::create (cmakePathStr.getString ())))
		{
			result = process->run (args, [this, scriptRunningValue, scriptOutputValue, buildDir,
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
		}
		if (!result)
		{
			// TODO: Show error
			Value::performSingleEdit (*scriptRunningValue, 0.);
		}
	}
}

//------------------------------------------------------------------------
void Controller::openCMakeGeneratedProject (const std::string& path)
{
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	if (auto process = Process::create (cmakePathStr.getString ()))
	{
		auto scriptOutputValue = model->getValue (valueIdScriptOutput);
		Process::ArgumentList args;
		args.add ("--open");
		args.addPath (path);
		auto result =
		    process->run (args, [scriptOutputValue, process] (Process::CallbackParams& p) mutable {
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
void Controller::copyScriptOutputToClipboard ()
{
	if (auto value = model->getValue (valueIdScriptOutput))
	{
		if (auto stringValue = value->dynamicCast<IStringValue> ())
		{
			if (stringValue->getString ().empty ())
				return;
			auto frame = contentView.get ();
			if (!frame)
				return;
			auto data =
			    CDropSource::create (stringValue->getString ().data (),
			                         static_cast<uint32_t> (stringValue->getString ().length ()),
			                         IDataPackage::Type::kText);
			frame->setClipboard (data);
		}
	}
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
			if (el.empty ())
				continue;
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
	{
		std::string path = "/usr/local/bin/cmake";
		std::ifstream stream (path);
		if (stream.is_open ())
			return {std::move (path)};
	}
#endif
#if MAC
	{
		std::string path = "/Applications/CMake.app/Contents/bin/cmake";
		std::ifstream stream (path);
		if (stream.is_open ())
			return {std::move (path)};
	}
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
