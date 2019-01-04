#pragma once

#include "FormatString.h"
#include "LogUtils.h"
#include "../Platforms/Basic/Public/BasicPlatformDebug.h"

#ifdef _DEBUG
#include <typeinfo>

#define ASSERTION_FAILED(Message, ...)\
auto msg = ThorCXLibrary::FormatString(Message, ##__VA_ARGS__);\
DebugAssertionFailed(msg.c_str(), __FUNCTION__, __FILE__, __LINE__);

#define VERIFY(Expr, Message, ...)	\
if( !(Expr))							\
{										\
	ASSERTION_FAILED(Message, ##__VA_ARGS__);\
}

#define UNEXPECTED		ASSERTION_FAILED
#define UNSUPPORTED		ASSERTION_FAILED

#define VERIFY_EXPR(Expr) VERIFY(Expr, "Debug expression failed:\n", #Expr)



#else

#   define CHECK_DYNAMIC_TYPE(...)
#   define VERIFY(...)
#   define UNEXPECTED(...)
#   define UNSUPPORTED(...)
#   define VERIFY_EXPR(...)

#endif