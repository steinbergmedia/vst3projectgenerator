//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/lib/cstring.h"
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
class LinkController
{
public:
	using UTF8String = VSTGUI::UTF8String;
	using StringList = std::vector<UTF8String>;

	static const LinkController& instance ();

	const StringList& getTitles () const { return titles; };
	const StringList& getUrls () const { return urls; }

private:
	LinkController ();

	std::vector<UTF8String> titles;
	std::vector<UTF8String> urls;
};

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
