#include "FileSystem.h"
#include "Logger.h"
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

bool FileSystem::IsRegularFile(const std::filesystem::path& path) const
{
	if (path.empty())
	{
		return false;
	}

	std::filesystem::path resolvedPath = ResolvePath(path);
	std::error_code ec;
	bool ok = std::filesystem::is_regular_file(resolvedPath, ec);

	return !ec && ok;
}

bool FileSystem::RemoveFile(const std::filesystem::path& path) const
{
	if (path.empty() || !IsInsideProject(path))
	{
		return false;
	}

	std::filesystem::path resolvedPath = ResolvePath(path);

	if (!IsRegularFile(resolvedPath))
	{
		return false;
	}

	std::error_code ec;

	if (std::filesystem::is_symlink(resolvedPath, ec) || ec)
	{
		return false;
	}

	bool removed = std::filesystem::remove(resolvedPath, ec);

	if (ec)
	{
		Logger::Error(
			"[FileSystem] Failed to delete file: " +
			resolvedPath.string() + " | " + ec.message(),
			LogType::Engine);

		return false;
	}

	return removed;
}

bool FileSystem::RelocateFile(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationPath) const
{
	if (sourcePath.empty() || destinationPath.empty())
	{
		return false;
	}

	if (!IsInsideProject(sourcePath) ||	!IsInsideProject(destinationPath))
	{
		return false;
	}

	std::filesystem::path resolvedSource = ResolvePath(sourcePath);
	std::filesystem::path resolvedDestination =	ResolvePath(destinationPath);

	if (resolvedSource == resolvedDestination)
	{
		return false;
	}

	if (!IsRegularFile(resolvedSource))
	{
		return false;
	}

	std::error_code ec;

	// Validate source symlink
	if (std::filesystem::is_symlink(resolvedSource, ec) || ec)
	{
		return false;
	}

	// Check whether destination already exists
	auto destinationStatus = std::filesystem::symlink_status(resolvedDestination, ec);

	if (ec && ec != std::errc::no_such_file_or_directory)
	{
		return false;
	}

	if (destinationStatus.type() !=	std::filesystem::file_type::not_found)
	{
		return false;
	}

	ec.clear();

	// Validate destination directory
	if (!std::filesystem::is_directory(resolvedDestination.parent_path(), ec) || ec)
	{
		return false;
	}

	// Move or rename the file
	std::filesystem::rename(resolvedSource,	resolvedDestination, ec);

	if (ec)
	{
		Logger::Error(
			"[FileSystem] Failed to move file: " +
			resolvedSource.string() + " -> " +
			resolvedDestination.string() + " | " +
			ec.message(),
			LogType::Engine);

		return false;
	}

	return true;
}

bool FileSystem::IsInsideProject(const std::filesystem::path& path) const
{
	if (path.empty())
	{
		return false;
	}

	std::error_code ec;

	auto resolvedPath = std::filesystem::weakly_canonical(ResolvePath(path), ec);

	if (ec)
	{
		return false;
	}

	auto projectRoot = std::filesystem::weakly_canonical(m_ProjectRoot, ec);

	if (ec)
	{
		return false;
	}

	auto relativePath = resolvedPath.lexically_relative(projectRoot);

	if (relativePath.empty() ||
		relativePath == "." ||
		*relativePath.begin() == "..")
	{
		return false;
	}

	return true;
}
