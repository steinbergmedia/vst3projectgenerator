#pragma once

#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/lib/cframe.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
static constexpr auto valueIdTabBar = "TabBar";
static constexpr auto valueIdVendor = "Vendor";
static constexpr auto valueIdEMail = "EMail";
static constexpr auto valueIdURL = "URL";
static constexpr auto valueIdVSTSDKPath = "VST SDK Path";
static constexpr auto valueIdCMakePath = "CMake Path";

static constexpr auto valueIdPluginType = "PlugIn Type";
static constexpr auto valueIdPluginPath = "PlugIn Path";
static constexpr auto valueIdPluginName = "PlugIn Name";
static constexpr auto valueIdPluginBundleID = "PlugIn Bundle ID";
static constexpr auto valueIdPluginFilenamePrefix = "PlugIn Filename Prefix";

static constexpr auto valueIdChooseCMakePath = "Choose CMake Path";
static constexpr auto valueIdChooseVSTSDKPath = "Choose VST SDK Path";
static constexpr auto valueIdChoosePluginPath = "Choose PlugIn Path";

//------------------------------------------------------------------------
class Controller : public VSTGUI::Standalone::UIDesc::CustomizationAdapter,
                   public VSTGUI::Standalone::WindowControllerAdapter
{
public:
	using ModelBindingPtr = VSTGUI::Standalone::UIDesc::ModelBindingPtr;
	using IWindow = VSTGUI::Standalone::IWindow;
	using IValue = VSTGUI::Standalone::IValue;
	using CFrame = VSTGUI::CFrame;
	using UTF8String = VSTGUI::UTF8String;

	Controller ();

	const ModelBindingPtr getModel () const { return model; }
private:
	void onSetContentView (IWindow& window, const VSTGUI::SharedPointer<CFrame>& contentView) override;

	void storePreferences ();
	void chooseVSTSDKPath ();
	void chooseCMakePath ();
	void choosePluginPath ();

	template<typename Proc>
	void chooseDir (const UTF8String& valueId, Proc proc) const;

	bool validateVSTSDKPath (const UTF8String& path);
	bool validateCMakePath (const UTF8String& path);
	bool validatePluginPath (const UTF8String& path);

	VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model;
	VSTGUI::SharedPointer<CFrame> contentView;
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
