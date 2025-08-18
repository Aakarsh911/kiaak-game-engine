#include "Core/Camera.hpp"
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h> // for glGetIntegerv(GL_VIEWPORT,...)

namespace Kiaak {
namespace Core {

Camera* Camera::s_active = nullptr;

Camera::Camera()
    : m_type(ProjectionType::Orthographic),
      m_zoom(1.0f),
      m_viewDirty(true),
      m_projDirty(true),
      m_view(1.0f),
      m_proj(1.0f)
{
}

void Camera::Start() {
    // If no active camera yet, make this one active by default.
    if (!s_active) SetActive();
}

void Camera::Update(double /*deltaTime*/) {
    // If transform moved/rotated/scaled, mark view dirty.
    // Easiest approach: always mark it dirty each frame (cheap for 2D).
    m_viewDirty = true;

    // Projection only needs update when viewport/zoom changes.
    // We’ll still check viewport size each frame and update if it changed.
    // Read current GL viewport:
    GLint vp[4] = {0,0,0,0};
    glGetIntegerv(GL_VIEWPORT, vp);
    static int lastW = -1, lastH = -1;
    if (vp[2] != lastW || vp[3] != lastH) {
        m_projDirty = true;
        lastW = vp[2];
        lastH = vp[3];
    }
}

void Camera::SetActive() {
    s_active = this;
}

Camera* Camera::GetActive() {
    return s_active;
}

void Camera::SetProjectionType(ProjectionType t) {
    if (m_type != t) {
        m_type = t;
        m_projDirty = true;
    }
}

void Camera::SetZoom(float zoom) {
    if (zoom <= 0.0f) zoom = 0.0001f;
    if (m_zoom != zoom) {
        m_zoom = zoom;
        m_projDirty = true;
    }
}

const glm::mat4& Camera::GetView() const {
    if (m_viewDirty) {
        RecalculateView();
        m_viewDirty = false;
    }
    return m_view;
}

const glm::mat4& Camera::GetProjection() const {
    if (m_projDirty) {
        RecalculateProjection();
        m_projDirty = false;
    }
    return m_proj;
}

void Camera::RecalculateView() const {
    // Use inverse of the GameObject transform as the view
    // (camera looks down -Z with up +Y; 2D sprites live in XY plane)
    const Transform* t = GetGameObject()->GetTransform();
    glm::mat4 model = t ? t->GetTransformMatrix() : glm::mat4(1.0f);
    m_view = glm::inverse(model);
}

void Camera::RecalculateProjection() const {
    // Build an orthographic projection in pixel space scaled by zoom.
    // Viewport (x,y,w,h)
    GLint vp[4] = {0,0,0,0};
    glGetIntegerv(GL_VIEWPORT, vp);
    float w = static_cast<float>(vp[2]);
    float h = static_cast<float>(vp[3]);
    if (w <= 0.0f) w = 1.0f;
    if (h <= 0.0f) h = 1.0f;

    // Centered ortho that matches your SpriteRenderer’s previous convention
    float halfW = 0.5f * w / m_zoom;
    float halfH = 0.5f * h / m_zoom;

    // Near/far for 2D
    float nearZ = -1000.0f;
    float farZ  =  1000.0f;

    m_proj = glm::ortho(-halfW, halfW, -halfH, halfH, nearZ, farZ);
}

} // namespace Core
} // namespace Kiaak
