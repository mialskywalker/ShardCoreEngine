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
class Camera;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd, uint32_t width, uint32_t height);

	void PreRender();
	void Render(Camera& camera);
	void PostRender();

private:
	// Modules
	std::unique_ptr<D3D12Module>				m_D3D12Module;
	std::unique_ptr<ResourceModule>				m_ResourceModule;
	std::unique_ptr<ShaderDescriptorModule>		m_ShaderDescriptorModule;
	
public:
	D3D12Module* GetD3D12() const { return m_D3D12Module.get(); }
	ResourceModule* GetResourceModule() const { return m_ResourceModule.get(); }
	ShaderDescriptorModule* GetShaderDescriptors() const { return m_ShaderDescriptorModule.get(); }

private:
	ComPtr<ID3D12Resource>						m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					vbv;
};
