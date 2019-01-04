#include "../Public/BasicPlatformDebug.h"

using namespace ThorCXLibrary;

String BasicPlatformDebug::FormatDebugMessage(DebugMessageSeverity Severity,
	const Char* Message,
	const char* Function,
	const char* File,
	int Line)
{
	std::stringstream msg_ss;
	static const Char* const strSeverities[] = { "Info", "Warning", "ERROR", "CRITICAL ERROR" };
	const auto* MessageSevery = strSeverities[static_cast<int>(Severity)];

	msg_ss << "Thorcx Render Engine: " << MessageSevery;
	if (Function != nullptr || File != nullptr)
	{
		msg_ss << " in ";
		if (Function != nullptr)
		{
			msg_ss << Function << "()";
			if (File != nullptr)
				msg_ss << " (";
		}
		if (File != nullptr)
		{
			msg_ss << File << ", " << Line << ")";
		}
	}
	msg_ss << ":" << Message << '\n';
	return msg_ss.str();
}