// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/lib/optional.h"
#include <functional>
#include <string>
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
class Process
{
public:
	struct CallbackParams
	{
		bool isEOF {false};
		int resultCode {0};
		std::vector<char> buffer;
	};

	using CallbackFunction = std::function<void (CallbackParams&)>;
	using ArgumentList = std::vector<std::string>;

	static std::shared_ptr<Process> create (const std::string& path);

	bool run (const ArgumentList& arguments, CallbackFunction&& callback);

	~Process () noexcept;

private:
	struct Impl;
	std::unique_ptr<Impl> pImpl {nullptr};
};

//------------------------------------------------------------------------
} // Steinberg
} // Vst
