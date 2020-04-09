// Flags       : clang-format SMTGSequencer

#include "cmakecapabilities.h"
#include "include/rapidjson/document.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

//------------------------------------------------------------------------
auto parseCMakeCapabilities (const std::string& capabilitesJSON)
    -> VSTGUI::Optional<CMakeCapabilites>
{
	using namespace rapidjson;
	Document doc;
	doc.Parse (capabilitesJSON.data (), capabilitesJSON.size ());

	if (!doc.HasMember ("version") || !doc.HasMember ("generators"))
		return {};

	auto& generators = doc["generators"];
	auto& version = doc["version"];
	if (!version.HasMember ("major") || !version.HasMember ("minor") ||
	    !version.HasMember ("patch") || !generators.IsArray ())
		return {};

	CMakeCapabilites cap;
	cap.versionMajor = version["major"].GetInt ();
	cap.versionMinor = version["minor"].GetInt ();
	cap.versionPatch = version["patch"].GetInt ();

	for (const auto& gen : generators.GetArray ())
	{
		if (!gen.HasMember ("name"))
			return {};
		auto name = std::string (gen["name"].GetString ());
		cap.generators.emplace_back (name);
		if (gen.HasMember ("extraGenerators"))
		{
			for (auto& extraGen : gen["extraGenerators"].GetArray ())
			{
				if (!extraGen.IsString ())
					continue;
				cap.generators.emplace_back (std::string (extraGen.GetString ()) + " - " + name);
			}
		}
	}

	return {std::move (cap)};
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
