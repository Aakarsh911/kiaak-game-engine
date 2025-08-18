#pragma once

#include "GameObject.hpp"
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

    // GameObject management (new component-based system)
    GameObject* CreateGameObject(const std::string& name = "GameObject");
    GameObject* GetGameObject(const std::string& name);
    GameObject* GetGameObject(uint32_t id);
    bool RemoveGameObject(const std::string& name);
    bool RemoveGameObject(uint32_t id);
    bool RemoveGameObject(GameObject* gameObject);
    void ClearAllGameObjects();
    
    // GameObject queries
    std::vector<GameObject*> GetAllGameObjects();
    std::vector<GameObject*> GetGameObjectsWithName(const std::string& name);
    size_t GetGameObjectCount() const;

    // Scene lifecycle
    void Start();
    void Update(double deltaTime);
    void FixedUpdate(double fixedDeltaTime);
    void Render();

private:
    // GameObject storage
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::unordered_map<std::string, GameObject*> m_gameObjectsByName;
    std::unordered_map<uint32_t, GameObject*> m_gameObjectsById;
    
    // Scene state
    bool m_started = false;
    
    // Helper methods
    std::string GenerateUniqueGameObjectName(const std::string& baseName) const;
};

} // namespace Core
} // namespace Kiaak
