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

//在每个平台的特定cpp文件内定义在DebugOutput.h内的extern声明
//这里是Win平台，使用WinSDK的OutputDebugString输出
namespace ThorCXLibrary
{
	DebugMessageCallbackType DebugMessageCallback = WindowsDebug::OutputDebugMessage;
}