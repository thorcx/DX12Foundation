#pragma once

#include "BasicTypes.h"

namespace ThorCXLibrary
{
	enum class DebugMessageSeverity : Int32
	{
		/// Information message
		Info = 0,

		/// Warning message
		Warning,

		/// Error, with potential recovery
		Error,

		/// Fatal error - recovery is not possible
		FatalError
	};

	/// Type of the debug message callback function

/// \param [in] Severity - Message severity
/// \param [in] Message - Debug message
/// \param [in] Function - Name of the function or nullptr
/// \param [in] Function - File name or nullptr
/// \param [in] Line - Line number
	using DebugMessageCallbackType = void(*)(DebugMessageSeverity Severity, const Char* Message, const char* Function, const char* File, int Line);

	
	extern DebugMessageCallbackType DebugMessageCallback;


	//ÿ��ʹ�ÿ���඼��Ҫ����ʵ��һ��DebugMessageCallbackType��ĺ���������Debug���
	void SetDebugMessageCallback(DebugMessageCallbackType DbgFunctionSignature);
}