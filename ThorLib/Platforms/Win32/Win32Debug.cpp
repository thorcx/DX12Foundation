#include "Win32Debug.h"
#include <Windows.h>

using namespace ThorCXLibrary;

void WindowsDebug::OutputDebugMessage(ThorCXLibrary::DebugMessageSeverity Severity, const ThorCXLibrary::Char* Message, const char *Function, const char *File, int Line)
{
	auto msg = FormatDebugMessage(Severity, Message, Function, File, Line);
	OutputDebugStringA(msg.c_str());

	if (Severity == DebugMessageSeverity::Error || Severity == DebugMessageSeverity::FatalError)
		std::cerr << msg;
	else
		std::cout << msg;
}

//��ÿ��ƽ̨���ض�cpp�ļ��ڶ�����DebugOutput.h�ڵ�extern����
//������Winƽ̨��ʹ��WinSDK��OutputDebugString���
namespace ThorCXLibrary
{
	DebugMessageCallbackType DebugMessageCallback = WindowsDebug::OutputDebugMessage;
}