#include "EditorUI.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "ImGuizmo.h"
#include "Camera.h"
#include "Scene.h"
#include <utility>
#include "FileSystem.h"
#include <cstdio>
#include <string>

#include "WindowLogger.h"
#include "WindowHierarchy.h"
#include "WindowInspector.h"

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
	m_WindowInspector = std::make_unique<WindowInspector>();

	return true;
}

void EditorUI::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
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

void EditorUI::Draw(Scene* activeScene, RenderTexture* renderTexture, Camera& camera, const FileSystem& fileSystem, const std::filesystem::path& currentScenePath)
{
    DrawFileMenu(activeScene, fileSystem, currentScenePath);

	m_WindowLogger->Draw();
	m_WindowHierarchy->Draw(activeScene);
	m_WindowInspector->Draw(activeScene, m_WindowHierarchy->GetSelectedUID());

	ImGui::Begin("Editor Settings");
	ImGui::Checkbox("Show Grid", &m_Settings.ShowGrid);
	ImGui::End();

	// ImGuizmo
	GameObject* selectedObject = nullptr;

	if (activeScene)
	{
		selectedObject = activeScene->GetGameObject(m_WindowHierarchy->GetSelectedUID());
	}

	m_WindowViewport.Draw(renderTexture, m_EditorGizmo, camera,	selectedObject);
}

EditorUI::EditorRequest EditorUI::ConsumePendingRequest()
{
	EditorRequest request = std::move(m_PendingRequest);
	m_PendingRequest = {};

	return request;
}

void EditorUI::ClearSelection()
{
	if (m_WindowHierarchy)
	{
		m_WindowHierarchy->ClearSelection();
	}
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

void EditorUI::DrawFileMenu(
    Scene* activeScene,
    const FileSystem& fileSystem,
    const std::filesystem::path& currentScenePath)
{
    bool openSaveAs = false;
    bool openLoad = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene", nullptr, false, activeScene != nullptr))
            {
                if (currentScenePath.empty())
                {
                    openSaveAs = true;
                }
                else
                {
                    m_PendingRequest = {
                        EditorAction::SaveScene,
                        currentScenePath
                    };
                }
            }

            if (ImGui::MenuItem("Save Scene As...", nullptr, false, activeScene != nullptr))
            {
                openSaveAs = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Load Scene..."))
            {
                openLoad = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Open modals after closing the menu
    if (openSaveAs)
    {
        std::snprintf(
            m_SaveSceneName,
            sizeof(m_SaveSceneName),
            "%s",
            activeScene->GetSceneName().c_str()
        );

        ImGui::OpenPopup("Save Scene As");
    }

    if (openLoad)
    {
        m_SelectedLoadPath.clear();
        ImGui::OpenPopup("Load Scene");
    }

    // SAVE AS WINDOW
    if (ImGui::BeginPopupModal(
        "Save Scene As",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText(
            "Scene Name",
            m_SaveSceneName,
            sizeof(m_SaveSceneName)
        );

        ImGui::TextDisabled("Saved in: Scenes/");

        std::string name = m_SaveSceneName;

        bool validName =
            !name.empty() &&
            name != "." &&
            name != ".." &&
            name.find_first_of("<>:\"/\\|?*") == std::string::npos &&
            name.back() != '.' &&
            name.back() != ' ';

        if (validName)
        {
            auto savePath =
                std::filesystem::path("Scenes") / (name + ".json");

            if (fileSystem.Exists(savePath))
            {
                ImGui::TextDisabled("Warning: Existing scene will be overwritten.");
            }
        }

        ImGui::BeginDisabled(!validName);

        if (ImGui::Button("Save"))
        {
            std::filesystem::path savePath =
                std::filesystem::path("Scenes") / (name + ".json");

            m_PendingRequest = {
                EditorAction::SaveScene,
                savePath
            };

            activeScene->SetName(name);

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // LOAD WINDOW
    if (ImGui::BeginPopupModal(
        "Load Scene",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Available Scenes:");

        ImGui::BeginChild(
            "SceneList",
            ImVec2(400, 220),
            true
        );

        std::vector<FileSystem::FileEntry> entries;

        if (fileSystem.ListDirectory("Scenes", entries))
        {
            bool foundScene = false;

            for (const auto& entry : entries)
            {
                if (entry.IsDirectory)
                    continue;

                if (entry.Path.extension() != ".json")
                    continue;

                foundScene = true;

                std::string filename =
                    entry.Path.filename().string();

                bool selected =
                    m_SelectedLoadPath == entry.Path;

                if (ImGui::Selectable(
                    filename.c_str(),
                    selected))
                {
                    m_SelectedLoadPath = entry.Path;
                }
            }

            if (!foundScene)
            {
                ImGui::TextDisabled("No scenes found.");
            }
        }
        else
        {
            ImGui::TextDisabled("Could not read Scenes directory.");
        }

        ImGui::EndChild();

        ImGui::BeginDisabled(m_SelectedLoadPath.empty());

        if (ImGui::Button("Load"))
        {
            m_PendingRequest = {
                EditorAction::LoadScene,
                m_SelectedLoadPath
            };

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
