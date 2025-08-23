#include "Core/Collider2D.hpp"
#include "Core/GameObject.hpp"
#include "Core/Scene.hpp"
#include "Core/Physics2D.hpp"
#include "Core/Transform.hpp"
#include "Graphics/SpriteRenderer.hpp"

namespace Kiaak
{
    namespace Core
    {

        void Collider2D::Start()
        {
            if (m_registered)
                return;
            if (auto *go = GetGameObject())
            {
                if (auto *sc = go->GetScene())
                {
                    if (auto *phys = sc->GetPhysics2D())
                    {
                        phys->RegisterCollider(this);
                        m_registered = true;
                    }
                }
            }
        }
        void Collider2D::OnDestroy()
        {
            if (!m_registered)
                return;
            if (auto *go = GetGameObject())
            {
                if (auto *sc = go->GetScene())
                {
                    if (auto *phys = sc->GetPhysics2D())
                    {
                        phys->UnregisterCollider(this);
                    }
                }
            }
            m_registered = false;
        }
        glm::vec2 Collider2D::GetWorldCenter() const
        {
            if (auto *go = GetGameObject())
            {
                if (auto *t = go->GetTransform())
                {
                    auto p = t->GetPosition();
                    return glm::vec2(p.x, p.y) + m_offset;
                }
            }
            return m_offset;
        }
        void Collider2D::GetAABB(glm::vec2 &outMin, glm::vec2 &outMax) const
        {
            glm::vec2 c = GetWorldCenter();
            glm::vec2 h = GetSize() * 0.5f;
            outMin = c - h;
            outMax = c + h;
        }
        void BoxCollider2D::Start()
        {
            if (m_size == glm::vec2(0.0f))
            {
                if (auto *go = GetGameObject())
                {
                    if (auto *spr = go->GetComponent<Graphics::SpriteRenderer>())
                        m_size = spr->GetSize();
                }
                if (m_size == glm::vec2(0.0f))
                    m_size = glm::vec2(1.0f);
            }
            Collider2D::Start();
        }

        // Dispatch helpers: call matching callbacks on all components of owning GameObject
        static void _ForEachComponent(GameObject *go, const std::function<void(Component *)> &fn)
        {
            if (!go)
                return;
            auto comps = go->GetAllComponents();
            for (auto *c : comps)
                if (c && c->IsEnabled())
                    fn(c);
        }
        void Collider2D::_DispatchCollisionEnter(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnCollisionEnter(other); });
        }
        void Collider2D::_DispatchCollisionStay(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnCollisionStay(other); });
        }
        void Collider2D::_DispatchCollisionExit(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnCollisionExit(other); });
        }
        void Collider2D::_DispatchTriggerEnter(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnTriggerEnter(other); });
        }
        void Collider2D::_DispatchTriggerStay(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnTriggerStay(other); });
        }
        void Collider2D::_DispatchTriggerExit(Collider2D *other)
        {
            if (auto *go = GetGameObject())
                _ForEachComponent(go, [other](Component *c)
                                  { c->OnTriggerExit(other); });
        }

    }
} // namespace
