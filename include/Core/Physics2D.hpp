#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_set>
#include <cstdint>

namespace Kiaak
{
    namespace Core
    {

        class Rigidbody2D;
        class Collider2D;
        class Transform;

        // Lightweight 2D physics world (no collisions yet)
        class Physics2D
        {
        public:
            // Contact info struct exposed for read-only access
            struct Contact
            {
                Collider2D *a{nullptr};
                Collider2D *b{nullptr};
                glm::vec2 point{0.0f};
                glm::vec2 normal{0.0f};
                float penetration{0.0f};
            };

            // Access contacts recorded during the last Step()
            const std::vector<Contact> &GetContacts() const { return m_contacts; }

            Physics2D();
            ~Physics2D() = default;

            void SetGravity(const glm::vec2 &g) { m_gravity = g; }
            glm::vec2 GetGravity() const { return m_gravity; }

            // Advance simulation; integrates velocities and writes back to attached Transforms
            void Step(double dt);

            // Registration API used by Rigidbody2D
            void RegisterBody(Rigidbody2D *rb);
            void UnregisterBody(Rigidbody2D *rb);
            void RegisterCollider(Collider2D *col);
            void UnregisterCollider(Collider2D *col);

        private:
            struct BodyRec
            {
                Rigidbody2D *rb{nullptr};
            };
            struct ColliderRec
            {
                Collider2D *col{nullptr};
            };

            struct PairKey
            {
                const Collider2D *a{nullptr};
                const Collider2D *b{nullptr};
                bool operator==(const PairKey &o) const { return a == o.a && b == o.b; }
            };
            struct PairKeyHasher
            {
                size_t operator()(const PairKey &k) const noexcept { return ((uintptr_t)k.a >> 4) ^ ((uintptr_t)k.b << 1); }
            };

            glm::vec2 m_gravity;
            std::vector<BodyRec> m_bodies;
            std::vector<ColliderRec> m_colliders;
            std::unordered_set<PairKey, PairKeyHasher> m_prevFramePairs;
            std::vector<Contact> m_contacts;

        public:
            const std::vector<ColliderRec> &GetColliders() const { return m_colliders; }
        };

    } // namespace Core
} // namespace Kiaak
