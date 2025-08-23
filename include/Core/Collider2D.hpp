#pragma once

#include "Component.hpp"
#include <glm/glm.hpp>

namespace Kiaak
{
    namespace Core
    {

        class Rigidbody2D;
        class Physics2D;
        class GameObject;
        class Transform;

        // Base 2D collider (axis-aligned). Derived shapes supply size.
        class Collider2D : public Component
        {
        public:
            Collider2D() = default;
            ~Collider2D() override = default;
            std::string GetTypeName() const override { return "Collider2D"; }
            void Start() override;     // register with physics
            void OnDestroy() override; // unregister

            void SetTrigger(bool t) { m_isTrigger = t; }
            bool IsTrigger() const { return m_isTrigger; }

            void SetOffset(const glm::vec2 &o) { m_offset = o; }
            glm::vec2 GetOffset() const { return m_offset; }

            glm::vec2 GetWorldCenter() const;                         // transform position + offset
            void GetAABB(glm::vec2 &outMin, glm::vec2 &outMax) const; // world-space AABB

            virtual glm::vec2 GetSize() const = 0; // width,height
            // Internal dispatch used by physics (routes to owner components)
            void _DispatchCollisionEnter(Collider2D *other);
            void _DispatchCollisionStay(Collider2D *other);
            void _DispatchCollisionExit(Collider2D *other);
            void _DispatchTriggerEnter(Collider2D *other);
            void _DispatchTriggerStay(Collider2D *other);
            void _DispatchTriggerExit(Collider2D *other);

        protected:
            bool m_isTrigger{false};
            glm::vec2 m_offset{0.0f};
            bool m_registered{false};
        };

        class BoxCollider2D : public Collider2D
        {
        public:
            BoxCollider2D() = default;
            ~BoxCollider2D() override = default;
            std::string GetTypeName() const override { return "BoxCollider2D"; }
            void Start() override; // auto-size from sprite if size==0
            void SetSize(const glm::vec2 &s) { m_size = s; }
            glm::vec2 GetSize() const override { return m_size; }

        private:
            glm::vec2 m_size{0.0f};
        };

    }
} // namespace Kiaak::Core
