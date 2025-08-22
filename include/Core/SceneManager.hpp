#pragma once

#include "Scene.hpp"
#include <memory>
#include <unordered_map>
#include <string>

namespace Kiaak
{
    namespace Core
    {

        class SceneManager
        {
        public:
            SceneManager();
            ~SceneManager();

            // Scene management
            bool CreateScene(const std::string &sceneName);
            bool LoadScene(const std::string &sceneName);
            bool UnloadScene(const std::string &sceneName);
            bool SwitchToScene(const std::string &sceneName);

            // Scene access
            Scene *GetCurrentScene();
            Scene *GetScene(const std::string &sceneName);
            bool HasScene(const std::string &sceneName) const;
            // Delete (unload & remove) a scene; if deleting current switches to another available scene first.
            bool DeleteScene(const std::string &sceneName);

            // Scene lifecycle
            void Update(double deltaTime);
            void Render();

            // Scene persistence (for future implementation)
            bool SaveScene(const std::string &sceneName, const std::string &filePath);
            bool LoadSceneFromFile(const std::string &sceneName, const std::string &filePath);

            // Utility
            std::vector<std::string> GetSceneNames() const;
            size_t GetSceneCount() const;

        private:
            std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
            std::string m_currentSceneName;
            Scene *m_currentScene;

            void SetCurrentScene(const std::string &sceneName);
        };

    } // namespace Core
} // namespace Kiaak
