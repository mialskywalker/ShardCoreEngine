#include "EditorUI.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

EditorUI::EditorUI() {}

EditorUI::~EditorUI() {}

bool EditorUI::Init(HWND hwnd, D3D12Module& d3d12, ShaderDescriptorModule& descriptors)
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	if (!ImGui_ImplWin32_Init(hwnd))
	{
		ImGui::DestroyContext();
		return false;
	}

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = d3d12.GetDevice();
	initInfo.CommandQueue = d3d12.GetCommandQueue();
	initInfo.NumFramesInFlight = d3d12.GetFramesInFlight();
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
	initInfo.SrvDescriptorHeap = descriptors.GetHeap();
	initInfo.UserData = &descriptors;

	initInfo.SrvDescriptorAllocFn = [](
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE* outCpu,
		D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
		{
			auto* module = static_cast<ShaderDescriptorModule*>(info->UserData);

			int index = module->AllocateDescriptor();

			if (index < 0)
			{
				IM_ASSERT(false && "Descriptor Heap is full!");
				*outCpu = {};
				*outGpu = {};
				return;
			}

			*outCpu = module->GetCPUHandle(index);
			*outGpu = module->GetGPUHandle(index);
		};

	initInfo.SrvDescriptorFreeFn = [](
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		{
			auto* module = static_cast<ShaderDescriptorModule*>(info->UserData);

			SIZE_T start = module->GetCPUHandle(0).ptr;
			SIZE_T offset = cpuHandle.ptr - start;

			UINT index = static_cast<UINT>(offset / module->GetDescriptorSize());

			module->FreeDescriptor(index);
		};

	if (!ImGui_ImplDX12_Init(&initInfo))
	{
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	return true;
}

void EditorUI::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void EditorUI::Render(ID3D12GraphicsCommandList* commandList, ShaderDescriptorModule& descriptors)
{
	ImGui::Render();

	ID3D12DescriptorHeap* heaps[] = { descriptors.GetHeap() };
	commandList->SetDescriptorHeaps(1, heaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

}

void EditorUI::Shutdown()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}