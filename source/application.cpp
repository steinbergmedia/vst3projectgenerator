#include "application.h"

#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
Application::Application ()
: Application::DelegateAdapter (
      {"VST3 Project Generator", "1.0.0", "net.steinberg.vst3.projectgenerator"})
{
	model = UIDesc::ModelBindingCallbacks::make ();
	model->addValue (
	    Value::makeStringListValue ("TabBar", {"Welcome", "Create Plug-In Project", "Preferences"}));
}

//------------------------------------------------------------------------
void Application::finishLaunching ()
{
	UIDesc::Config config;
	config.uiDescFileName = "Window.uidesc";
	config.viewName = "Window";
	config.modelBinding = model;
	config.windowConfig.title = "VST3 Project Generator";
	config.windowConfig.autoSaveFrameName = "MainWindow";
	config.windowConfig.style.border ().close ().centered ();
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
static VSTGUI::Standalone::Application::Init gAppDelegate (std::make_unique<Application> ());

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
