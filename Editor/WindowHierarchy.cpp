#include "WindowHierarchy.h"
#include "EditorSelection.h"
#include "imgui.h"
#include "Scene.h"
#include "GameObject.h"

void WindowHierarchy::Draw(Scene* scene)
{
	if (!ImGui::Begin("Hierarchy"))
    {
        ImGui::End();
        return;
    }

    if (scene && m_SceneName != scene->GetSceneName())
    {
        m_SceneName = scene->GetSceneName();
    }


	if (scene)
	{
        ImGui::TextUnformatted(m_SceneName.c_str());

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
        if (ImGui::MenuItem(
            "Create Empty GameObject",
            nullptr,
            false,
            scene != nullptr))
        {
            scene->CreateGameObject("GameObject");
        }

        ImGui::EndPopup();
    }

	ImGui::End();
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
    if (m_Selection.GetGameObjectUID() == go->GetUID())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = ImGui::TreeNodeEx(static_cast<void*>(go), flags, "%s", go->GetName().c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsItemToggledOpen())
    {
        m_Selection.SelectGameObject(go->GetUID());
    }

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

    if (opened)
    {
        for (GameObject* child : go->GetChildren())
        {
            DrawGameObject(child);
        }

        ImGui::TreePop();
    }
}