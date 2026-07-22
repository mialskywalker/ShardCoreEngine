#pragma once
#include "windows.h"
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

	void PreRender();
	void Render();
	void PostRender();

private:
	HWND										m_Hwnd = nullptr;
	uint32_t									m_Width = 1280;
	uint32_t									m_Height = 720;
	static const uint32_t						m_FramesInFlight = 2;

	// DirectX 12
	ComPtr<ID3D12Debug>							m_DebugInterface;
	ComPtr<IDXGIFactory6>						m_Factory;
	ComPtr<IDXGIAdapter4>						m_Adapter;
	ComPtr<ID3D12Device5>						m_Device;

	ComPtr<ID3D12InfoQueue>						m_InfoQueue;

	ComPtr<ID3D12CommandQueue>					m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>				m_CommandAllocators[2];
	ComPtr<ID3D12GraphicsCommandList>			m_CommandList;

	ComPtr<ID3D12Resource>						m_BackBuffers[m_FramesInFlight];
	ComPtr<IDXGISwapChain4>						m_SwapChain;

	ComPtr<ID3D12DescriptorHeap>				m_RTVDescriptorHeap;	

	uint32_t									m_CurrentFrameIndex = 0;
	uint32_t									m_RTVDescriptorSize = 0;

	ComPtr<ID3D12Fence>							m_Fence;
	uint64_t									m_FenceValue = 0;
	HANDLE										m_FenceEvent;

private:
	void EnableDebugLayer();
	void EnableInfoQueue();
	
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateDescriptorHeaps();
	bool CreateRenderTargetViews();

	bool CreateCommandAllocators();
	bool CreateCommandList();

	bool CreateFence();
	void WaitForGPU();
};
