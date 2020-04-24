// Flags       : clang-format SMTGSequencer

#import "../../process.h"
#import <Cocoa/Cocoa.h>

using namespace Steinberg::Vst;
using namespace VSTGUI;

//------------------------------------------------------------------------
@interface SMTG_ExternalProcess : NSObject
{
	Process::CallbackFunction callback;
}
@property (readwrite, retain) NSTask* task;

- (instancetype)initWithTask:(NSTask*)task;
- (void)setCallback:(Process::CallbackFunction&&)inCallback;

@end

//------------------------------------------------------------------------
@implementation SMTG_ExternalProcess

//------------------------------------------------------------------------
- (instancetype)initWithTask:(NSTask*)inTask;
{
	self = [super init];
	if (self)
	{
		self.task = inTask;

		[[NSNotificationCenter defaultCenter]
		    addObserver:self
		       selector:@selector (onData:)
		           name:NSFileHandleReadCompletionNotification
		         object:[[self.task standardOutput] fileHandleForReading]];
	}

	return self;
}

//------------------------------------------------------------------------
- (void)setCallback:(Process::CallbackFunction&&)inCallback
{
	callback = std::move (inCallback);
}

//------------------------------------------------------------------------
- (void)dealloc
{
	if (self.task)
		[self.task terminate];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

//------------------------------------------------------------------------
- (void)onData:(NSNotification*)aNotification
{
	Process::CallbackParams params {};

	NSData* data = [[aNotification userInfo] objectForKey:NSFileHandleNotificationDataItem];
	if (data.length)
	{
		auto dataBytes = reinterpret_cast<const char*> (data.bytes);
		params.buffer = {dataBytes, dataBytes + data.length};
	}
	else
	{
		[self.task terminate];
		[self.task waitUntilExit];
		params.resultCode = self.task.terminationStatus;
		params.isEOF = true;

		[[NSNotificationCenter defaultCenter] removeObserver:self];
		self.task = nil;
	}
	callback (params);
	if (!params.isEOF)
		[[aNotification object] readInBackgroundAndNotify];
}
@end

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
struct Process::Impl
{
	SMTG_ExternalProcess* delegate {nil};
	~Impl () noexcept
	{
		if (delegate)
			[delegate release];
	}
};

//------------------------------------------------------------------------
std::shared_ptr<Process> Process::create (const std::string& path)
{
	auto proc = std::make_shared<Process> ();
	proc->pImpl = std::make_unique<Impl> ();

	NSTask* task = [NSTask new];
	task.launchPath = [NSString stringWithUTF8String:path.data ()];
	task.standardOutput = [NSPipe pipe];
	task.standardError = task.standardOutput;
	task.environment = [NSDictionary new];
	[[task.standardOutput fileHandleForReading] readInBackgroundAndNotify];

	proc->pImpl->delegate = [[SMTG_ExternalProcess alloc] initWithTask:task];

	return proc;
}

//------------------------------------------------------------------------
bool Process::run (const ArgumentList& arguments, CallbackFunction&& callback)
{
	NSMutableArray<NSString*>* args = [NSMutableArray new];
	for (const auto& arg : arguments.args)
	{
		[args addObject:[NSString stringWithUTF8String:arg.data ()]];
	}
	pImpl->delegate.task.arguments = args;

#if DEBUG
	NSLog (@"%@", [args description]);
#endif

	[pImpl->delegate setCallback:std::move (callback)];
	@try
	{
		[pImpl->delegate.task launch];
	}
	@catch (NSException* exception)
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------
Process::~Process () noexcept = default;

//------------------------------------------------------------------------
void Process::ArgumentList::addPath (const std::string& str)
{
	args.emplace_back (str);
}

//------------------------------------------------------------------------
bool openURL (const std::string& url)
{
	NSURL* nsUrl = [NSURL URLWithString:[NSString stringWithUTF8String:url.data ()]];
	return [[NSWorkspace sharedWorkspace] openURL:nsUrl];
}

//------------------------------------------------------------------------
} // Steinberg
} // Vst
