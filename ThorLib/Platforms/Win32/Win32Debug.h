#pragma once

#include "../Basic/Public/BasicPlatformDebug.h"

//Winƽ̨�µ�

struct WindowsDebug : public BasicPlatformDebug
{
	static void OutputDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity,
		const ThorCXLibrary::Char* Message,
		const char *Function,
		const char *File,
		int Line);
};
