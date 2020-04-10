// Flags       : clang-format SMTGSequencer

#pragma once

#include "vstgui/lib/optional.h"
#include <functional>
#include <memory>
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

	struct ArgumentList
	{
		void add (const std::string& str) { args.emplace_back (str); }
		void addPath (const std::string& str);

		std::vector<std::string> args;
	};

	using CallbackFunction = std::function<void (CallbackParams&)>;

	static std::shared_ptr<Process> create (const std::string& path);

	bool run (const ArgumentList& arguments, CallbackFunction&& callback);

	~Process () noexcept;

private:
	struct Impl;
	std::unique_ptr<Impl> pImpl {nullptr};
};

//------------------------------------------------------------------------
bool openURL (const std::string& url);

//------------------------------------------------------------------------
} // Steinberg
} // Vst
