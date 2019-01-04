#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include "DebugOutput.h"
#include "FormatString.h"

namespace ThorCXLibrary
{
	
#define LOG_DEBUG_MESSAGE(Severity, ...)\
auto _msg = ThorCXLibrary::FormatString(__VA_ARGS__);\
if(ThorCXLibrary::DebugMessageCallback != nullptr)\
	ThorCXLibrary::DebugMessageCallback(Severity , _msg.c_str(), nullptr, nullptr, 0);


#define LOG_ERROR_MESSAGE(...)		LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Error , ##__VA_ARGS__)
#define LOG_WARNING_MESSAGE(...)	LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Warning, ##__VA_ARGS__)
#define LOG_INFO_MESSAGE(...)		LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Info, ##__VA_ARGS__)
}
