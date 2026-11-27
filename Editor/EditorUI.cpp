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

    // HEX color conversion
    const auto Color = [](unsigned int hex, float alpha = 1.0f)
        {
            return ImVec4(
                ((hex >> 16) & 0xFF) / 255.0f,
                ((hex >> 8) & 0xFF) / 255.0f,
                (hex & 0xFF) / 255.0f,
                alpha
            );
        };

    // ========================================
    // Color palette
    // ========================================

    const ImVec4 background = Color(0x101013);
    const ImVec4 panel = Color(0x19191E);
    const ImVec4 panelLight = Color(0x222228);

    const ImVec4 control = Color(0x28282F);
    const ImVec4 controlHover = Color(0x34343E);
    const ImVec4 controlActive = Color(0x40404B);

    const ImVec4 border = Color(0x303038);
    const ImVec4 borderLight = Color(0x414149);

    const ImVec4 accent = Color(0xC63146);
    const ImVec4 accentHover = Color(0xE04054);
    const ImVec4 accentActive = Color(0xA52438);
    const ImVec4 accentBright = Color(0xED485B);
    const ImVec4 accentSubtle = Color(0x552630);

    const ImVec4 text = Color(0xE5E5EB);
    const ImVec4 textDisabled = Color(0x777783);

    // ========================================
    // Layout & spacing
    // ========================================

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.CellPadding = ImVec2(8.0f, 5.0f);

    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);

    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 11.0f;
    style.GrabMinSize = 10.0f;

    style.WindowMinSize = ImVec2(120.0f, 80.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    // ========================================
    // Rounding
    // ========================================

    style.WindowRounding = 5.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    // ========================================
    // Borders
    // ========================================

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    style.TabBorderSize = 0.0f;
    style.TabBarBorderSize = 1.0f;
    style.TabBarOverlineSize = 2.0f;

    // ========================================
    // Rendering
    // ========================================

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.55f;

    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;

    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;

    ImVec4* colors = style.Colors;

    // ========================================
    // Text
    // ========================================

    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = textDisabled;
    colors[ImGuiCol_TextSelectedBg] = Color(0xC63146, 0.35f);

    // ========================================
    // Backgrounds
    // ========================================

    colors[ImGuiCol_WindowBg] = panel;
    colors[ImGuiCol_ChildBg] = background;
    colors[ImGuiCol_PopupBg] = panelLight;
    colors[ImGuiCol_MenuBarBg] = panelLight;

    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = Color(0x000000, 0.0f);

    // ========================================
    // Window titles
    // ========================================

    colors[ImGuiCol_TitleBg] = background;
    colors[ImGuiCol_TitleBgActive] = panelLight;
    colors[ImGuiCol_TitleBgCollapsed] = background;

    // ========================================
    // Input fields
    // ========================================

    colors[ImGuiCol_FrameBg] = background;
    colors[ImGuiCol_FrameBgHovered] = control;
    colors[ImGuiCol_FrameBgActive] = controlHover;

    // ========================================
    // Buttons
    // ========================================

    colors[ImGuiCol_Button] = control;
    colors[ImGuiCol_ButtonHovered] = controlHover;
    colors[ImGuiCol_ButtonActive] = controlActive;

    // ========================================
    // Checkboxes & sliders
    // ========================================

    colors[ImGuiCol_CheckMark] = accentBright;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentBright;

    // ========================================
    // Headers, hierarchy & selection
    // ========================================

    colors[ImGuiCol_Header] = accentSubtle;
    colors[ImGuiCol_HeaderHovered] = Color(0x67303B);
    colors[ImGuiCol_HeaderActive] = Color(0x823442);

    // ========================================
    // Tabs
    // ========================================

    colors[ImGuiCol_Tab] = panel;
    colors[ImGuiCol_TabHovered] = controlHover;
    colors[ImGuiCol_TabSelected] = control;

    colors[ImGuiCol_TabSelectedOverline] = accentBright;

    colors[ImGuiCol_TabDimmed] = background;
    colors[ImGuiCol_TabDimmedSelected] = panelLight;
    colors[ImGuiCol_TabDimmedSelectedOverline] = Color(0x78303B);

    // ========================================
    // Scrollbars
    // ========================================

    colors[ImGuiCol_ScrollbarBg] = background;
    colors[ImGuiCol_ScrollbarGrab] = controlHover;
    colors[ImGuiCol_ScrollbarGrabHovered] = Color(0x50505C);
    colors[ImGuiCol_ScrollbarGrabActive] = accent;

    // ========================================
    // Separators
    // ========================================

    colors[ImGuiCol_Separator] = border;
    colors[ImGuiCol_SeparatorHovered] = accentHover;
    colors[ImGuiCol_SeparatorActive] = accentActive;

    // ========================================
    // Resize grips
    // ========================================

    colors[ImGuiCol_ResizeGrip] = Color(0x414149, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = accentHover;
    colors[ImGuiCol_ResizeGripActive] = accentActive;

    // ========================================
    // Docking
    // ========================================

    colors[ImGuiCol_DockingPreview] = Color(0xC63146, 0.45f);
    colors[ImGuiCol_DockingEmptyBg] = background;

    // ========================================
    // Tables
    // ========================================

    colors[ImGuiCol_TableHeaderBg] = panelLight;
    colors[ImGuiCol_TableBorderStrong] = borderLight;
    colors[ImGuiCol_TableBorderLight] = border;

    colors[ImGuiCol_TableRowBg] = Color(0x19191E, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = Color(0xFFFFFF, 0.025f);

    // ========================================
    // Plots
    // ========================================

    colors[ImGuiCol_PlotLines] = accentBright;
    colors[ImGuiCol_PlotLinesHovered] = Color(0xFF8494);
    colors[ImGuiCol_PlotHistogram] = accent;
    colors[ImGuiCol_PlotHistogramHovered] = accentHover;

    // ========================================
    // Navigation & modal windows
    // ========================================

    colors[ImGuiCol_NavCursor] = accentBright;
    colors[ImGuiCol_NavWindowingHighlight] = Color(0xFFFFFF, 0.65f);
    colors[ImGuiCol_NavWindowingDimBg] = Color(0x000000, 0.4f);
    colors[ImGuiCol_ModalWindowDimBg] = Color(0x000000, 0.65f);

    colors[ImGuiCol_DragDropTarget] = accentBright;
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
