#pragma once

#include <Windows.h>
#include <memory>
#include <filesystem>
#include "d3dx12.h"
#include "imgui.h"
#include "WindowViewport.h"
#include "EditorSettings.h"
#include "EditorGizmo.h"

class D3D12Module;
class ShaderDescriptorModule;
class Scene;
class WindowLogger;
class WindowHierarchy;
class WindowInspector;
class Camera;
class FileSystem;

enum class EditorAction
{
	None,
	SaveScene,
	LoadScene
};

class EditorUI
{
public:
	struct EditorRequest
	{
		EditorAction Action = EditorAction::None;
		std::filesystem::path Path;
	};

	EditorUI();
	~EditorUI();

	bool Init(HWND hwnd, D3D12Module& d3d12, ShaderDescriptorModule& descriptors);
	void BeginFrame();
	void Render(ID3D12GraphicsCommandList* commandList, ShaderDescriptorModule& descriptors);
	void Shutdown();

public:
	void Draw(Scene* activeScene, RenderTexture* renderTexture, Camera& camera, const FileSystem& fileSystem, const std::filesystem::path& currentScenePath);

	bool IsSceneHovered() const { return m_WindowViewport.IsHovered(); }
	uint32_t GetViewportWidth() const { return m_WindowViewport.GetWidth(); }
	uint32_t GetViewportHeight() const { return m_WindowViewport.GetHeight(); }
	const EditorSettings& GetSettings() const { return m_Settings; }

	EditorRequest ConsumePendingRequest();

	void ClearSelection();

private:
	void SetupStyle();
	void SetupFonts();
	void DrawFileMenu(Scene* activeScene, const FileSystem& fileSystem,	const std::filesystem::path& currentScenePath);

private:
	std::unique_ptr<WindowLogger>		m_WindowLogger;
	std::unique_ptr<WindowHierarchy>	m_WindowHierarchy;
	std::unique_ptr<WindowInspector>	m_WindowInspector;
	WindowViewport						m_WindowViewport;
	EditorSettings						m_Settings;
	EditorGizmo							m_EditorGizmo;

	EditorRequest m_PendingRequest;
	char m_SaveSceneName[128] = {};
	std::filesystem::path m_SelectedLoadPath;
};