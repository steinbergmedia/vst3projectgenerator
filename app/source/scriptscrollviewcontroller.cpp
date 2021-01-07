//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "scriptscrollviewcontroller.h"
#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/standalone/include/helpers/value.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
ScriptScrollViewController::ScriptScrollViewController (IController* parent, ValuePtr value)
: ValueListenerViewController (parent, value)
{
}

//------------------------------------------------------------------------
auto ScriptScrollViewController::verifyView (CView* view, const UIAttributes& attributes,
                                             const IUIDescription* description) -> CView*
{
	if (auto sv = dynamic_cast<CScrollView*> (view))
	{
		scrollView = sv;
		if (auto label = dynamic_cast<CMultiLineTextLabel*> (scrollView->getView (0)))
		{
			label->registerViewListener (this);
		}
	}
	return controller->verifyView (view, attributes, description);
}

//------------------------------------------------------------------------
void ScriptScrollViewController::scrollToBottom ()
{
	if (!scrollView)
		return;
	auto containerSize = scrollView->getContainerSize ();
	containerSize.top = containerSize.bottom - 10;
	scrollView->makeRectVisible (containerSize);
}

//------------------------------------------------------------------------
void ScriptScrollViewController::onEndEdit (IValue&)
{
	scrollToBottom ();
}

//------------------------------------------------------------------------
void ScriptScrollViewController::viewWillDelete (CView* view)
{
	if (auto label = dynamic_cast<CMultiLineTextLabel*> (view))
		label->unregisterViewListener (this);
}

//------------------------------------------------------------------------
void ScriptScrollViewController::viewAttached (CView* view)
{
	if (auto label = dynamic_cast<CMultiLineTextLabel*> (view))
	{
		if (label->getAutoHeight ())
		{
			label->setAutoHeight (false);
			label->setAutoHeight (true);
		}
		label->unregisterViewListener (this);
		scrollToBottom ();
	}
}

//------------------------------------------------------------------------
void ScriptScrollViewController::appendContextMenuItems (COptionMenu& contextMenu, CView* view,
                                                         const CPoint& where)
{
	if (auto stringValue = getValue ()->dynamicCast<IStringValue> ())
	{
		if (stringValue->getString ().empty ())
			return;
		auto commandItem = new CCommandMenuItem ({"Copy text to clipboard"});
		commandItem->setActions ([&, stringValue] (CCommandMenuItem*) {
			auto frame = contextMenu.getFrame ();
			if (!frame)
				return;
			auto data =
			    CDropSource::create (stringValue->getString ().data (),
			                         static_cast<uint32_t> (stringValue->getString ().length ()),
			                         IDataPackage::Type::kText);
			frame->setClipboard (data);
		});
		contextMenu.addEntry (commandItem);
	}
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
