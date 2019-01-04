#include "DebugOutput.h"

namespace ThorCXLibrary
{
	void SetDebugMessageCallback(DebugMessageCallbackType DbgFunctionSignature)
	{
		// DebugMessageCallback variable is defined in every platform-specific implementation file
		// and initialized with the platform-specific callback
		DebugMessageCallback = DbgFunctionSignature;
	}
}