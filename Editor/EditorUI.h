#pragma once

#include <Windows.h>
#include "d3dx12.h"
#include "imgui.h"

class D3D12Module;
class ShaderDescriptorModule;

class EditorUI
{
public:
	EditorUI();
	~EditorUI();

	bool Init(HWND hwnd, D3D12Module& d3d12, ShaderDescriptorModule& descriptors);
	void BeginFrame();
	void Render(ID3D12GraphicsCommandList* commandList, ShaderDescriptorModule& descriptors);
	void Shutdown();
};