#pragma once

#include "Component.hpp"
#include <glm/glm.hpp>

namespace Kiaak
{
    namespace Core
    {

        class Physics2D;

        class Rigidbody2D : public Component
        {
        public:
            enum class BodyType
            {
                Static,
                Kinematic,
                Dynamic
            };

            Rigidbody2D();
            ~Rigidbody2D() override = default;

            // Component
            void Start() override;
            void FixedUpdate(double dt) override;
            void OnDestroy() override;
            std::string GetTypeName() const override { return "Rigidbody2D"; }

            // Properties
            void SetBodyType(BodyType t) { m_type = t; }
            BodyType GetBodyType() const { return m_type; }

            void SetGravityScale(float s) { m_gravityScale = s; }
            float GetGravityScale() const { return m_gravityScale; }

            void SetUseGravity(bool u) { m_useGravity = u; }
            bool GetUseGravity() const { return m_useGravity; }

            void SetLinearDamping(float d) { m_linearDamping = d; }
            float GetLinearDamping() const { return m_linearDamping; }

            void SetMass(float m) { m_mass = m; }
            float GetMass() const { return m_mass; }

            void SetVelocity(const glm::vec2 &v) { m_velocity = v; }
            glm::vec2 GetVelocity() const { return m_velocity; }

            // Grounded helper (set by physics)
            void SetGrounded(bool g) { m_grounded = g; }
            bool IsGrounded() const { return m_grounded; }

            void AddForce(const glm::vec2 &f) { m_accumForce += f; }
            void AddImpulse(const glm::vec2 &j)
            {
                if (m_type == BodyType::Dynamic && m_mass > 0.0f)
                    m_velocity += j / m_mass;
            }

            // Debug / inspector: read accumulated force pending application
            glm::vec2 GetAccumulatedForce() const { return m_accumForce; }

            // Teleport without impulses
            void Teleport(const glm::vec2 &pos, float rotationDegZ = 0.0f);

        private:
            BodyType m_type{BodyType::Dynamic};
            float m_gravityScale{1.0f};
            float m_linearDamping{0.0f};
            float m_mass{1.0f};
            glm::vec2 m_velocity{0.0f};
            glm::vec2 m_accumForce{0.0f};
            bool m_registered{false};
            bool m_useGravity{true};
            bool m_grounded{false};
        };

    } // namespace Core
} // namespace Kiaak
