#pragma once

#include "Component.hpp"
#include "Transform.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

namespace Kiaak
{
    namespace Core
    {

        /**
         * GameObject is the fundamental object in the scene
         * It acts as a container for components and provides a transform
         */
        class GameObject
        {
        public:
            GameObject(const std::string &name = "GameObject");
            ~GameObject();

            // GameObject properties
            void SetName(const std::string &name) { m_name = name; }
            const std::string &GetName() const { return m_name; }

            // Simple hierarchy (editor organization only; does NOT affect transform math)
            void SetParent(GameObject *newParent);
            GameObject *GetParent() const { return m_parent; }
            const std::vector<GameObject *> &GetChildren() const { return m_children; }
            void AddChild(GameObject *child);
            void RemoveChild(GameObject *child);

            void SetActive(bool active) { m_active = active; }
            bool IsActive() const { return m_active; }

            // Transform access (every GameObject has a Transform)
            Transform *GetTransform() { return m_transform; }
            const Transform *GetTransform() const { return m_transform; }

            // Component management
            template <typename T, typename... Args>
            T *AddComponent(Args &&...args);

            template <typename T>
            T *GetComponent();

            template <typename T>
            const T *GetComponent() const;

            template <typename T>
            std::vector<T *> GetComponents();

            template <typename T>
            bool HasComponent() const;

            template <typename T>
            bool RemoveComponent();

            void RemoveAllComponents();

            // Component access by type name (for editor/serialization)
            Component *GetComponent(const std::string &typeName);
            bool RemoveComponent(const std::string &typeName);
            std::vector<Component *> GetAllComponents();

            // Lifecycle methods
            void Start();
            void Update(double deltaTime);
            void FixedUpdate(double fixedDeltaTime);
            void OnDestroy();

            // Unique ID for this GameObject
            uint32_t GetID() const { return m_id; }

        private:
            std::string m_name;
            bool m_active = true;
            uint32_t m_id;

            // Hierarchy
            GameObject *m_parent = nullptr;
            std::vector<GameObject *> m_children; // raw pointers; ownership stays in Scene

            // Transform is always present
            Transform *m_transform = nullptr;

            // Component storage
            std::vector<std::unique_ptr<Component>> m_components;
            std::unordered_map<std::type_index, Component *> m_componentMap;

            // Static ID counter
            static uint32_t s_nextID;

            // Helper methods
            void AddComponentInternal(std::unique_ptr<Component> component);
        };

        // Template implementations
        template <typename T, typename... Args>
        T *GameObject::AddComponent(Args &&...args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must be derived from Component");

            // Don't allow duplicate Transform components
            if constexpr (std::is_same_v<T, Transform>)
            {
                if (m_transform != nullptr)
                {
                    return m_transform;
                }
            }

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T *componentPtr = component.get();

            AddComponentInternal(std::move(component));

            return componentPtr;
        }

        template <typename T>
        T *GameObject::GetComponent()
        {
            auto it = m_componentMap.find(std::type_index(typeid(T)));
            if (it != m_componentMap.end())
            {
                return static_cast<T *>(it->second);
            }
            return nullptr;
        }

        template <typename T>
        const T *GameObject::GetComponent() const
        {
            auto it = m_componentMap.find(std::type_index(typeid(T)));
            if (it != m_componentMap.end())
            {
                return static_cast<const T *>(it->second);
            }
            return nullptr;
        }

        template <typename T>
        std::vector<T *> GameObject::GetComponents()
        {
            std::vector<T *> result;
            for (auto &component : m_components)
            {
                if (T *casted = dynamic_cast<T *>(component.get()))
                {
                    result.push_back(casted);
                }
            }
            return result;
        }

        template <typename T>
        bool GameObject::HasComponent() const
        {
            return m_componentMap.find(std::type_index(typeid(T))) != m_componentMap.end();
        }

        template <typename T>
        bool GameObject::RemoveComponent()
        {
            auto it = m_componentMap.find(std::type_index(typeid(T)));
            if (it != m_componentMap.end())
            {
                Component *component = it->second;

                // Don't allow removing Transform
                if (component == m_transform)
                {
                    return false;
                }

                // Remove from map
                m_componentMap.erase(it);

                // Remove from vector
                m_components.erase(
                    std::remove_if(m_components.begin(), m_components.end(),
                                   [component](const std::unique_ptr<Component> &ptr)
                                   {
                                       return ptr.get() == component;
                                   }),
                    m_components.end());

                return true;
            }
            return false;
        }

    } // namespace Core
} // namespace Kiaak
