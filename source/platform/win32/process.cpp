//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST3 Hosting Example
// Filename    : ExternalProcess_win32.cpp
// Created by  : ascheffler, 06/2016
// Description : <#Description#>
//------------------------------------------------------------------------

#include "../../process.h"
#include "VST3/include/common/StringConvert.h"
#include "VST3/include/common/Timer.h"
#include <Windows.h>
#include <algorithm>
#include <array>

//------------------------------------------------------------------------
namespace VST3 {
namespace Hosting {

//------------------------------------------------------------------------
struct ExternalProcess::Impl
{
	HANDLE readPipe {nullptr};
	HANDLE writePipe {nullptr};
	PROCESS_INFORMATION procInfo {};
	CallbackFunction callback;
	Timer::Ptr timer;
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

	auto appPath = StringConvert::convert (appPathUTF8Str);

	std::string commandLine;
	auto it = arguments.begin ();
	if (it != arguments.end ())
		commandLine = "\"" + *it + "\"";
	while (++it != arguments.end ())
		commandLine += " \"" + *it + "\"";

	auto commandLineUTF16 = StringConvert::convert (commandLine);
	auto success = CreateProcess (
	    reinterpret_cast<const TCHAR*> (appPath.data ()),
	    const_cast<LPWSTR> (reinterpret_cast<const TCHAR*> (commandLineUTF16.data ())),
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

	pImpl->timer = Timer::create (16, [pImpl] () {
		Process::CallbackParams params;
		if (WaitForSingleObject (pImpl->procInfo.hProcess, 0) == WAIT_OBJECT_0)
		{
			DWORD exitCode {};
			GetExitCodeProcess (pImpl->procInfo.hProcess, &exitCode);
			params.isEOF = true;
			params.resultCode = exitCode;
		}
		else if (WaitForSingleObject (pImpl->readPipe, 0) == WAIT_OBJECT_0)
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
		if (!params.buffer.empty () || params.isEOF)
			pImpl->callback (params);
		if (params.isEOF)
			return false;
		return true;
	});

	return true;
}

//------------------------------------------------------------------------
Process::~Process () noexcept = default;

//------------------------------------------------------------------------
} // Hosting
} // VST3
