#include "Core/Scene.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include <algorithm>
#include <iostream>

namespace Kiaak {
namespace Core {

Scene::Scene() {
    std::cout << "Scene created" << std::endl;
}

Scene::~Scene() {
    ClearAllGameObjects();
    std::cout << "Scene destroyed" << std::endl;
}

// GameObject management (new component-based system)
GameObject* Scene::CreateGameObject(const std::string& name) {
    std::string uniqueName = GenerateUniqueGameObjectName(name);
    
    auto gameObject = std::make_unique<GameObject>(uniqueName);
    GameObject* gameObjectPtr = gameObject.get();
    
    // Add to storage
    uint32_t id = gameObjectPtr->GetID();
    m_gameObjectsById[id] = gameObjectPtr;
    m_gameObjectsByName[uniqueName] = gameObjectPtr;
    m_gameObjects.push_back(std::move(gameObject));
    
    // If scene is already started, start this GameObject
    if (m_started) {
        gameObjectPtr->Start();
    }
    
    std::cout << "Created GameObject '" << uniqueName << "' with ID " << id << std::endl;
    return gameObjectPtr;
}

GameObject* Scene::GetGameObject(const std::string& name) {
    auto it = m_gameObjectsByName.find(name);
    return (it != m_gameObjectsByName.end()) ? it->second : nullptr;
}

GameObject* Scene::GetGameObject(uint32_t id) {
    auto it = m_gameObjectsById.find(id);
    return (it != m_gameObjectsById.end()) ? it->second : nullptr;
}

bool Scene::RemoveGameObject(const std::string& name) {
    auto it = m_gameObjectsByName.find(name);
    if (it != m_gameObjectsByName.end()) {
        return RemoveGameObject(it->second);
    }
    return false;
}

bool Scene::RemoveGameObject(uint32_t id) {
    auto it = m_gameObjectsById.find(id);
    if (it != m_gameObjectsById.end()) {
        return RemoveGameObject(it->second);
    }
    return false;
}

bool Scene::RemoveGameObject(GameObject* gameObject) {
    if (!gameObject) return false;
    
    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& ptr) {
            return ptr.get() == gameObject;
        });
    
    if (it != m_gameObjects.end()) {
        uint32_t id = gameObject->GetID();
        std::string name = gameObject->GetName();
        
        // Remove from maps
        m_gameObjectsById.erase(id);
        m_gameObjectsByName.erase(name);
        
        // Remove from vector
        m_gameObjects.erase(it);
        
        std::cout << "Removed GameObject '" << name << "' with ID " << id << std::endl;
        return true;
    }
    
    return false;
}

void Scene::ClearAllGameObjects() {
    size_t count = m_gameObjects.size();
    m_gameObjects.clear();
    m_gameObjectsByName.clear();
    m_gameObjectsById.clear();
    
    if (count > 0) {
        std::cout << "Cleared " << count << " GameObjects from scene" << std::endl;
    }
}

std::vector<GameObject*> Scene::GetAllGameObjects() {
    std::vector<GameObject*> result;
    result.reserve(m_gameObjects.size());
    
    for (auto& gameObject : m_gameObjects) {
        result.push_back(gameObject.get());
    }
    
    return result;
}

std::vector<GameObject*> Scene::GetGameObjectsWithName(const std::string& name) {
    std::vector<GameObject*> result;
    
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->GetName() == name) {
            result.push_back(gameObject.get());
        }
    }
    
    return result;
}

size_t Scene::GetGameObjectCount() const {
    return m_gameObjects.size();
}

// Scene lifecycle
void Scene::Start() {
    if (m_started) return;
    
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->Start();
        }
    }
    
    m_started = true;
    std::cout << "Scene started with " << m_gameObjects.size() << " GameObjects" << std::endl;
}

void Scene::Update(double deltaTime) {
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->Update(deltaTime);
        }
    }
}

void Scene::FixedUpdate(double fixedDeltaTime) {
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->FixedUpdate(fixedDeltaTime);
        }
    }
}

void Scene::Render() {
    // Render GameObjects with SpriteRenderer components
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            auto* spriteRenderer = gameObject->GetComponent<Graphics::SpriteRenderer>();
            if (spriteRenderer && spriteRenderer->IsEnabled()) {
                spriteRenderer->Render();
            }
        }
    }
}

std::string Scene::GenerateUniqueGameObjectName(const std::string& baseName) const {
    std::string name = baseName;
    int counter = 1;
    
    while (m_gameObjectsByName.find(name) != m_gameObjectsByName.end()) {
        name = baseName + "_" + std::to_string(counter);
        counter++;
    }
    
    return name;
}

} // namespace Core
} // namespace Kiaak
