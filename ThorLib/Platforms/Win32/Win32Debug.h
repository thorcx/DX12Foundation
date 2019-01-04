#pragma once

#include "../Basic/Public/BasicPlatformDebug.h"

//Win平台下的

struct WindowsDebug : public BasicPlatformDebug
{

	static void AssertionFailed(const ThorCXLibrary::Char *Message,
								const char *Function, // type of __FUNCTION__
								const char *File,     // type of __FILE__
								int Line);

	static void OutputDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity,
		const ThorCXLibrary::Char* Message,
		const char *Function,
		const char *File,
		int Line);
};
