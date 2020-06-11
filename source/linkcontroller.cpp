//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "linkcontroller.h"

#include "include/rapidjson/document.h"
#include "vstgui/lib/malloc.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/icommondirectories.h"
#include "vstgui/uidescription/cstream.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace ProjectCreator {

using namespace VSTGUI::Standalone;
using namespace VSTGUI;

//------------------------------------------------------------------------
LinkController::LinkController ()
{
	auto path = IApplication::instance ().getCommonDirectories ().get (
	    CommonDirectoryLocation::AppResourcesPath);
	if (!path)
		return;

	*path += "links.json";
	CFileStream fs;
	if (!fs.open (path->data (), CFileStream::kReadMode))
		return;
	auto fileSize = fs.seek (0, SeekableStream::SeekMode::kSeekEnd);
	fs.seek (0, SeekableStream::SeekMode::kSeekSet);

	Buffer<char> buffer (fileSize);
	fs.readRaw (buffer.data (), buffer.size ());

	try
	{
		using namespace rapidjson;
		Document doc;
		doc.Parse (buffer.data (), buffer.size ());
		for (const auto& value : doc.GetArray ())
		{
			auto title = UTF8String (value["title"].GetString ());
			auto url = value["url"].GetString ();
			if (!title.empty () && url)
			{
				title += UTF8String (" -> ") + url;
				titles.emplace_back (title);
				urls.emplace_back (url);
			}
		}
	}
	catch (...)
	{
		titles.clear ();
		urls.clear ();
	}
}

//------------------------------------------------------------------------
const LinkController& LinkController::instance ()
{
	static LinkController gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
} // ProjectCreator
} // Vst
} // Steinberg
