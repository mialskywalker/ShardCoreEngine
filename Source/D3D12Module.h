#pragma once

#include "Module.h"
#include <dxgi1_6.h>

class D3D12Module : public Module
{
public:

	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;

private:

	HWND hWnd = NULL;

	ComPtr<ID3D12Debug> debugLayer;
	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;
	ComPtr<ID3D12InfoQueue> infoQueue;

public:

	IDXGIFactory6* getFactory() { return factory.Get(); }
	IDXGIAdapter4* getAdapter() { return adapter.Get(); }
	ID3D12Device5* getDevice() { return device.Get(); }

private:

	void enableDebugLayer();
	bool createFactory();
	bool createDevice();
	void setupInfoQueue();
};