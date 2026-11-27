#pragma once

#include "AssetTypes.h"
#include "EditorIcons.h"
#include "imgui.h"

struct AssetVisual
{
    const char* Icon;
    const char* Label;
    ImU32 Color;
};

namespace AssetVisuals
{
    inline AssetVisual GetFolder()
    {
        return {
            EditorIcons::Folder,
            "Folder",
            IM_COL32(213, 174, 104, 255)
        };
    }

    inline AssetVisual Get(AssetType type)
    {
        switch (type)
        {
        case AssetType::Texture:
            return {
                EditorIcons::Texture,
                "Texture",
                IM_COL32(118, 184, 154, 255)
            };

        case AssetType::Sprite:
            return {
                EditorIcons::Sprite,
                "Sprite",
                IM_COL32(96, 196, 196, 255)
            };

        case AssetType::Mesh:
            return {
                EditorIcons::Mesh,
                "Mesh",
                IM_COL32(121, 168, 219, 255)
            };

        case AssetType::Model:
            return {
                EditorIcons::Model,
                "Model",
                IM_COL32(121, 168, 219, 255)
            };

        case AssetType::AnimationClip:
            return {
                EditorIcons::AnimationClip,
                "Animation Clip",
                IM_COL32(173, 145, 214, 255)
            };

        case AssetType::AnimationGraph:
            return {
                EditorIcons::AnimationGraph,
                "Animation Graph",
                IM_COL32(219, 120, 141, 255)
            };

        case AssetType::Audio:
            return {
                EditorIcons::Audio,
                "Audio",
                IM_COL32(121, 168, 219, 255)
            };

        case AssetType::Material:
            return {
                EditorIcons::Material,
                "Material",
                IM_COL32(217, 155, 101, 255)
            };

        case AssetType::Scene:
            return {
                EditorIcons::Scene,
                "Scene",
                IM_COL32(213, 213, 223, 255)
            };

        case AssetType::Prefab:
            return {
                EditorIcons::Prefab,
                "Prefab",
                IM_COL32(114, 153, 228, 255)
            };

        default:
            return {
                EditorIcons::File,
                "Unknown",
                IM_COL32(140, 140, 150, 255)
            };
        }
    }
}