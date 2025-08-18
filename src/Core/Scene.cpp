#include "Core/Scene.hpp"
#include <algorithm>
#include <iostream>

namespace Kiaak {
namespace Core {

Scene::Scene() {
    std::cout << "Scene created" << std::endl;
}

Scene::~Scene() {
    ClearAllSprites();
    std::cout << "Scene destroyed" << std::endl;
}

Kiaak::Sprite* Scene::CreateSprite(const std::string& id, const std::string& texturePath) {
    // Check if sprite with this ID already exists
    if (sprites.find(id) != sprites.end()) {
        std::cout << "Warning: Sprite with ID '" << id << "' already exists. Replacing it." << std::endl;
        RemoveSprite(id);
    }

    // Create new sprite
    auto sprite = std::make_unique<Kiaak::Sprite>(texturePath);
    
    // Store raw pointer for return
    Kiaak::Sprite* spritePtr = sprite.get();
    
    // Add to containers
    sprites[id] = std::move(sprite);
    spriteLayers[id] = 0; // Default layer
    renderOrderDirty = true;
    
    std::cout << "Created sprite '" << id << "' with texture: " << texturePath << std::endl;
    return spritePtr;
}

Kiaak::Sprite* Scene::GetSprite(const std::string& id) {
    auto it = sprites.find(id);
    if (it != sprites.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool Scene::RemoveSprite(const std::string& id) {
    auto spriteIt = sprites.find(id);
    auto layerIt = spriteLayers.find(id);
    
    if (spriteIt != sprites.end()) {
        sprites.erase(spriteIt);
        if (layerIt != spriteLayers.end()) {
            spriteLayers.erase(layerIt);
        }
        renderOrderDirty = true;
        std::cout << "Removed sprite '" << id << "'" << std::endl;
        return true;
    }
    
    std::cout << "Warning: Sprite '" << id << "' not found for removal" << std::endl;
    return false;
}

void Scene::ClearAllSprites() {
    size_t count = sprites.size();
    sprites.clear();
    spriteLayers.clear();
    renderOrder.clear();
    renderOrderDirty = true;
    
    if (count > 0) {
        std::cout << "Cleared " << count << " sprites from scene" << std::endl;
    }
}

void Scene::RenderAll() {
    if (sprites.empty()) {
        return;
    }
    
    // Update render order if needed
    if (renderOrderDirty) {
        UpdateRenderOrder();
    }
    
    // Render sprites in layer order
    for (const std::string& spriteId : renderOrder) {
        auto it = sprites.find(spriteId);
        if (it != sprites.end()) {
            it->second->Draw();
        }
    }
}

void Scene::SetSpriteLayer(const std::string& id, int layer) {
    if (sprites.find(id) != sprites.end()) {
        spriteLayers[id] = layer;
        renderOrderDirty = true;
        std::cout << "Set sprite '" << id << "' to layer " << layer << std::endl;
    } else {
        std::cout << "Warning: Cannot set layer for non-existent sprite '" << id << "'" << std::endl;
    }
}

int Scene::GetSpriteLayer(const std::string& id) const {
    auto it = spriteLayers.find(id);
    return (it != spriteLayers.end()) ? it->second : 0;
}

size_t Scene::GetSpriteCount() const {
    return sprites.size();
}

std::vector<std::string> Scene::GetSpriteIds() const {
    std::vector<std::string> ids;
    ids.reserve(sprites.size());
    
    for (const auto& pair : sprites) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

bool Scene::HasSprite(const std::string& id) const {
    return sprites.find(id) != sprites.end();
}

void Scene::UpdateRenderOrder() const {
    renderOrder.clear();
    renderOrder.reserve(sprites.size());
    
    // Collect all sprite IDs with their layers
    std::vector<std::pair<std::string, int>> spriteLayerPairs;
    for (const auto& spritePair : sprites) {
        const std::string& id = spritePair.first;
        int layer = GetSpriteLayer(id);
        spriteLayerPairs.emplace_back(id, layer);
    }
    
    // Sort by layer (lower layers render first)
    std::sort(spriteLayerPairs.begin(), spriteLayerPairs.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    
    // Extract sorted IDs
    for (const auto& pair : spriteLayerPairs) {
        renderOrder.push_back(pair.first);
    }
    
    renderOrderDirty = false;
}

std::string Scene::GenerateUniqueId(const std::string& baseName) const {
    std::string id = baseName;
    int counter = 1;
    
    while (HasSprite(id)) {
        id = baseName + "_" + std::to_string(counter);
        counter++;
    }
    
    return id;
}

} // namespace Core
} // namespace Kiaak
