#pragma once

#include "../Basic/Public/BasicPlatformDebug.h"

//Win平台下的

struct WindowsDebug : public BasicPlatformDebug
{
	static void OutputDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity,
		const ThorCXLibrary::Char* Message,
		const char *Function,
		const char *File,
		int Line);
};
