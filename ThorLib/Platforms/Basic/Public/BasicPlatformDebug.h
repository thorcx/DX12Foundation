#pragma once

#include "../../../ThorCommon/LogUtils.h"


struct BasicPlatformDebug
{
	static ThorCXLibrary::String FormatAssertionFailedMessage(const ThorCXLibrary::Char* Message,
															const char* Function,  // type of __FUNCTION__
															const char* File,      // type of __FILE__
															int Line);

	static ThorCXLibrary::String FormatDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity,
		const ThorCXLibrary::Char* Message,
		const char* Function,
		const char* File,
		int Line);
	
};

//平台相关的Deubg函数前向声明
void DebugAssertionFailed(const ThorCXLibrary::Char* Message, const char* Function, const char* File, int Line);