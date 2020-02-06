#pragma once

#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class Application : public VSTGUI::Standalone::Application::DelegateAdapter,
                    public VSTGUI::Standalone::WindowListenerAdapter
{
public:
	Application ();

	void finishLaunching () override;
	void onClosed (const VSTGUI::Standalone::IWindow& window) override;
private:
	VSTGUI::Standalone::UIDesc::ModelBindingCallbacksPtr model;
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
