#pragma once

#include "AssetTypes.h"

struct AssetReference
{
    AssetID ID = InvalidAssetID;

    bool IsValid() const
    {
        return ID != InvalidAssetID;
    }

    void Reset()
    {
        ID = InvalidAssetID;
    }

    bool operator==(const AssetReference& other) const
    {
        return ID == other.ID;
    }

    bool operator!=(const AssetReference& other) const
    {
        return ID != other.ID;
    }
};