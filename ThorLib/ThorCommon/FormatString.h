#pragma once

#include <sstream>



namespace ThorCXLibrary
{
	template<typename SSType>
	void FormatStrSS(SSType &ss)
	{
	}

	template<typename SSType, typename ArgType>
	void FormatStrSS(SSType &ss, const ArgType& Arg)
	{
		ss << Arg;
	}

	//模板递归调用展开
	template<typename SSType, typename FirstArgType, typename... RestArgsType>
	void FormatStrSS(SSType &ss, const FirstArgType& FirstArg, const RestArgsType&... RestArgs)
	{
		FormatStrSS(ss, FirstArg);
		FormatStrSS(ss, RestArgs...);
	}

	template<typename... RestArgsType>
	std::string FormatString(const RestArgsType&... Args)
	{
		std::stringstream ss;
		FormatStrSS(ss, Args...);
		return ss.str();
	}

}