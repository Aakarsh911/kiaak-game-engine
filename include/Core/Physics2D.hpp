#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Kiaak {
namespace Core {

class Rigidbody2D;
class Transform;

// Lightweight 2D physics world (no collisions yet)
class Physics2D {
public:
    Physics2D();
    ~Physics2D() = default;

    void SetGravity(const glm::vec2& g) { m_gravity = g; }
    glm::vec2 GetGravity() const { return m_gravity; }

    // Advance simulation; integrates velocities and writes back to attached Transforms
    void Step(double dt);

    // Registration API used by Rigidbody2D
    void RegisterBody(Rigidbody2D* rb);
    void UnregisterBody(Rigidbody2D* rb);

private:
    struct BodyRec {
        Rigidbody2D* rb{nullptr};
    };

    glm::vec2 m_gravity;
    std::vector<BodyRec> m_bodies;
};

} // namespace Core
} // namespace Kiaak
