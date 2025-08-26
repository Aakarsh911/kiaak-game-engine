#include "Core/Physics2D.hpp"
#include "Core/Rigidbody2D.hpp"
#include "Core/Collider2D.hpp"
#include "Core/Collider2D.hpp"
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include <algorithm>
#include <iostream>

namespace Kiaak
{
    namespace Core
    {

        Physics2D::Physics2D() : m_gravity(0.0f, -9.81f) {}

        void Physics2D::RegisterBody(Rigidbody2D *rb)
        {
            if (!rb)
                return;
            auto it = std::find_if(m_bodies.begin(), m_bodies.end(), [rb](const BodyRec &r)
                                   { return r.rb == rb; });
            if (it == m_bodies.end())
                m_bodies.push_back({rb});
        }

        void Physics2D::UnregisterBody(Rigidbody2D *rb)
        {
            if (!rb)
                return;
            m_bodies.erase(std::remove_if(m_bodies.begin(), m_bodies.end(), [rb](const BodyRec &r)
                                          { return r.rb == rb; }),
                           m_bodies.end());
        }

        void Physics2D::RegisterCollider(Collider2D *col)
        {
            if (!col)
                return;
            auto it = std::find_if(m_colliders.begin(), m_colliders.end(), [col](const ColliderRec &r)
                                   { return r.col == col; });
            if (it == m_colliders.end())
                m_colliders.push_back({col});
        }
        void Physics2D::UnregisterCollider(Collider2D *col)
        {
            if (!col)
                return;
            m_colliders.erase(std::remove_if(m_colliders.begin(), m_colliders.end(), [col](const ColliderRec &r)
                                             { return r.col == col; }),
                              m_colliders.end());
        }

