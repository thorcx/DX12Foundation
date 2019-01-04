#pragma once

#include <vector>
#include "../../../ThorCommon/BasicTypes.h"

enum class EFileAccessMode
{
	Read,
	Overwrite,
	Append
};

enum class FilePosOrigin
{
	Start,
	Curr,
	End
};

struct FileOpenAttribs
{
	const ThorCXLibrary::Char *strFilePath;
	EFileAccessMode AccessMode;
	FileOpenAttribs(const ThorCXLibrary::Char *Path = nullptr,
		EFileAccessMode Access = EFileAccessMode::Read) :
		strFilePath(Path),
		AccessMode(Access)
	{}
};

class BasicFile
{
public:
	BasicFile(const FileOpenAttribs &OpenAttribs, ThorCXLibrary::Char SlashSymbol);
	virtual ~BasicFile();

	size_t GetSize();

	const ThorCXLibrary::String& GetPath() { return m_Path; }

protected:

	ThorCXLibrary::String GetOpenModeStr();
	FileOpenAttribs m_OpenAttribs;
	ThorCXLibrary::String m_Path;
};

struct FindFileData
{
	virtual const ThorCXLibrary::Char* Name() const = 0;
	virtual bool IsDirectory() const = 0;
	virtual ~FindFileData() {}
};

struct BasicFileSystem
{
	static BasicFile* OpenFile(FileOpenAttribs &OpenAttribs);
	static void ReleaseFile(BasicFile*);

	static std::string GetFullPath(const ThorCXLibrary::Char *strFilePath);

	static bool FileExists(const ThorCXLibrary::Char *strFilePath);

	static void SetWorkingDirectory(const ThorCXLibrary::Char *strWorkingDir) { m_strWorkingDirectory = strWorkingDir; }
	static const ThorCXLibrary::String &GetWorkingDirectory() { return m_strWorkingDirectory; }

	static ThorCXLibrary::Char GetSlashSymbol();
	static void CorrectSlashes(ThorCXLibrary::String &Path, ThorCXLibrary::Char SlashSymbol);

	static void SplitFilePath(const ThorCXLibrary::String& FullName,
		ThorCXLibrary::String *Path,
		ThorCXLibrary::String *Name);

protected:
	static ThorCXLibrary::String m_strWorkingDirectory;
};