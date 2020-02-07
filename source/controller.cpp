#include "controller.h"

#include "vstgui/lib/cfileselector.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

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

	model = UIDesc::ModelBindingCallbacks::make ();
	model->addValue (Value::makeStringListValue (
	    valueIdTabBar, {"Welcome", "Create Plug-In Project", "Preferences"}));
	model->addValue (Value::makeStringValue (valueIdVendor, vendorPref ? *vendorPref : "Vendor"),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdEMail, emailPref ? *emailPref : "E-Mail"),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));
	model->addValue (Value::makeStringValue (valueIdURL, urlPref ? *urlPref : "URL"),
	                 UIDesc::ValueCalls::onEndEdit ([this] (IValue&) { storePreferences (); }));

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
void Controller::chooseDir (const UTF8String& valueId, Proc proc) const
{
	auto value = model->getValue (valueId);
	if (!value)
		return;

	auto fileSelector =
	    owned (CNewFileSelector::create (contentView, CNewFileSelector::kSelectDirectory));
	if (!fileSelector)
		return;

	Preferences prefs;
	if (auto vstSdkPathPref = prefs.get (valueId))
		fileSelector->setInitialDirectory (*vstSdkPathPref);

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
	chooseDir (valueIdVSTSDKPath,
	           [this] (const UTF8String& path) { return validateVSTSDKPath (path); });
}

//------------------------------------------------------------------------
void Controller::chooseCMakePath ()
{
	chooseDir (valueIdCMakePath,
	           [this] (const UTF8String& path) { return validateCMakePath (path); });
}

//------------------------------------------------------------------------
bool Controller::validateVSTSDKPath (const UTF8String& path)
{
	// TODO: check that the path is valid
	return true;
}

//------------------------------------------------------------------------
bool Controller::validateCMakePath (const UTF8String& path)
{
	// TODO: check that the path is valid
	return true;
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
