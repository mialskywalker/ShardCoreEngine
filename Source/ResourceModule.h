#pragma once

#include "Module.h"
#include <filesystem>

class ResourceModule : public Module
{
public:

	ResourceModule();
	~ResourceModule();

public:

	bool init() override;

	ComPtr<ID3D12Resource> createUploadBuffer(size_t bufferSize, void* data);

private:

	ComPtr<ID3D12GraphicsCommandList4> commandList;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	
};