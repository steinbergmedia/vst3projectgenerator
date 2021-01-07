//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "valuelistenerviewcontroller.h"
#include "vstgui/lib/iviewlistener.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class ScriptScrollViewController : public ValueListenerViewController,
                                   public VSTGUI::ViewListenerAdapter,
                                   public VSTGUI::IContextMenuController2
{
public:
	using CView = VSTGUI::CView;
	using UIAttributes = VSTGUI::UIAttributes;
	using IUIDescription = VSTGUI::IUIDescription;
	using IValue = VSTGUI::Standalone::IValue;
	using COptionMenu = VSTGUI::COptionMenu;
	using CPoint = VSTGUI::CPoint;
	using CScrollView = VSTGUI::CScrollView;

	ScriptScrollViewController (IController* parent, ValuePtr value);

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override;

	void scrollToBottom ();

	void onEndEdit (IValue&) override;

	void viewWillDelete (CView* view) override;

	void viewAttached (CView* view) override;

	void appendContextMenuItems (COptionMenu& contextMenu, CView* view,
	                             const CPoint& where) override;

private:
	CScrollView* scrollView {nullptr};
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
