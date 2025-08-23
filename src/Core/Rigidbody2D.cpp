#include "Core/Rigidbody2D.hpp"
#include "Core/GameObject.hpp"
#include "Core/Scene.hpp"
#include "Core/Transform.hpp"

namespace Kiaak {
namespace Core {

Rigidbody2D::Rigidbody2D() = default;

void Rigidbody2D::Start() {
    if (auto* go = GetGameObject()) {
        if (auto* sc = go->GetScene()) {
            if (auto* phys = sc->GetPhysics2D()) {
                phys->RegisterBody(this);
                m_registered = true;
            }
        }
    }
}

void Rigidbody2D::FixedUpdate(double /*dt*/) {
    // Forces accumulated externally are handled by Physics2D::Step (not used yet)
}

void Rigidbody2D::OnDestroy() {
    if (!m_registered) return;
    if (auto* go = GetGameObject()) {
        if (auto* sc = go->GetScene()) {
            if (auto* phys = sc->GetPhysics2D()) {
                phys->UnregisterBody(this);
            }
        }
    }
    m_registered = false;
}

void Rigidbody2D::Teleport(const glm::vec2& pos, float rotZ) {
    if (auto* go = GetGameObject()) {
        if (auto* t = go->GetTransform()) {
            glm::vec3 p = t->GetPosition();
            p.x = pos.x; p.y = pos.y;
            t->SetPosition(p);
            t->SetRotationZ(rotZ);
        }
    }
}

} // namespace Core
} // namespace Kiaak
