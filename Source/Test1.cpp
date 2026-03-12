#include "Common.h"
#include "Test1.h"

#include "Application.h"
#include "D3D12Module.h"
#include "EditorModule.h"

bool Test1::init()
{
	imgui = app->getEditor()->getImGui();
	return true;
}

void Test1::render()
{
	D3D12Module* d3d12 = app->getD3D12();
	ID3D12GraphicsCommandList4* commandList = d3d12->getCommandList();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRTVCPUDescriptorHandle();
	commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

	float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtv, color, 0, nullptr);

	imgui->record(commandList);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	commandList->ResourceBarrier(1, &barrier);

}