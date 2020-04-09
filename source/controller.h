// Flags       : clang-format SMTGSequencer

#pragma once

#include "cmakecapabilities.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"

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
static constexpr auto valueIdCreateProject = "Create Project";

static constexpr auto valueIdCreateIDEProject = "Create IDE Project";
static constexpr auto valueIdCMakeGenerators = "CMake Generators";

static constexpr auto valueIdScriptOutput = "Script Output";
static constexpr auto valueIdScriptRunning = "Script Running";

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
	using StringList = std::vector<std::string>;

	Controller ();

	const ModelBindingPtr getModel () const { return model; }

private:
	void onShow (const IWindow& window) override;
	void onSetContentView (IWindow& window,
	                       const VSTGUI::SharedPointer<CFrame>& contentView) override;

	void storePreferences ();
	void chooseVSTSDKPath ();
	void chooseCMakePath ();
	void choosePluginPath ();

	bool verifyCMakeInstallation ();
	void showCMakeNotInstalledWarning ();
	void gatherCMakeInformation ();

	void createProject ();

	template <typename Proc>
	void runFileSelector (const UTF8String& valueId, VSTGUI::CNewFileSelector::Style style,
	                      Proc proc) const;

	bool validateVSTSDKPath (const UTF8String& path);
	bool validateCMakePath (const UTF8String& path);
	bool validatePluginPath (const UTF8String& path);

	void onScriptRunning (bool state);

	StringList getEnvPaths ();
	VSTGUI::Optional<UTF8String> findCMakePath (const StringList& envPaths);

	VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model;
	VSTGUI::SharedPointer<CFrame> contentView;

	CMakeCapabilites cmakeCapabilities = {};
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
