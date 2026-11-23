#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

class FileSystem
{
	struct FileEntry
	{
		std::filesystem::path Path;
		bool IsDirectory = false;
	};

public:
	bool Init(const std::filesystem::path& projectRoot);

	std::filesystem::path ResolvePath(const std::filesystem::path& path) const;

	bool Exists(const std::filesystem::path& path) const;

	bool CreateDirectories(const std::filesystem::path& path) const;

    bool ReadTextFile(const std::filesystem::path& path, std::string& outText) const;

    bool WriteTextFile(const std::filesystem::path& path, const std::string& text) const;

    bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& outData) const;

    bool WriteBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) const;

	bool ListDirectory(const std::filesystem::path& directory, std::vector<FileEntry>& outEntries) const;

	const std::filesystem::path& GetProjectRoot() const { return m_ProjectRoot; }

private:
	std::filesystem::path m_ProjectRoot;
};