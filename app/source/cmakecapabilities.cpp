//------------------------------------------------------------------------
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

	CMakeCapabilites cap;

	try
	{
		Document doc;
		doc.Parse (capabilitesJSON.data (), capabilitesJSON.size ());
		if (!doc.IsObject ())
			return {};

		if (!doc.HasMember ("version") || !doc.HasMember ("generators"))
			return {};

		auto& generators = doc["generators"];
		auto& version = doc["version"];
		if (!version.HasMember ("major") || !version.HasMember ("minor") ||
		    !version.HasMember ("patch") || !generators.IsArray ())
			return {};

		cap.versionMajor = version["major"].GetInt ();
		cap.versionMinor = version["minor"].GetInt ();
		cap.versionPatch = version["patch"].GetInt ();

		for (const auto& gen : generators.GetArray ())
		{
			if (!gen.HasMember ("name"))
				return {};

			GeneratorCapabilites genCap;
			genCap.name = std::string (gen["name"].GetString ());

			if (gen.HasMember ("platformSupport") && gen["platformSupport"].GetBool ())
			{
				if (gen.HasMember ("supportedPlatforms"))
				{
					for (auto& platform : gen["supportedPlatforms"].GetArray ())
					{
						genCap.platforms.emplace_back (std::string (platform.GetString ()));
					}
				}
			}

			cap.generators.emplace_back (genCap);
			if (gen.HasMember ("extraGenerators"))
			{
				for (auto& extraGen : gen["extraGenerators"].GetArray ())
				{
					if (!extraGen.IsString ())
						continue;
					GeneratorCapabilites extraGenCap;
					extraGenCap.name =
					    std::string (extraGen.GetString ()) + " - " + std::string (genCap.name);
					cap.generators.emplace_back (extraGenCap);
				}
			}
		}
		std::sort (cap.generators.begin (), cap.generators.end (),
		           [] (const auto& lhs, const auto& rhs) {
			           return lhs.name.getString () > rhs.name.getString ();
		           });
	}
	catch (...)
	{
		return {};
	}

	return {std::move (cap)};
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
