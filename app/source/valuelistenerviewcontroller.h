//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/standalone/include/helpers/valuelistener.h"
#include "vstgui/uidescription/delegationcontroller.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class ValueListenerViewController : public VSTGUI::DelegationController,
                                    public VSTGUI::Standalone::ValueListenerAdapter
{
public:
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using DelegationController = VSTGUI::DelegationController;

	ValueListenerViewController (VSTGUI::IController* parent, ValuePtr value)
	: DelegationController (parent), value (value)
	{
		value->registerListener (this);
	}

	virtual ~ValueListenerViewController () noexcept { value->unregisterListener (this); }

	const ValuePtr& getValue () const { return value; }

private:
	ValuePtr value;
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
