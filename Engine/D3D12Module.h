#pragma once

#include "Module.h"
#include "windows.h"
#include <cstdint>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class D3D12Module : public Module
{
public:
	D3D12Module() {}
	~D3D12Module() {}

	bool Init() override;
	void PreRender() override;
	void PostRender() override;

private:
	HWND m_Hwnd = nullptr;
	uint32_t m_Width = 1280;
	uint32_t m_Height = 720;

	// DirectX 12
	ComPtr<ID3D12Debug> m_DebugController;
	ComPtr<IDXGIFactory6> m_Factory;
	ComPtr<IDXGIAdapter4> m_Adapter;
	ComPtr<ID3D12Device4> m_Device;

	uint32_t m_CurrentFrameIndex = 0;
	static const uint32_t FrameCount = 2;

	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocators[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	ComPtr<ID3D12Resource> m_BackBuffers[FrameCount];
	ComPtr<IDXGISwapChain4> m_SwapChain;

	ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	uint32_t m_RTVDescriptorSize = 0;

	ComPtr<ID3D12Fence> m_Fence;
	uint64_t m_FenceValue = 0;
	HANDLE m_FenceEvent;

private:
	bool EnableDebugLayer();
	bool CreateFactory();
	bool PickAdapter();
	bool CreateDevice();

	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateRTVDescriptorHeap();
	bool CreateRTV();

	bool CreateCommandAllocators();
	bool CreateCommandList();

	bool CreateFence();
	void WaitForGPU();
};