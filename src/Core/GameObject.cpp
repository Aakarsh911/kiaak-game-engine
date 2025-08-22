#include "Core/GameObject.hpp"
#include <algorithm>

namespace Kiaak
{
    namespace Core
    {

        uint32_t GameObject::s_nextID = 1;

        GameObject::GameObject(const std::string &name)
            : m_name(name), m_id(s_nextID++)
        {

            // Every GameObject must have a Transform component
            m_transform = AddComponent<Transform>();
        }

        GameObject::~GameObject()
        {
            OnDestroy();
        }

        void GameObject::Start()
        {
            if (!m_active)
                return;

            m_started = true;
            for (auto &component : m_components)
            {
                if (component->IsEnabled())
                {
                    component->Start();
                }
            }
        }

        void GameObject::Update(double deltaTime)
        {
            if (!m_active)
                return;

            for (auto &component : m_components)
            {
                if (component->IsEnabled())
                {
                    component->Update(deltaTime);
                }
            }
        }

        void GameObject::FixedUpdate(double fixedDeltaTime)
        {
            if (!m_active)
                return;

            for (auto &component : m_components)
            {
                if (component->IsEnabled())
                {
                    component->FixedUpdate(fixedDeltaTime);
                }
            }
        }

        void GameObject::OnDestroy()
        {
            for (auto &component : m_components)
            {
                component->OnDestroy();
            }
        }

        void GameObject::SetParent(GameObject *newParent)
        {
            if (m_parent == newParent)
                return;
            if (m_parent)
            {
                RemoveChild(this); // remove from old parent list
            }
            m_parent = newParent;
            if (m_parent)
            {
                m_parent->AddChild(this);
            }
        }

        void GameObject::AddChild(GameObject *child)
        {
            if (!child)
                return;
            if (std::find(m_children.begin(), m_children.end(), child) == m_children.end())
            {
                m_children.push_back(child);
                if (child->GetParent() != this)
                {
                    child->m_parent = this; // direct set to avoid recursion
                }
            }
        }

        void GameObject::RemoveChild(GameObject *child)
        {
            if (!child)
                return;
            m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
            if (child->GetParent() == this)
            {
                child->m_parent = nullptr;
            }
        }

        void GameObject::AddComponentInternal(std::unique_ptr<Component> component)
        {
            Component *componentPtr = component.get();
            componentPtr->m_gameObject = this;

            // Add to type map for fast lookup
            m_componentMap[std::type_index(typeid(*componentPtr))] = componentPtr;

            // Store the component
            m_components.push_back(std::move(component));

            // Special handling for Transform
            if (auto *transform = dynamic_cast<Transform *>(componentPtr))
            {
                if (m_transform == nullptr)
                {
                    m_transform = transform;
                }
            }
        }

        Component *GameObject::GetComponent(const std::string &typeName)
        {
            for (auto &component : m_components)
            {
                if (component->GetTypeName() == typeName)
                {
                    return component.get();
                }
            }
            return nullptr;
        }

        bool GameObject::RemoveComponent(const std::string &typeName)
        {
            auto it = std::find_if(m_components.begin(), m_components.end(),
                                   [&typeName](const std::unique_ptr<Component> &component)
                                   {
                                       return component->GetTypeName() == typeName;
                                   });

            if (it != m_components.end())
            {
                Component *component = it->get();

                // Don't allow removing Transform
                if (component == m_transform)
                {
                    return false;
                }

                // Remove from type map
                m_componentMap.erase(std::type_index(typeid(*component)));

                // Remove from vector
                m_components.erase(it);

                return true;
            }
            return false;
        }

        std::vector<Component *> GameObject::GetAllComponents()
        {
            std::vector<Component *> result;
            result.reserve(m_components.size());

            for (auto &component : m_components)
            {
                result.push_back(component.get());
            }

            return result;
        }

        void GameObject::RemoveAllComponents()
        {
            // Keep only the Transform component
            auto it = std::find_if(m_components.begin(), m_components.end(),
                                   [this](const std::unique_ptr<Component> &component)
                                   {
                                       return component.get() == m_transform;
                                   });

            if (it != m_components.end())
            {
                auto transform = std::move(*it);
                m_components.clear();
                m_componentMap.clear();

                // Re-add the Transform
                m_componentMap[std::type_index(typeid(Transform))] = transform.get();
                m_components.push_back(std::move(transform));
            }
        }

    } // namespace Core
} // namespace Kiaak
