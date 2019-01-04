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

//ƽ̨��ص�Deubg����ǰ������
void DebugAssertionFailed(const ThorCXLibrary::Char* Message, const char* Function, const char* File, int Line);