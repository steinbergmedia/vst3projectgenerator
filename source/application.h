//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class Application : public VSTGUI::Standalone::Application::DelegateAdapter,
                    public VSTGUI::Standalone::WindowListenerAdapter
{
	using IWindow = VSTGUI::Standalone::IWindow;

public:
	Application ();

	void finishLaunching () override;
	void onClosed (const IWindow& window) override;
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
