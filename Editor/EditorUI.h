#pragma once

#include <Windows.h>
#include <memory>
#include "d3dx12.h"
#include "imgui.h"

class D3D12Module;
class ShaderDescriptorModule;
class Scene;
class WindowLogger;
class WindowHierarchy;
class WindowInspector;
class MeshLibrary;

class EditorUI
{
public:
	EditorUI();
	~EditorUI();

	bool Init(HWND hwnd, D3D12Module& d3d12, ShaderDescriptorModule& descriptors);
	void BeginFrame();
	void Render(ID3D12GraphicsCommandList* commandList, ShaderDescriptorModule& descriptors);
	void Shutdown();

public:
	void Draw(Scene* activeScene, MeshLibrary* meshLibrary);

private:
	void SetupStyle();
	void SetupFonts();

private:
	std::unique_ptr<WindowLogger>		m_WindowLogger;
	std::unique_ptr<WindowHierarchy>	m_WindowHierarchy;
	std::unique_ptr<WindowInspector>	m_WindowInspector;
};