#pragma once

#include "Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Kiaak {
namespace Core {

/**
 * Transform component handles position, rotation, and scale of GameObjects
 * Every GameObject automatically has a Transform component
 */
class Transform : public Component {
public:
    Transform();
    ~Transform() override = default;

    // Position
    void SetPosition(const glm::vec3& position);
    void SetPosition(float x, float y, float z = 0.0f);
    const glm::vec3& GetPosition() const { return m_position; }

    // Rotation (in degrees)
    void SetRotation(const glm::vec3& rotation);
    void SetRotation(float x, float y, float z);
    void SetRotationZ(float angleDegrees); // Common for 2D
    const glm::vec3& GetRotation() const { return m_rotation; }

    // Scale
    void SetScale(const glm::vec3& scale);
    void SetScale(float x, float y, float z = 1.0f);
    void SetScale(float uniformScale);
    const glm::vec3& GetScale() const { return m_scale; }

    // Matrix calculations
    glm::mat4 GetTransformMatrix() const;
    glm::mat4 GetModelMatrix() const { return GetTransformMatrix(); }

    // Relative transformations
    void Translate(const glm::vec3& translation);
    void Translate(float x, float y, float z = 0.0f);
    void Rotate(const glm::vec3& rotation);
    void RotateZ(float angleDegrees);
    void Scale(const glm::vec3& scale);
    void Scale(float uniformScale);

    // Component interface
    std::string GetTypeName() const override { return "Transform"; }

private:
    glm::vec3 m_position;
    glm::vec3 m_rotation; // Euler angles in degrees
    glm::vec3 m_scale;

    // Cache for matrix calculation
    mutable glm::mat4 m_transformMatrix;
    mutable bool m_matrixDirty = true;

    void MarkMatrixDirty() { m_matrixDirty = true; }
    void UpdateMatrix() const;
};

} // namespace Core
} // namespace Kiaak
