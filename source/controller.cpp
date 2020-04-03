// Flags       : clang-format SMTGSequencer

#include "controller.h"
#include "process.h"

#include "vstgui/lib/cfileselector.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/icommondirectories.h"

#include <fstream>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

#if WINDOWS
static constexpr auto PlatformPathDelimiter = '\\';
static constexpr auto EnvPathSeparator = ';';
static constexpr auto CMakeExecutableName = "CMake.exe";
#else
static constexpr auto PlatformPathDelimiter = '/';
static constexpr auto EnvPathSeparator = ':';
static constexpr auto CMakeExecutableName = "cmake";
#endif

//------------------------------------------------------------------------
static constexpr auto CMakeWebPageURL = "https://cmake.org";

//------------------------------------------------------------------------
static void showSimpleAlert (const char* headline, const char* description)
{
	AlertBoxForWindowConfig config;
	config.headline = headline;
	config.description = description;
	config.defaultButton = "OK";
	config.window = IApplication::instance ().getWindows ().front ();
	IApplication::instance ().showAlertBoxForWindow (config);
}

//------------------------------------------------------------------------
static void setPreferenceStringValue (Preferences& prefs, const UTF8String& key,
                                      const ValuePtr& value)
{
	if (!value)
		return;
	if (auto strValue = value->dynamicCast<IStringValue> ())
		prefs.set (key, strValue->getString ());
}

//------------------------------------------------------------------------
static UTF8String getModelValueString (VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model,
                                       const UTF8String& key)
{
	auto value = model->getValue (key);
	if (auto strValue = value->dynamicCast<IStringValue> ())
	{
		return strValue->getString ();
	}

	return {};
}

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

	model->addValue (Value::makeStringValue (valueIdScriptOutput, ""));
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
}

//------------------------------------------------------------------------
void Controller::onScriptRunning (bool state)
{
	static constexpr auto valuesToDisable = {valueIdTabBar,
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
	                                         valueIdCreateProject};
	for (const auto& valueID : valuesToDisable)
	{
		if (auto value = model->getValue (valueID))
			value->setActive (!state);
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
void Controller::showCMakeNotInstalledWarning ()
{
	AlertBoxForWindowConfig config;
	config.headline = "CMake not found!";
	config.description = "You need to install CMake for your platform to use this application.";
	config.defaultButton = "OK";
	config.secondButton = "Open the CMake homepage";
	config.window = IApplication::instance ().getWindows ().front ();
	config.callback = [] (AlertResult result) {
		if (result == AlertResult::SecondButton)
			openURL (CMakeWebPageURL);
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
	// TODO: check that the path is valid
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
	auto cmakePathStr = getModelValueString (model, valueIdCMakePath);
	auto sdkPathStr = getModelValueString (model, valueIdVSTSDKPath);
	auto pluginOutputPathStr = getModelValueString (model, valueIdPluginPath);
	auto vendorStr = getModelValueString (model, valueIdVendor);
	auto emailStr = getModelValueString (model, valueIdEMail);
	auto pluginNameStr = getModelValueString (model, valueIdPluginName);
	auto filenamePrefixStr = getModelValueString (model, valueIdPluginFilenamePrefix);
	auto pluginBundleIDStr = getModelValueString (model, valueIdPluginBundleID);

	if (cmakePathStr.empty () || !validateCMakePath (cmakePathStr))
	{
		showCMakeNotInstalledWarning ();
		return;
	}
	if (sdkPathStr.empty () || !validateVSTSDKPath (sdkPathStr))
	{
		showSimpleAlert ("Cannot create Project", "The VST3 SDK Path is not correct.");
		return;
	}
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
		args.emplace_back ("-DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI=\"" + pluginOutputPathStr + "\"");
		args.emplace_back ("-DSMTG_PLUGIN_NAME_CLI=\"" + pluginNameStr + "\"");
		args.emplace_back ("-DSMTG_PLUGIN_IDENTIFIER_CLI=\"" + pluginBundleIDStr + "\"");
		args.emplace_back ("-DSMTG_VENDOR_NAME_CLI=\"" + vendorStr + "\"");
		args.emplace_back ("-DSMTG_VENDOR_EMAIL_CLI=\"" + emailStr + "\"");
		args.emplace_back ("-DSMTG_PREFIX_FOR_FILENAMES_CLI=\"" + filenamePrefixStr + "\"");
		args.emplace_back ("-P");
		args.emplace_back (scriptPath->getString ());

		if (auto process = Process::create (cmakePathStr.getString ()))
		{
			auto scriptRunningValue = model->getValue (valueIdScriptRunning);
			if (scriptRunningValue)
				Value::performSingleEdit (*scriptRunningValue, 1.);
			auto scriptOutputValue = model->getValue (valueIdScriptOutput);
			if (scriptOutputValue)
				Value::performStringValueEdit (*scriptOutputValue, "");

			if (!process->run (args, [scriptRunningValue, scriptOutputValue,
			                          process] (Process::CallbackParams& p) mutable {
				    if (!p.buffer.empty ())
				    {
					    if (auto v = dynamicPtrCast<IStringValue> (scriptOutputValue))
					    {
						    Value::performStringValueEdit (
						        *scriptOutputValue,
						        v->getString () + std::string (p.buffer.data (), p.buffer.size ()));
					    }
				    }
				    if (p.isEOF)
				    {
					    if (scriptRunningValue)
						    Value::performSingleEdit (*scriptRunningValue, 0.);
					    process.reset ();
				    }
			    }))
			{
				showSimpleAlert ("Could not execute CMake", "Please verify your path to CMake!");
			}
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
} // ProjectCreator
} // Vst
} // Steinberg
