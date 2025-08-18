#pragma once

#include "Graphics/Sprite.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace Kiaak {
namespace Core {

class Scene {
public:
    Scene();
    ~Scene();

    // Sprite management
    Kiaak::Sprite* CreateSprite(const std::string& id, const std::string& texturePath);
    Kiaak::Sprite* GetSprite(const std::string& id);
    bool RemoveSprite(const std::string& id);
    void ClearAllSprites();

    // Rendering
    void RenderAll();

    // Layer management
    void SetSpriteLayer(const std::string& id, int layer);
    int GetSpriteLayer(const std::string& id) const;

    // Utility
    size_t GetSpriteCount() const;
    std::vector<std::string> GetSpriteIds() const;
    bool HasSprite(const std::string& id) const;

private:
    // Storage for sprites
    std::unordered_map<std::string, std::unique_ptr<Kiaak::Sprite>> sprites;
    
    // Layer management for render order
    std::unordered_map<std::string, int> spriteLayers;
    
    // Cache for sorted rendering (updated when layers change)
    mutable std::vector<std::string> renderOrder;
    mutable bool renderOrderDirty = true;
    
    // Helper methods
    void UpdateRenderOrder() const;
    std::string GenerateUniqueId(const std::string& baseName) const;
};

} // namespace Core
} // namespace Kiaak
