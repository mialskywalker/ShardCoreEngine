#pragma once

#include "windows.h"
#include <cstdint>
#include <memory>

#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class D3D12Module;
class ResourceModule;
class ShaderDescriptorModule;
class RenderTexture;
class Camera;
class DebugDraw;
class Scene;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd, uint32_t width, uint32_t height);

	void PreRender();
	void Render(Camera& camera, Scene* scene);
	void PostRender();

	void RequestSceneResize(uint32_t width, uint32_t height);

private:
	void RenderSceneToTexture(Camera& camera, Scene* scene);
	void DrawScene(Camera& camera, Scene* scene);
	void PrepareBackBuffer();

private:
	// Modules
	std::unique_ptr<D3D12Module>				m_D3D12Module;
	std::unique_ptr<ResourceModule>				m_ResourceModule;
	std::unique_ptr<ShaderDescriptorModule>		m_ShaderDescriptorModule;
	std::unique_ptr<RenderTexture>				m_SceneRenderTexture;
	std::unique_ptr<DebugDraw>					m_DebugDraw;

	uint32_t m_RequestedSceneWidth = 0;
	uint32_t m_RequestedSceneHeight = 0;
	
public:
	D3D12Module* GetD3D12() const { return m_D3D12Module.get(); }
	ResourceModule* GetResourceModule() const { return m_ResourceModule.get(); }
	ShaderDescriptorModule* GetShaderDescriptors() const { return m_ShaderDescriptorModule.get(); }
	RenderTexture* GetSceneRenderTexture() const { return m_SceneRenderTexture.get(); }
	DebugDraw* GetDebugDraw() const { return m_DebugDraw.get(); }
};
