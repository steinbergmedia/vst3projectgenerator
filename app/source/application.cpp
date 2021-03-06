//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "application.h"
#include "controller.h"
#include "version.h"

#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
Application::Application ()
: Application::DelegateAdapter (
      {"VST3 Project Generator", BUILD_STRING, "com.steinberg.vst3sdk.projectgenerator"})
{
}

//------------------------------------------------------------------------
void Application::finishLaunching ()
{
	auto controller = std::make_shared<Controller> ();

	UIDesc::Config config;
	config.uiDescFileName = "Window.uidesc";
	config.viewName = "Window";
	config.modelBinding = controller->getModel ();
	config.customization = controller;
	config.windowConfig.title = "VST3 Project Generator";
	config.windowConfig.autoSaveFrameName = "MainWindow";
	config.windowConfig.style.border ().close ().centered ().size ();
	if (auto window = UIDesc::makeWindow (config))
	{
		window->show ();
		window->registerWindowListener (this);
	}
	else
	{
		IApplication::instance ().quit ();
	}
}

//------------------------------------------------------------------------
void Application::onClosed (const IWindow& window)
{
	IApplication::instance ().quit ();
}

//------------------------------------------------------------------------
static Standalone::Application::Init gAppDelegate (std::make_unique<Application> ());

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
