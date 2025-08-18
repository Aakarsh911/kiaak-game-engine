#include "Core/SceneManager.hpp"
#include <iostream>

namespace Kiaak {
namespace Core {

SceneManager::SceneManager() : m_currentScene(nullptr) {
    std::cout << "SceneManager created" << std::endl;
}

SceneManager::~SceneManager() {
    // Clean up all scenes
    m_scenes.clear();
    std::cout << "SceneManager destroyed" << std::endl;
}

bool SceneManager::CreateScene(const std::string& sceneName) {
    if (HasScene(sceneName)) {
        std::cerr << "Scene '" << sceneName << "' already exists!" << std::endl;
        return false;
    }
    
    auto scene = std::make_unique<Scene>();
    m_scenes[sceneName] = std::move(scene);
    
    std::cout << "Created scene: " << sceneName << std::endl;
    
    // If this is the first scene, make it current
    if (m_currentScene == nullptr) {
        SetCurrentScene(sceneName);
    }
    
    return true;
}

bool SceneManager::LoadScene(const std::string& sceneName) {
    // For now, just create an empty scene
    // Later this would load from a file
    return CreateScene(sceneName);
}

bool SceneManager::UnloadScene(const std::string& sceneName) {
    if (!HasScene(sceneName)) {
        std::cerr << "Scene '" << sceneName << "' does not exist!" << std::endl;
        return false;
    }
    
    // Don't unload the current scene
    if (sceneName == m_currentSceneName) {
        std::cerr << "Cannot unload current scene '" << sceneName << "'!" << std::endl;
        return false;
    }
    
    m_scenes.erase(sceneName);
    std::cout << "Unloaded scene: " << sceneName << std::endl;
    return true;
}

bool SceneManager::SwitchToScene(const std::string& sceneName) {
    if (!HasScene(sceneName)) {
        std::cerr << "Scene '" << sceneName << "' does not exist!" << std::endl;
        return false;
    }
    
    std::cout << "Switching from scene '" << m_currentSceneName << "' to '" << sceneName << "'" << std::endl;
    SetCurrentScene(sceneName);
    return true;
}

Scene* SceneManager::GetCurrentScene() {
    return m_currentScene;
}

Scene* SceneManager::GetScene(const std::string& sceneName) {
    auto it = m_scenes.find(sceneName);
    if (it != m_scenes.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool SceneManager::HasScene(const std::string& sceneName) const {
    return m_scenes.find(sceneName) != m_scenes.end();
}

void SceneManager::Update(double deltaTime) {
    if (m_currentScene) {
        // For now, scenes don't have update logic
        // This is where scene-specific update code would go
    }
}

void SceneManager::Render() {
    if (m_currentScene) {
        m_currentScene->Render();
    }
}

bool SceneManager::SaveScene(const std::string& sceneName, const std::string& filePath) {
    // TODO: Implement scene serialization
    std::cout << "Scene saving not yet implemented" << std::endl;
    return false;
}

bool SceneManager::LoadSceneFromFile(const std::string& sceneName, const std::string& filePath) {
    // TODO: Implement scene deserialization
    std::cout << "Scene loading from file not yet implemented" << std::endl;
    return false;
}

std::vector<std::string> SceneManager::GetSceneNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_scenes) {
        names.push_back(pair.first);
    }
    return names;
}

size_t SceneManager::GetSceneCount() const {
    return m_scenes.size();
}

void SceneManager::SetCurrentScene(const std::string& sceneName) {
    auto it = m_scenes.find(sceneName);
    if (it != m_scenes.end()) {
        m_currentSceneName = sceneName;
        m_currentScene = it->second.get();
        std::cout << "Current scene set to: " << sceneName << std::endl;
    }
}

} // namespace Core
} // namespace Kiaak
