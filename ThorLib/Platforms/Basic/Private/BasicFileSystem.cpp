#include "../Public/BasicFileSystem.h"
#include "../../../ThorCommon/DebugUtils.h"
#include <algorithm>

ThorCXLibrary::String BasicFileSystem::m_strWorkingDirectory;

BasicFile::BasicFile(const FileOpenAttribs &OpenAttribs, ThorCXLibrary::Char SlashSymbol)
	:m_OpenAttribs(OpenAttribs),
	m_Path(BasicFileSystem::GetFullPath(OpenAttribs.strFilePath))
{
	BasicFileSystem::CorrectSlashes(m_Path, SlashSymbol);

	m_OpenAttribs.strFilePath = m_Path.c_str();
}

BasicFile::~BasicFile()
{

}

ThorCXLibrary::String BasicFile::GetOpenModeStr()
{
	std::string OpenModeStr;
	switch (m_OpenAttribs.AccessMode)
	{
	case EFileAccessMode::Read:	OpenModeStr += 'r'; break;
	case EFileAccessMode::Overwrite: OpenModeStr += 'w'; break;
	case EFileAccessMode::Append: OpenModeStr += 'a'; break;
	default: break;
	}
	// Always open file in binary mode. Text mode is platform-specific
	OpenModeStr += 'b';
	return OpenModeStr;
}

std::string BasicFileSystem::GetFullPath(const ThorCXLibrary::Char *strFilePath)
{
	std::string FullPath = m_strWorkingDirectory;
	auto len = FullPath.length();
	if (len > 0 && FullPath[len - 1] != '\\')
	{
		FullPath += '\\';
	}
	FullPath += strFilePath;
	return FullPath;
}

BasicFile* BasicFileSystem::OpenFile(FileOpenAttribs &OpenAttribs)
{
	return nullptr;
}

void BasicFileSystem::ReleaseFile(BasicFile *pFile)
{
	if (pFile)
		delete pFile;
}

bool BasicFileSystem::FileExists(const ThorCXLibrary::Char* strFilePath)
{
	return false;
}

ThorCXLibrary::Char BasicFileSystem::GetSlashSymbol()
{
	return 0;
}

void BasicFileSystem::CorrectSlashes(ThorCXLibrary::String &Path, ThorCXLibrary::Char SlashSymbol)
{
	VERIFY(SlashSymbol == '\\' || SlashSymbol == '/', "Incorrect slash symbol");
	ThorCXLibrary::Char RevSlash = (SlashSymbol == '\\') ? '/' : '\\';
	std::replace(Path.begin(), Path.end(), RevSlash, SlashSymbol);
}

void BasicFileSystem::SplitFilePath(const ThorCXLibrary::String& FullName, ThorCXLibrary::String *Path, ThorCXLibrary::String *Name)
{
	auto LastSlashPos = FullName.find_last_of("/\\");
	if (Path)
	{
		if (LastSlashPos != ThorCXLibrary::String::npos)
			*Path = FullName.substr(0, LastSlashPos);
		else
			*Path = "";
	}
	if (Name)
	{
		if (LastSlashPos != ThorCXLibrary::String::npos)
			*Name = FullName.substr(LastSlashPos + 1);
		else
			*Name = FullName;
	}
}