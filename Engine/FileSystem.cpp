#include "FileSystem.h"
#include <fstream>
#include <algorithm>

bool FileSystem::Init(const std::filesystem::path& projectRoot)
{
	if (projectRoot.empty())
	{
		return false;
	}

	std::error_code ec;
	m_ProjectRoot = std::filesystem::absolute(projectRoot, ec).lexically_normal();

	return !ec;
}

std::filesystem::path FileSystem::ResolvePath(const std::filesystem::path& path) const
{
	if (path.is_absolute())
	{
		return path.lexically_normal();
	}

	return (m_ProjectRoot / path).lexically_normal();
}

bool FileSystem::Exists(const std::filesystem::path& path) const
{
	std::error_code ec;

	return std::filesystem::exists(ResolvePath(path), ec) && !ec;
}

bool FileSystem::CreateDirectories(const std::filesystem::path& path) const
{
	std::filesystem::path resolvedPath = ResolvePath(path);
	std::error_code ec;
	std::filesystem::create_directories(resolvedPath, ec);

	if (ec)
	{
		return false;
	}

	return std::filesystem::is_directory(resolvedPath, ec) && !ec;
}

bool FileSystem::ReadTextFile(const std::filesystem::path& path, std::string& outText) const
{
	outText.clear();

	std::ifstream file(ResolvePath(path), std::ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	outText.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	return !file.bad();
}

bool FileSystem::WriteTextFile(const std::filesystem::path& path, const std::string& text) const
{
	auto resolvedPath = ResolvePath(path);
	if (!CreateDirectories(resolvedPath.parent_path()))
	{
		return false;
	}

	std::ofstream file(resolvedPath, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		return false;
	}

	file.write(text.data(), text.size());

	return file.good();
}

bool FileSystem::ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& outData) const
{
	outData.clear();

	std::ifstream file(ResolvePath(path), std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		return false;
	}

	std::streamsize size = file.tellg();
	if (size < 0)
	{
		return false;
	}

	file.seekg(0, std::ios::beg);
	outData.resize(static_cast<size_t>(size));

	if (size > 0)
	{
		if (!file.read(reinterpret_cast<char*>(outData.data()),	size))
		{
			outData.clear();
			return false;
		}
	}

	return true;
}

bool FileSystem::WriteBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) const
{
	auto resolvedPath = ResolvePath(path);
	if (!CreateDirectories(resolvedPath.parent_path()))
	{
		return false;
	}

	std::ofstream file(resolvedPath, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		return false;
	}

	if (!data.empty())
	{
		file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	}

	return file.good();
}

bool FileSystem::ListDirectory(const std::filesystem::path& directory, std::vector<FileEntry>& outEntries) const
{
	outEntries.clear();

	std::filesystem::path resolvedPath = ResolvePath(directory);

	std::error_code ec;

	if (!std::filesystem::is_directory(resolvedPath, ec))
	{
		return false;
	}

	for (const auto& entry :
		std::filesystem::directory_iterator(resolvedPath, ec))
	{
		if (ec)
		{
			break;
		}

		FileEntry fileEntry;
		fileEntry.Path = entry.path().lexically_relative(m_ProjectRoot);
		fileEntry.IsDirectory = entry.is_directory(ec);

		if (ec)
		{
			break;
		}

		outEntries.push_back(fileEntry);
	}

	if (ec)
	{
		outEntries.clear();
		return false;
	}

	// Folders first, then files
	std::sort(outEntries.begin(), outEntries.end(),
		[](const FileEntry& a, const FileEntry& b)
		{
			if (a.IsDirectory != b.IsDirectory)
			{
				return a.IsDirectory;
			}

			return a.Path.filename() < b.Path.filename();
		}
	);

	return true;
}

bool FileSystem::ListFilesRecursive(const std::filesystem::path& directory, std::vector<std::filesystem::path>& outFiles) const
{
	outFiles.clear();

	if (directory.empty())
	{
		return false;
	}

	std::filesystem::path resolvedPath = ResolvePath(directory);
	std::error_code ec;
	
	if (!std::filesystem::is_directory(resolvedPath, ec))
	{
		return false;
	}

	std::filesystem::recursive_directory_iterator it(resolvedPath, ec);
	std::filesystem::recursive_directory_iterator end;

	if (ec)
	{
		return false;
	}

	while (it != end)
	{
		bool isSymlink = it->is_symlink(ec);

		if (ec)
		{
			outFiles.clear();
			return false;
		}

		if (!isSymlink)
		{
			bool isFile = it->is_regular_file(ec);

			if (ec)
			{
				outFiles.clear();
				return false;
			}

			if (isFile)
			{
				std::filesystem::path relativePath = it->path().lexically_relative(m_ProjectRoot);

				if (relativePath.empty() || *relativePath.begin() == "..")
				{
					outFiles.clear();
					return false;
				}

				outFiles.push_back(relativePath);
			}
		}

		it.increment(ec);

		if (ec)
		{
			outFiles.clear();
			return false;
		}
	}

	std::sort(outFiles.begin(), outFiles.end());

	return true;
}
