#pragma once

#include "GameObject.hpp"
#include "Physics2D.hpp"
#include <utility>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace Kiaak
{
    namespace Core
    {

        class Camera; // forward declare

        class Scene
        {
        public:
            Scene();
            ~Scene();

            // GameObject management (new component-based system)
            GameObject *CreateGameObject(const std::string &name = "GameObject");
            GameObject *GetGameObject(const std::string &name);
            GameObject *GetGameObject(uint32_t id);
            bool RemoveGameObject(const std::string &name);
            bool RemoveGameObject(uint32_t id);
            bool RemoveGameObject(GameObject *gameObject);
            void ClearAllGameObjects();

            // GameObject queries
            std::vector<GameObject *> GetAllGameObjects();
            std::vector<GameObject *> GetGameObjectsWithName(const std::string &name);
            size_t GetGameObjectCount() const;

            // Scene lifecycle
            void Start();
            void Update(double deltaTime);
            void FixedUpdate(double fixedDeltaTime, bool runPhysics = true);
            // Render scene contents. When includeDisabledForEditor is true (editor mode),
            // components that are disabled are still drawn for authoring visibility.
            void Render(bool includeDisabledForEditor = false);

            // Scene camera designation (used when entering play mode)
            void SetDesignatedCamera(Camera *cam) { m_designatedCamera = cam; }
            Camera *GetDesignatedCamera() const { return m_designatedCamera; }
            // Helper to clear if a GO removed
            void OnGameObjectRemoved(GameObject *go);

            // Physics access (for components)
            Physics2D* GetPhysics2D() { return &m_physics2D; }

        private:
            // GameObject storage
            std::vector<std::unique_ptr<GameObject>> m_gameObjects;
            std::unordered_map<std::string, GameObject *> m_gameObjectsByName;
            std::unordered_map<uint32_t, GameObject *> m_gameObjectsById;

            // Scene state
            bool m_started = false;

            // Non-owned pointer to designated scene camera component
            Camera *m_designatedCamera{nullptr};

            // Physics world (2D)
            Physics2D m_physics2D;

            // Helper methods
            std::string GenerateUniqueGameObjectName(const std::string &baseName) const;
        };

    } // namespace Core
} // namespace Kiaak
