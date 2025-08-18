#include "Core/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Kiaak {
namespace Core {

Transform::Transform() 
    : m_position(0.0f), m_rotation(0.0f), m_scale(1.0f), m_transformMatrix(1.0f) {
}

void Transform::SetPosition(const glm::vec3& position) {
    m_position = position;
    MarkMatrixDirty();
}

void Transform::SetPosition(float x, float y, float z) {
    SetPosition(glm::vec3(x, y, z));
}

void Transform::SetRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
    MarkMatrixDirty();
}

void Transform::SetRotation(float x, float y, float z) {
    SetRotation(glm::vec3(x, y, z));
}

void Transform::SetRotationZ(float angleDegrees) {
    m_rotation.z = angleDegrees;
    MarkMatrixDirty();
}

void Transform::SetScale(const glm::vec3& scale) {
    m_scale = scale;
    MarkMatrixDirty();
}

void Transform::SetScale(float x, float y, float z) {
    SetScale(glm::vec3(x, y, z));
}

void Transform::SetScale(float uniformScale) {
    SetScale(glm::vec3(uniformScale));
}

void Transform::Translate(const glm::vec3& translation) {
    m_position += translation;
    MarkMatrixDirty();
}

void Transform::Translate(float x, float y, float z) {
    Translate(glm::vec3(x, y, z));
}

void Transform::Rotate(const glm::vec3& rotation) {
    m_rotation += rotation;
    MarkMatrixDirty();
}

void Transform::RotateZ(float angleDegrees) {
    m_rotation.z += angleDegrees;
    MarkMatrixDirty();
}

void Transform::Scale(const glm::vec3& scale) {
    m_scale *= scale;
    MarkMatrixDirty();
}

void Transform::Scale(float uniformScale) {
    Scale(glm::vec3(uniformScale));
}

glm::mat4 Transform::GetTransformMatrix() const {
    if (m_matrixDirty) {
        UpdateMatrix();
    }
    return m_transformMatrix;
}

void Transform::UpdateMatrix() const {
    // Create transformation matrix: Translation * Rotation * Scale
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    
    // Convert degrees to radians and create rotation matrix
    glm::mat4 rotation = glm::eulerAngleXYZ(
        glm::radians(m_rotation.x),
        glm::radians(m_rotation.y),
        glm::radians(m_rotation.z)
    );
    
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);
    
    m_transformMatrix = translation * rotation * scale;
    m_matrixDirty = false;
}

} // namespace Core
} // namespace Kiaak
