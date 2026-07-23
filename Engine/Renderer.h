#pragma once

#include "windows.h"
#include <cstdint>
#include <memory>

class D3D12Module;
class ResourceModule;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd, uint32_t width, uint32_t height);

	void PreRender();
	void Render();
	void PostRender();

private:
	// Modules
	std::unique_ptr<D3D12Module>				m_D3D12Module;
	std::unique_ptr<ResourceModule>				m_ResourceModule;


	float										m_ClearColor[4];

public:
	D3D12Module* GetD3D12() const { return m_D3D12Module.get(); }

	bool TestUploadBuffer();
	bool TestDefaultBuffer();
};
