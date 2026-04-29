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
	ComPtr<IDXGISwapChain4> m_SwapChain;

	uint32_t m_CurrentFrameIndex = 0;
	static const uint32_t FrameCount = 2;

	ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	ComPtr<ID3D12Resource> m_BackBuffers[FrameCount];

	uint32_t m_RTVDescriptorSize = 0;

	ComPtr<ID3D12CommandAllocator> m_CommandAllocators[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	ComPtr<ID3D12Fence> m_Fence;
	uint64_t m_FenceValue = 0;
	HANDLE m_FenceEvent;

	D3D12_VIEWPORT m_Viewport = {};
	D3D12_RECT m_ScissorRect = {};

private:
	bool CreateFactory();
	bool PickAdapter();
	bool CreateDevice();

	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateRenderTargets();

	bool CreateCommandAllocators();
	bool CreateCommandList();

	bool CreateFence();

	void WaitForGPU();
};