        void Physics2D::Step(double dt)
        {
            const float fdt = static_cast<float>(dt);
            // Reset grounded flags each step (will be re-set during collision processing)
            for (auto &rec : m_bodies)
                if (rec.rb)
                    rec.rb->SetGrounded(false);
            for (auto it = m_bodies.begin(); it != m_bodies.end();)
            {
                Rigidbody2D *rb = it->rb;
                if (!rb)
                {
                    it = m_bodies.erase(it);
                    continue;
                }

                if (rb->GetBodyType() == Rigidbody2D::BodyType::Dynamic)
                {
                    // Integrate forces -> velocity
                    glm::vec2 vel = rb->GetVelocity();
                    glm::vec2 acc(0.0f);
                    bool applyGravity = rb->GetUseGravity();
                    // Suppress gravity if grounded and not moving upward
                    if (applyGravity && rb->IsGrounded() && vel.y <= 0.0f)
                        applyGravity = false;
                    if (applyGravity)
                    {
                        acc = m_gravity * rb->GetGravityScale();
                    }

                    // Apply accumulated external forces (AddForce) -> acceleration = F / m
                    glm::vec2 extF = rb->GetAccumulatedForce();
                    if (rb->GetMass() > 0.0f)
                        acc += extF / rb->GetMass();

                    vel += acc * fdt;
                    // simple linear damping
                    vel *= (1.0f / (1.0f + rb->GetLinearDamping() * fdt));

                    rb->SetVelocity(vel);

                    // Clear accumulated forces after applying them for this step
                    // (forces are re-applied each frame by callers if persistent)
                    rb->AddForce(glm::vec2(0.0f - extF.x, 0.0f - extF.y));

                    // Integrate velocity -> position
                    if (auto *go = rb->GetGameObject())
                    {
                        if (auto *t = go->GetTransform())
                        {
                            glm::vec3 p = t->GetPosition();
                            p.x += vel.x * fdt;
                            p.y += vel.y * fdt;
                            t->SetPosition(p);
                        }
                    }
                }
                else if (rb->GetBodyType() == Rigidbody2D::BodyType::Kinematic)
                {
                    // Kinematic: user sets velocity directly
                    glm::vec2 vel = rb->GetVelocity();
                    if (auto *go = rb->GetGameObject())
                    {
                        if (auto *t = go->GetTransform())
                        {
                            glm::vec3 p = t->GetPosition();
                            p.x += vel.x * fdt;
                            p.y += vel.y * fdt;
                            t->SetPosition(p);
                        }
                    }
                }
                else
                {
                    // Static: do nothing
                }
                ++it;
            }

            // Broad-phase naive pair test with event dispatch + simple resolution (one dynamic) + trigger handling
            std::unordered_set<PairKey, PairKeyHasher> currentPairs;
            auto makeKey = [&](Collider2D *a, Collider2D *b)
            { if(a>b) std::swap(a,b); return PairKey{a,b}; };
            if (m_colliders.size() > 1)
            {
                for (size_t i = 0; i < m_colliders.size(); ++i)
                {
                    Collider2D *A = m_colliders[i].col;
                    if (!A || !A->IsEnabled())
                        continue;
                    Rigidbody2D *rbA = nullptr;
                    if (auto *goA = A->GetGameObject())
                        rbA = goA->GetComponent<Rigidbody2D>();
                    for (size_t j = i + 1; j < m_colliders.size(); ++j)
                    {
                        Collider2D *B = m_colliders[j].col;
                        if (!B || !B->IsEnabled() || A == B)
                            continue;
                        Rigidbody2D *rbB = nullptr;
                        if (auto *goB = B->GetGameObject())
                            rbB = goB->GetComponent<Rigidbody2D>();
                        glm::vec2 aMin, aMax, bMin, bMax;
                        A->GetAABB(aMin, aMax);
                        B->GetAABB(bMin, bMax);
                        bool overlap = (aMin.x <= bMax.x && aMax.x >= bMin.x && aMin.y <= bMax.y && aMax.y >= bMin.y);
                        if (!overlap)
                            continue;
                        // Record pair for event state tracking
                        auto key = makeKey(A, B);
                        currentPairs.insert(key);
                        bool trigger = A->IsTrigger() || B->IsTrigger();
                        if (trigger)
                        {
                            bool was = m_prevFramePairs.find(key) != m_prevFramePairs.end();
                            if (!was)
                            {
                                A->_DispatchTriggerEnter(B);
                                B->_DispatchTriggerEnter(A);
                            }
                            else
                            {
                                A->_DispatchTriggerStay(B);
                                B->_DispatchTriggerStay(A);
                            }
                            // Triggers purposely have no resolution or velocity modification here.
                            continue;
                        }
                        // Solid collision events & resolution (only one dynamic for now)
                        bool was = m_prevFramePairs.find(key) != m_prevFramePairs.end();
                        if (!was)
                        {
                            A->_DispatchCollisionEnter(B);
                            B->_DispatchCollisionEnter(A);
                        }
                        else
                        {
                            A->_DispatchCollisionStay(B);
                            B->_DispatchCollisionStay(A);
                        }
                        auto typeA = rbA ? rbA->GetBodyType() : Rigidbody2D::BodyType::Static;
                        auto typeB = rbB ? rbB->GetBodyType() : Rigidbody2D::BodyType::Static;
                        bool dynA = rbA && typeA == Rigidbody2D::BodyType::Dynamic;
                        bool dynB = rbB && typeB == Rigidbody2D::BodyType::Dynamic;
                        if (!dynA && !dynB)
                            continue; // both static/kinematic -> no resolution (kinematic vs static intentionally skipped for now)

                        // Compute penetration extents using A/B (not yet choosing which moves)
                        float penLeft = bMax.x - aMin.x;  // penetration if we move A right
                        float penRight = aMax.x - bMin.x; // penetration if we move A left
                        float penX = std::min(penLeft, penRight);
                        float penDown = bMax.y - aMin.y; // penetration if we move A up
                        float penUp = aMax.y - bMin.y;   // penetration if we move A down
                        float penY = std::min(penDown, penUp);
                        if (penX <= 0.f || penY <= 0.f)
                            continue;

                        // Decide separation axis with vertical bias if one of bodies (prefer dynamic) falling
                        glm::vec2 refVel(0.0f);
                        if (dynA && rbA)
                            refVel = rbA->GetVelocity();
                        else if (dynB && rbB)
                            refVel = rbB->GetVelocity();
                        bool verticalPreferred = std::abs(refVel.y) > std::abs(refVel.x) * 0.5f;
                        glm::vec2 normal(0.0f);
                        float penetration = 0.0f;
                        bool verticalResolution = !((penX < penY) && !verticalPreferred);
                        if (!verticalResolution)
                        {
                            penetration = penX;
                            float cA = (aMin.x + aMax.x) * 0.5f;
                            float cB = (bMin.x + bMax.x) * 0.5f;
                            normal = (cA > cB) ? glm::vec2(1, 0) : glm::vec2(-1, 0); // push A away from B
                        }
                        else
                        {
                            penetration = penY;
                            float cA = (aMin.y + aMax.y) * 0.5f;
                            float cB = (bMin.y + bMax.y) * 0.5f;
                            normal = (cA > cB) ? glm::vec2(0, 1) : glm::vec2(0, -1);
                        }

                        auto applyTranslation = [&](Collider2D *col, float scale)
                        {
                            if (!col)
                                return;
                            if (auto *go = col->GetGameObject())
                                if (auto *t = go->GetTransform())
                                {
                                    auto p = t->GetPosition();
                                    p.x += normal.x * penetration * scale;
                                    p.y += normal.y * penetration * scale;
                                    t->SetPosition(p);
                                }
                        };

                        if (dynA && dynB)
                        {
                            // split correction
                            applyTranslation(A, 0.5f);
                            applyTranslation(B, -0.5f);
                            if (rbA)
                            {
                                auto v = rbA->GetVelocity();
                                float vn = glm::dot(v, normal);
                                if (vn < 0)
                                    v -= vn * normal; // remove into-normal component
                                rbA->SetVelocity(v);
                                if (verticalResolution && normal.y > 0)
                                    rbA->SetGrounded(true);
                            }
                            if (rbB)
                            {
                                auto v = rbB->GetVelocity();
                                float vn = glm::dot(v, -normal); // relative to opposite normal
                                if (vn < 0)
                                    v -= vn * (-normal);
                                rbB->SetVelocity(v);
                                if (verticalResolution && normal.y < 0)
                                    rbB->SetGrounded(true);
                            }
                        }
                        else
                        {
                            // Only one dynamic: move the dynamic one fully
                            if (dynA)
                            {
                                applyTranslation(A, 1.0f);
                                if (rbA)
                                {
                                    auto v = rbA->GetVelocity();
                                    float vn = glm::dot(v, normal);
                                    if (vn < 0)
                                    {
                                        v -= vn * normal;
                                        rbA->SetVelocity(v);
                                    }
                                    if (verticalResolution && normal.y > 0)
                                        rbA->SetGrounded(true);
                                }
                            }
                            else if (dynB)
                            {
                                applyTranslation(B, -1.0f); // normal defined to push A away from B, so move B opposite
                                if (rbB)
                                {
                                    auto v = rbB->GetVelocity();
                                    float vn = glm::dot(v, -normal);
                                    if (vn < 0)
                                    {
                                        v -= vn * (-normal);
                                        rbB->SetVelocity(v);
                                    }
                                    if (verticalResolution && normal.y < 0)
                                        rbB->SetGrounded(true);
                                }
                            }
                        }
                    }
                }
            }
            // Exit events for pairs gone this frame
            for (const auto &prev : m_prevFramePairs)
            {
                if (currentPairs.find(prev) == currentPairs.end())
                {
                    auto *A = const_cast<Collider2D *>(prev.a);
                    auto *B = const_cast<Collider2D *>(prev.b);
                    if (A && B)
                    {
                        if (A->IsTrigger() || B->IsTrigger())
                        {
                            A->_DispatchTriggerExit(B);
                            B->_DispatchTriggerExit(A);
                        }
                        else
                        {
                            A->_DispatchCollisionExit(B);
                            B->_DispatchCollisionExit(A);
                        }
                    }
                }
            }
            m_prevFramePairs.swap(currentPairs);
        }

    } // namespace Core
} // namespace Kiaak
