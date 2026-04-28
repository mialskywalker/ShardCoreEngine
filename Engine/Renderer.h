#pragma once
#include <windows.h>
#include <cstdint>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Renderer
{

public:
	Renderer();
	~Renderer();

	bool Init(HWND hwnd, uint32_t width, uint32_t height);
	void Render();

private:
	HWND m_Hwnd = nullptr;
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;

	ComPtr<IDXGIFactory6> m_Factory;
	ComPtr<IDXGIAdapter1> m_Adapter;
	ComPtr<ID3D12Device> m_Device;

	ComPtr<ID3D12CommandQueue> m_CommandQueue;

private:
	bool CreateFactory();
	bool PickAdapter();
	bool CreateDevice();

	bool CreateCommandQueue();
};