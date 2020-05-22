//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "dimmviewcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
DimmViewController::DimmViewController (IController* parent, ValuePtr value, float dimm)
: ValueListenerViewController (parent, value), dimmValue (dimm)
{
}

//------------------------------------------------------------------------
auto DimmViewController::verifyView (CView* view, const UIAttributes& attributes,
                                     const IUIDescription* description) -> CView*
{
	if (auto name = attributes.getAttributeValue (IUIDescription::kCustomViewName))
	{
		if (*name == "Container")
		{
			dimmView = view;
			onEndEdit (*getValue ());
		}
	}
	return controller->verifyView (view, attributes, description);
}

//------------------------------------------------------------------------
void DimmViewController::onEndEdit (IValue& value)
{
	if (!dimmView)
		return;
	bool b = value.getValue () > 0.5;
	float alphaValue = b ? dimmValue : 1.f;
	dimmView->setAlphaValue (alphaValue);
	dimmView->setMouseEnabled (!b);
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
