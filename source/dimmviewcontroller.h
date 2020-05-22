//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "valuelistenerviewcontroller.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class DimmViewController : public ValueListenerViewController
{
public:
	using CView = VSTGUI::CView;
	using UIAttributes = VSTGUI::UIAttributes;
	using IUIDescription = VSTGUI::IUIDescription;
	using IValue = VSTGUI::Standalone::IValue;

	DimmViewController (IController* parent, ValuePtr value, float dimm = 0.f);

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override;

	void onEndEdit (IValue& value) override;

private:
	float dimmValue {0.f};
	CView* dimmView {nullptr};
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
