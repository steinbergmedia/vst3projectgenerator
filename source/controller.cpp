#include "controller.h"

#include "vstgui/lib/cfileselector.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/ialertbox.h"
#include <cstdlib>
#include <fstream>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

#if WINDOWS
static constexpr auto PlatformPathDelimiter = '\\';
static constexpr auto CMakeExecutableName = "CMake.exe";
#else
static constexpr auto PlatformPathDelimiter = '/';
static constexpr auto CMakeExecutableName = "cmake";
#endif

//------------------------------------------------------------------------
static void showSimpleAlert (const char* headline, const char* description)
{
	AlertBoxConfig config;
	config.headline = headline;
	config.description = description;
	config.defaultButton = "OK";
	IApplication::instance ().showAlertBox (config);
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
	model->addValue (Value::makeStringValue (valueIdPluginPath, ""));

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
	return true;
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
		while (std::getline (input, el, ':'))
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
