#pragma once

#include <memory>
#include <filesystem>

class FileSystem;
class Scene;

class SceneSerializer
{
public:
	explicit SceneSerializer(const FileSystem& fs);
	~SceneSerializer() = default;

	const bool SaveScene(const Scene& scene, const std::filesystem::path& path);
	std::unique_ptr<Scene> LoadScene(const std::filesystem::path& path);

private:
	const FileSystem& m_FileSystem;
};