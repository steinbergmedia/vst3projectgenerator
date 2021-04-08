//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "../../process.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/platform/linux/x11platform.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <stdio.h>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

//------------------------------------------------------------------------
struct Process::Impl
{
	FILE* handle = nullptr;
	CallbackFunction callback;
	SharedPointer<CVSTGUITimer> timer;
	std::string appPathUTF8Str;

	~Impl () noexcept
	{
		if (handle)
			pclose (handle);
	}
};

//------------------------------------------------------------------------
std::shared_ptr<Process> Process::create (const std::string& path)
{
	auto proc = std::make_shared<Process> ();
	proc->pImpl = std::make_unique<Process::Impl> ();
	proc->pImpl->appPathUTF8Str = path;

	return proc;
}

//------------------------------------------------------------------------
bool Process::run (const ArgumentList& arguments, CallbackFunction&& callback)
{
	pImpl->callback = std::move (callback);

	std::string commandLine;
	auto lastSeparatorPos = pImpl->appPathUTF8Str.find_last_of ('\\');
	if (lastSeparatorPos != std::string::npos)
	{
		commandLine = pImpl->appPathUTF8Str.substr (lastSeparatorPos + 1);
		commandLine += " ";
	}

	// Construct the command line arguments for popen, e.g. " -E capabilities"
	std::string appPath (pImpl->appPathUTF8Str.data ());
	auto it = arguments.args.begin ();
	if (it != arguments.args.end ())
		commandLine += " " + *it;
	while (++it != arguments.args.end ())
		commandLine += " " + *it;

	// Construct the full command, e.g. "/usr/bin/cmake -E capabilities"
	const std::string command = appPath + commandLine;

	pImpl->handle = popen (command.data(), "r");
	pImpl->timer = makeOwned<CVSTGUITimer> ([&, this] (CVSTGUITimer* timer) {
		Process::CallbackParams params;
		
		constexpr size_t kBufferSize = 256;
		char plainBuffer[kBufferSize] = {0};
		if (fgets (plainBuffer, sizeof (plainBuffer), pImpl->handle) != NULL)
		{
			// plainBuffer[255] is '\0', hence -1
			params.buffer.assign (std::begin (plainBuffer), std::end (plainBuffer) - 1);
		}
		else
		{
			params.isEOF = true;
			params.resultCode = 0;
		}

		if (params.isEOF)
			timer->stop ();
		if (!params.buffer.empty () || params.isEOF)
			pImpl->callback (params);
	});

	return true;
}

//------------------------------------------------------------------------
Process::~Process () noexcept = default;

//------------------------------------------------------------------------
void Process::ArgumentList::addPath (const std::string& str)
{
	args.emplace_back ("\"" + str + "\"");
}

//------------------------------------------------------------------------
void Process::ArgumentList::add (const std::string& str)
{
	args.emplace_back (str);
}

//------------------------------------------------------------------------
bool openURL (const std::string& url)
{
	const std::string command = "xdg-open " + url;
	system (command.data ());
	return true;
}

//------------------------------------------------------------------------
} // Steinberg
} // Vst
