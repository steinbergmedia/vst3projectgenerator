//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer

#include "../../process.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/platform/win32/win32support.h"
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <shellapi.h>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

//------------------------------------------------------------------------
struct Process::Impl
{
	HANDLE readPipe {nullptr};
	HANDLE writePipe {nullptr};
	PROCESS_INFORMATION procInfo {};
	CallbackFunction callback;
	SharedPointer<CVSTGUITimer> timer;
	std::string appPathUTF8Str;

	~Impl () noexcept
	{
		if (readPipe)
			CloseHandle (readPipe);
		if (writePipe)
			CloseHandle (writePipe);
		if (procInfo.hProcess)
			CloseHandle (procInfo.hProcess);
		if (procInfo.hThread)
			CloseHandle (procInfo.hThread);
	}
};

//------------------------------------------------------------------------
std::shared_ptr<Process> Process::create (const std::string& path)
{
	auto proc = std::make_shared<Process> ();
	proc->pImpl = std::make_unique<Process::Impl> ();

	SECURITY_ATTRIBUTES saAttr {};
	saAttr.nLength = sizeof (SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = nullptr;

	if (!CreatePipe (&proc->pImpl->readPipe, &proc->pImpl->writePipe, &saAttr, 0))
		return nullptr;
	if (!SetHandleInformation (proc->pImpl->readPipe, HANDLE_FLAG_INHERIT, 0))
		return nullptr;

	proc->pImpl->appPathUTF8Str = path;

	return proc;
}

//------------------------------------------------------------------------
bool Process::run (const ArgumentList& arguments, CallbackFunction&& callback)
{
	pImpl->callback = std::move (callback);

	STARTUPINFO startupInfo {};
	startupInfo.cb = sizeof (STARTUPINFO);
	startupInfo.hStdError = pImpl->writePipe;
	startupInfo.hStdOutput = pImpl->writePipe;
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = 0;

	std::string commandLine;
	auto lastSeparatorPos = pImpl->appPathUTF8Str.find_last_of ('\\');
	if (lastSeparatorPos != std::string::npos)
	{
		commandLine = pImpl->appPathUTF8Str.substr (lastSeparatorPos + 1);
		commandLine += " ";
	}

	UTF8StringHelper appPath (pImpl->appPathUTF8Str.data ());

	auto it = arguments.args.begin ();
	if (it != arguments.args.end ())
		commandLine += *it;
	while (++it != arguments.args.end ())
		commandLine += " " + *it;

	UTF8StringHelper commandLineUTF16 (commandLine.data ());
	auto success = CreateProcess (
	    reinterpret_cast<const TCHAR*> (appPath.getWideString ()),
	    const_cast<LPWSTR> (reinterpret_cast<const TCHAR*> (commandLineUTF16.getWideString ())),
	    NULL, // process security attributes
	    NULL, // primary thread security attributes
	    TRUE, // handles are inherited
	    0, // creation flags
	    NULL, // use parent's environment
	    NULL, // use parent's current directory
	    &startupInfo, &pImpl->procInfo);

	if (!success)
	{
		return false;
	}

	pImpl->timer = makeOwned<CVSTGUITimer> ([this] (CVSTGUITimer* timer) {
		Process::CallbackParams params;
		if (WaitForSingleObject (pImpl->readPipe, 0) == WAIT_OBJECT_0)
		{
			DWORD bytesAvailable {};
			PeekNamedPipe (pImpl->readPipe, NULL, 0, nullptr, &bytesAvailable, nullptr);
			if (bytesAvailable > 0)
			{
				params.buffer.resize (bytesAvailable);
				DWORD readBytes = 0;
				ReadFile (pImpl->readPipe, params.buffer.data (), bytesAvailable, &readBytes,
				          nullptr);
			}
		}
		if (WaitForSingleObject (pImpl->procInfo.hProcess, 0) == WAIT_OBJECT_0)
		{
			DWORD exitCode {};
			GetExitCodeProcess (pImpl->procInfo.hProcess, &exitCode);
			params.isEOF = true;
			params.resultCode = exitCode;
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
	if (str.find (' ') != std::string::npos)
		addPath (str);
	else
		args.emplace_back (str);
}

//------------------------------------------------------------------------
bool openURL (const std::string& url)
{
	UTF8StringHelper helper (url.data (), static_cast<int> (url.size ()));
	ShellExecute (0, 0, helper.getWideString (), 0, 0, SW_SHOW);
	return true;
}

//------------------------------------------------------------------------
} // Steinberg
} // Vst
