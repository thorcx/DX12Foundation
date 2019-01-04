#include "Win32Debug.h"
#include "../../ThorCommon/FormatString.h"
#include <Windows.h>
#include <csignal>
#include <iostream>
using namespace ThorCXLibrary;

void WindowsDebug::AssertionFailed(const ThorCXLibrary::Char *Message, const char *Function, const char *File, int Line)
{
	auto AssertionFailedMessage = FormatAssertionFailedMessage(Message, Function, File, Line);
	OutputDebugMessage(DebugMessageSeverity::Error, AssertionFailedMessage.c_str(), nullptr, nullptr, 0);

	int nCode = MessageBoxA(NULL, AssertionFailedMessage.c_str(), "Runtime assertion failed",
							MB_TASKMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
	// Abort: abort the program
	if (nCode == IDABORT)
	{
		// raise abort signal
		raise(SIGABRT);

		// We usually won't get here, but it's possible that
		//  SIGABRT was ignored.  So exit the program anyway.
		exit(3);
	}

	// Retry: call the debugger
	if (nCode == IDRETRY)
	{
		DebugBreak();
		// return to user code
		return;
	}

	// Ignore: continue execution
	if (nCode == IDIGNORE)
		return;
}

void WindowsDebug::OutputDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity, const ThorCXLibrary::Char* Message, const char *Function, const char *File, int Line)
{
	auto msg = FormatDebugMessage(Severity, Message, Function, File, Line);
	OutputDebugStringA(msg.c_str());

	if (Severity == DebugMessageSeverity::Error || Severity == DebugMessageSeverity::FatalError)
		std::cerr << msg;
	else
		std::cout << msg;
}

void DebugAssertionFailed(const ThorCXLibrary::Char* Message, const char* Function, const char* File, int Line)
{
	WindowsDebug::AssertionFailed(Message, Function, File, Line);
}

//��ÿ��ƽ̨���ض�cpp�ļ��ڶ�����DebugOutput.h�ڵ�extern����
//������Winƽ̨��ʹ��WinSDK��OutputDebugString���
namespace ThorCXLibrary
{
	DebugMessageCallbackType DebugMessageCallback = WindowsDebug::OutputDebugMessage;
}