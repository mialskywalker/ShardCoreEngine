#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

class FileSystem
{
public:
	struct FileEntry
	{
		std::filesystem::path Path;
		bool IsDirectory = false;
	};

	bool Init(const std::filesystem::path& projectRoot);

	std::filesystem::path ResolvePath(const std::filesystem::path& path) const;

	bool Exists(const std::filesystem::path& path) const;

	bool CreateDirectories(const std::filesystem::path& path) const;

    bool ReadTextFile(const std::filesystem::path& path, std::string& outText) const;

    bool WriteTextFile(const std::filesystem::path& path, const std::string& text) const;

    bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& outData) const;

    bool WriteBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) const;

	bool ListDirectory(const std::filesystem::path& directory, std::vector<FileEntry>& outEntries) const;

	bool ListFilesRecursive(const std::filesystem::path& directory, std::vector<std::filesystem::path>& outFiles) const;

	bool IsRegularFile(const std::filesystem::path& path) const;

	bool RemoveFile(const std::filesystem::path& path) const;

	bool RelocateFile(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationPath) const;

	const std::filesystem::path& GetProjectRoot() const { return m_ProjectRoot; }

private:
	bool IsInsideProject(const std::filesystem::path& path) const;

private:
	std::filesystem::path m_ProjectRoot;
};