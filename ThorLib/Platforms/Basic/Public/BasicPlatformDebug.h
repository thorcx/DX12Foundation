#pragma once

#include "../../../ThorCommon/LogUtils.h"


struct BasicPlatformDebug
{
	static ThorCXLibrary::String FormatDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity,
		const ThorCXLibrary::Char* Message,
		const char* Function,
		const char* File,
		int Line);
	
};