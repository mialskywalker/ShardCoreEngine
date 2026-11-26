#include "WindowHierarchy.h"
#include "imgui.h"
#include "Scene.h"
#include "GameObject.h"

void WindowHierarchy::Draw(Scene* scene)
{
	ImGui::Begin("Hierarchy");

    if (scene && m_SceneName != scene->GetSceneName())
    {
        m_SceneName = scene->GetSceneName();
    }

    if (scene != m_LastScene)
    {
        m_SelectedUID = 0;
        m_LastScene = scene;
    }

    if (scene && m_SelectedUID != 0)
    {
        if (!scene->GetGameObject(m_SelectedUID))
        {
            m_SelectedUID = 0;
        }
    }

	if (scene)
	{
        ImGui::Text(m_SceneName.c_str());

		for (auto& go : scene->GetGameObjects())
		{
			if (go->GetParent() == nullptr)
			{
				DrawGameObject(go.get());
			}
		}

        // Process child creation
        if (m_PendingCreateChildUID != 0)
        {
            GameObject* parent = scene->GetGameObject(m_PendingCreateChildUID);
            if (parent)
            {
                GameObject* child = scene->CreateGameObject("GameObject");
                if (child)
                {
                    child->SetParent(parent);
                }
            }

            m_PendingCreateChildUID = 0;
        }

        // Process deletion
        if (m_PendingDeleteUID != 0)
        {
            scene->RemoveGameObject(m_PendingDeleteUID);
            m_PendingDeleteUID = 0;
        }
	}

    // Context menu
    if (ImGui::BeginPopupContextWindow(
        "HierarchyContext",
        ImGuiPopupFlags_NoOpenOverItems |
        ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Empty GameObject"))
        {
            scene->CreateGameObject("GameObject");
        }

        ImGui::EndPopup();
    }

	ImGui::End();
}

void WindowHierarchy::ClearSelection()
{
    m_SelectedUID = 0;
}

void WindowHierarchy::DrawGameObject(GameObject* go)
{
    if (!go)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    // Objects without children are leaf nodes
    if (go->GetChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    // Selected
    if (m_SelectedUID == go->GetUID())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = ImGui::TreeNodeEx(static_cast<void*>(go), flags, "%s", go->GetName().c_str());

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Create Child"))
        {
            m_PendingCreateChildUID = go->GetUID();
        }

        if (ImGui::MenuItem("Delete"))
        {
            m_PendingDeleteUID = go->GetUID();
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        m_SelectedUID = go->GetUID();
    }

    if (opened)
    {
        for (GameObject* child : go->GetChildren())
        {
            DrawGameObject(child);
        }

        ImGui::TreePop();
    }
}