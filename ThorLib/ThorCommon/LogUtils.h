#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include "DebugOutput.h"
#include "FormatString.h"

namespace ThorCXLibrary
{
	template<bool>
	void ThrowIf(std::string &&)
	{}

	template<>
	inline void ThrowIf<true>(std::string &&msg)
	{
		throw std::runtime_error(std::move(msg));
	}

	template<bool bThrowException, typename... ArgsType>
	void LogError(const char *Function, const char *FullFilePath, int Line, const ArgsType&... Args)
	{
		std::string FileName(FullFilePath);
		auto LastSlashPos = FileName.find_last_of("/\\");
		if (LastSlashPos != std::string::npos)
			FileName.erase(0, LastSlashPos + 1);
		auto msg = ThorCXLibrary::FormatString(Args...);
		if (DebugMessageCallback != nullptr)
		{
			DebugMessageCallback(bThrowException ? DebugMessageSeverity::FatalError : DebugMessageSeverity::Error, msg.c_str(), Function, FileName.c_str(), Line);
		}
		else
		{
			std::cerr << "ThorcxEngine: " << (bThrowException ? "Fatal Error" : "Error") << " in" << Function << "() (" << FileName << ", " << Line << "): " << msg << '\n';
		}
		ThrowIf<bThrowException>(std::move(msg));
	}
}

#define LOG_ERROR(...)\
ThorCXLibrary::LogError<false>(__FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);

#define LOG_ERROR_AND_THROW(...)\
ThorCXLibrary::LogError<true>(__FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);


#define LOG_DEBUG_MESSAGE(Severity, ...)\
auto _msg = ThorCXLibrary::FormatString(__VA_ARGS__);\
if(ThorCXLibrary::DebugMessageCallback != nullptr)\
	ThorCXLibrary::DebugMessageCallback(Severity , _msg.c_str(), nullptr, nullptr, 0);


#define LOG_ERROR_MESSAGE(...)		LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Error , ##__VA_ARGS__)
#define LOG_WARNING_MESSAGE(...)	LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Warning, ##__VA_ARGS__)
#define LOG_INFO_MESSAGE(...)		LOG_DEBUG_MESSAGE(ThorCXLibrary::DebugMessageSeverity::Info, ##__VA_ARGS__)


#define CHECK(Expr, Severity, ...)				\
if( !(Expr) )									\
{												\
	LOG_DEBUG_MESSAGE(Severity, ##__VA_ARGS__);	\
}

#define CHECK_ERR(Expr, ...)  CHECK(Expr, ThorCXLibrary::DebugMessageSeverity::Error,   ##__VA_ARGS__)
#define CHECK_WARN(Expr, ...) CHECK(Expr, ThorCXLibrary::DebugMessageSeverity::Warning, ##__VA_ARGS__)
#define CHECK_INFO(Expr, ...) CHECK(Expr, ThorCXLibrary::DebugMessageSeverity::Info,    ##__VA_ARGS__)

#define CHECK_THROW(Expr, ...)		\
if( !(Expr) )						\
{									\
	LOG_ERROR_AND_THROW(##__VA_ARGS__);\
}
