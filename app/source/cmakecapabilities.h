//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/lib/cstring.h"
#include "vstgui/lib/optional.h"
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
struct GeneratorCapabilites
{
	VSTGUI::UTF8String name;

	std::vector<VSTGUI::UTF8String> platforms;
};

struct CMakeCapabilites
{
	int32_t versionMajor {0};
	int32_t versionMinor {0};
	int32_t versionPatch {0};

	std::vector<GeneratorCapabilites> generators;
};

//------------------------------------------------------------------------
VSTGUI::Optional<CMakeCapabilites> parseCMakeCapabilities (const std::string& capabilitesJSON);

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
