#pragma once

#include "Module.h"
#include <dxgi1_6.h>

class D3D12Module : public Module
{
public:

	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;
	void preRender() override;
	void postRender() override;
	void flush();
	void resize();

private:

	HWND									hWnd = NULL;

	ComPtr<ID3D12Debug>						debugLayer;
	ComPtr<IDXGIFactory6>					factory;
	ComPtr<IDXGIAdapter4>					adapter;
	ComPtr<ID3D12Device5>					device;
	ComPtr<ID3D12InfoQueue>					infoQueue;

	ComPtr<ID3D12CommandQueue>				commandQueue;
	ComPtr<IDXGISwapChain4>					swapChain;
	ComPtr<ID3D12CommandAllocator>			commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList4>		commandList;

	ComPtr<ID3D12DescriptorHeap>			RTVdescriptorHeap;
	ComPtr<ID3D12DescriptorHeap>			DSVdescriptorHeap;
	ComPtr<ID3D12Resource>					backBuffers[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12Resource>					depthStencilBuffer;
	ComPtr<ID3D12Fence>						fence;

	HANDLE									fenceEvent = NULL;

	unsigned								currentBufferIndex = 0;
	unsigned								windowWidth = 0;
	unsigned								windowHeight = 0;
	uint64_t								fenceValues[FRAMES_IN_FLIGHT] = { 0, 0, 0 };
	uint64_t								fenceValue = 0;

	bool									vsync = false;

public:

	IDXGIFactory6* getFactory() { return factory.Get(); }
	IDXGIAdapter4* getAdapter() { return adapter.Get(); }
	ID3D12Device5* getDevice() { return device.Get(); }
	ID3D12CommandQueue* getCommandQueue() { return commandQueue.Get(); }
	ID3D12CommandAllocator* getCurrentCommandAllocator() { return commandAllocators[currentBufferIndex].Get(); }
	ID3D12GraphicsCommandList4* getCommandList() { return commandList.Get(); }
	ID3D12Resource* getCurrentBackBuffer() { return backBuffers[currentBufferIndex].Get(); }
	HWND getHwnd() { return hWnd; }

	bool getVSYNC() { return vsync; }
	const unsigned& getcurrentBackBufferIndex() const { return currentBufferIndex; }
	unsigned getWindowWidth() { return windowWidth; }
	unsigned getWindowHeight() { return windowHeight; }

	D3D12_CPU_DESCRIPTOR_HANDLE getRTVCPUDescriptorHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE getDSVCPUDescriptorHandle();

	void setVSYNC(bool v) { vsync = v; }

private:

	void enableDebugLayer();
	bool createFactory();
	bool createDevice();
	void setupInfoQueue();
	bool createCommandQueue();
	bool createSwapChain();
	bool createRTVDescriptorHeap();
	void createRTV();
	bool createCommandAllocators();
	bool createCommandList();
	bool createFence();
	bool createDepthStencilBuffer();
	bool createDSVDescriptorHeap();
	void createDSV();

	void getWindowSize(unsigned& width, unsigned& height);
};