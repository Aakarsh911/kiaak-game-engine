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

        public:
            const std::vector<ColliderRec> &GetColliders() const { return m_colliders; }
        };

    } // namespace Core
} // namespace Kiaak
