#include "EditorUI.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "WindowLogger.h"
#include "WindowHierarchy.h"

EditorUI::EditorUI() {}

EditorUI::~EditorUI() {}

bool EditorUI::Init(HWND hwnd, D3D12Module& d3d12, ShaderDescriptorModule& descriptors)
{
	ImGui::CreateContext();

	// Style Setup
	SetupStyle();
	SetupFonts();

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

	// init members
	m_WindowLogger = std::make_unique<WindowLogger>();
	m_WindowHierarchy = std::make_unique<WindowHierarchy>();

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

void EditorUI::Draw(Scene* activeScene)
{
	m_WindowLogger->Draw();
	m_WindowHierarchy->Draw(activeScene);
}

void EditorUI::SetupStyle()
{
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	// Sizes
	style.WindowRounding = 3.0f;
	style.ChildRounding = 3.0f;
	style.FrameRounding = 3.0f;
	style.PopupRounding = 3.0f;
	style.GrabRounding = 3.0f;

	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.FramePadding = ImVec2(8.0f, 5.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);

	// Colors
	ImVec4* colors = style.Colors;

	colors[ImGuiCol_WindowBg] =
		ImVec4(0.082f, 0.082f, 0.090f, 1.0f);

	colors[ImGuiCol_ChildBg] =
		ImVec4(0.141f, 0.141f, 0.153f, 1.0f);

	colors[ImGuiCol_FrameBg] =
		ImVec4(0.200f, 0.200f, 0.220f, 1.0f);

	colors[ImGuiCol_Button] =
		ImVec4(0.784f, 0.196f, 0.235f, 1.0f);

	colors[ImGuiCol_ButtonHovered] =
		ImVec4(0.894f, 0.282f, 0.314f, 1.0f);

	colors[ImGuiCol_ButtonActive] =
		ImVec4(0.650f, 0.140f, 0.180f, 1.0f);

	colors[ImGuiCol_CheckMark] =
		ImVec4(0.894f, 0.282f, 0.314f, 1.0f);

	colors[ImGuiCol_DockingPreview] =
		ImVec4(0.784f, 0.196f, 0.235f, 0.7f);

	// Window titles
	colors[ImGuiCol_TitleBg] =
		ImVec4(0.10f, 0.10f, 0.11f, 1.0f);

	colors[ImGuiCol_TitleBgActive] =
		ImVec4(0.15f, 0.15f, 0.16f, 1.0f);

	// Tabs
	colors[ImGuiCol_Tab] =
		ImVec4(0.13f, 0.13f, 0.14f, 1.0f);

	colors[ImGuiCol_TabHovered] =
		ImVec4(0.38f, 0.16f, 0.18f, 1.0f);

	colors[ImGuiCol_TabSelected] =
		ImVec4(0.22f, 0.22f, 0.24f, 1.0f);

	colors[ImGuiCol_TabSelectedOverline] =
		ImVec4(0.88f, 0.20f, 0.24f, 1.0f);

	// Hierarchy / Selectable
	colors[ImGuiCol_Header] =
		ImVec4(0.32f, 0.13f, 0.16f, 1.0f);

	colors[ImGuiCol_HeaderHovered] =
		ImVec4(0.45f, 0.17f, 0.20f, 1.0f);

	colors[ImGuiCol_HeaderActive] =
		ImVec4(0.62f, 0.19f, 0.23f, 1.0f);

	// Borders
	colors[ImGuiCol_Border] =
		ImVec4(0.23f, 0.23f, 0.25f, 0.65f);

	style.TabRounding = 3.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
}

void EditorUI::SetupFonts()
{
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF(
		"Assets/Fonts/Roboto-Regular.ttf"
	);

	ImGui::GetStyle().FontSizeBase = 15.0f;
}
