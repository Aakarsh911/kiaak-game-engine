#include "Core/Physics2D.hpp"
#include "Core/Rigidbody2D.hpp"
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include <algorithm>

namespace Kiaak {
namespace Core {

Physics2D::Physics2D() : m_gravity(0.0f, -9.81f) {}

void Physics2D::RegisterBody(Rigidbody2D* rb) {
    if (!rb) return;
    auto it = std::find_if(m_bodies.begin(), m_bodies.end(), [rb](const BodyRec& r){return r.rb==rb;});
    if (it == m_bodies.end()) m_bodies.push_back({rb});
}

void Physics2D::UnregisterBody(Rigidbody2D* rb) {
    if (!rb) return;
    m_bodies.erase(std::remove_if(m_bodies.begin(), m_bodies.end(), [rb](const BodyRec& r){return r.rb==rb;}), m_bodies.end());
}

void Physics2D::Step(double dt) {
    const float fdt = static_cast<float>(dt);
    for (auto it = m_bodies.begin(); it != m_bodies.end(); ) {
        Rigidbody2D* rb = it->rb;
        if (!rb) { it = m_bodies.erase(it); continue; }

        if (rb->GetBodyType() == Rigidbody2D::BodyType::Dynamic) {
            // Integrate forces -> velocity
            glm::vec2 vel = rb->GetVelocity();
            glm::vec2 acc(0.0f);
            if (rb->GetUseGravity()) {
                acc = m_gravity * rb->GetGravityScale();
            }
            vel += acc * fdt;
            // simple linear damping
            vel *= (1.0f / (1.0f + rb->GetLinearDamping() * fdt));

            rb->SetVelocity(vel);

            // Integrate velocity -> position
            if (auto* go = rb->GetGameObject()) {
                if (auto* t = go->GetTransform()) {
                    glm::vec3 p = t->GetPosition();
                    p.x += vel.x * fdt;
                    p.y += vel.y * fdt;
                    t->SetPosition(p);
                }
            }
        } else if (rb->GetBodyType() == Rigidbody2D::BodyType::Kinematic) {
            // Kinematic: user sets velocity directly
            glm::vec2 vel = rb->GetVelocity();
            if (auto* go = rb->GetGameObject()) {
                if (auto* t = go->GetTransform()) {
                    glm::vec3 p = t->GetPosition();
                    p.x += vel.x * fdt;
                    p.y += vel.y * fdt;
                    t->SetPosition(p);
                }
            }
        } else {
            // Static: do nothing
        }
        ++it;
    }
}

} // namespace Core
} // namespace Kiaak
