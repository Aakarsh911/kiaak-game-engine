#include "Core/Camera.hpp"
#include "Core/GameObject.hpp"
#include "Core/Scene.hpp"
#include "Core/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

namespace Kiaak
{
    namespace Core
    {

        Camera *Camera::s_active = nullptr;

        Camera::Camera()
            : m_type(ProjectionType::Orthographic),
              m_zoom(1.0f),
              m_viewDirty(true),
              m_projDirty(true),
              m_view(1.0f),
              m_proj(1.0f)
        {
        }

        void Camera::Start()
        {
            if (!s_active)
                SetActive();
        }

        void Camera::SetActive()
        {
            s_active = this;
        }

        Camera *Camera::GetActive()
        {
            return s_active;
        }

        void Camera::SetProjectionType(ProjectionType t)
        {
            if (m_type != t)
            {
                m_type = t;
                m_projDirty = true;
            }
        }

        void Camera::SetZoom(float zoom)
        {
            if (zoom <= 0.0f)
                zoom = 0.0001f;
            if (m_zoom != zoom)
            {
                m_zoom = zoom;
                m_projDirty = true;
            }
        }

        void Camera::SetOrthographicSize(float size)
        {
            if (size <= 0.0f)
                size = 0.0001f;
            if (m_orthographicSize != size)
            {
                m_orthographicSize = size;
                m_projDirty = true;
            }
        }

        const glm::mat4 &Camera::GetView() const
        {
            if (m_viewDirty)
            {
                RecalculateView();
                m_viewDirty = false;
            }
            return m_view;
        }

        const glm::mat4 &Camera::GetProjection() const
        {
            // If the framebuffer/viewport size changed, the aspect changed → rebuild P.
            static int lastVPW = -1, lastVPH = -1; // shared across cams is fine for 2D/editor
            GLint vp[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_VIEWPORT, vp);
            const int vpw = vp[2];
            const int vph = vp[3];
            if (vpw != lastVPW || vph != lastVPH)
            {
                lastVPW = vpw;
                lastVPH = vph;
                m_projDirty = true; // (mutable) safe to flip inside const
            }

            if (m_projDirty)
            {
                RecalculateProjection();
                m_projDirty = false;
            }
            return m_proj;
        }

        void Camera::RecalculateView() const
        {
            // Use inverse of the GameObject transform as the view
            // (camera looks down -Z with up +Y; 2D sprites live in XY plane)
            const Transform *t = GetGameObject()->GetTransform();
            glm::mat4 model = t ? t->GetTransformMatrix() : glm::mat4(1.0f);
            m_view = glm::inverse(model);
        }

        void Camera::RecalculateProjection() const
        {
            GLint vp[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_VIEWPORT, vp);
            float w = static_cast<float>(vp[2]);
            float h = static_cast<float>(vp[3]);
            if (w <= 0.0f)
                w = 1.0f;
            if (h <= 0.0f)
                h = 1.0f;

            float aspectRatio = w / h;

            float orthoSize = m_orthographicSize / m_zoom;
            float halfHeight = orthoSize;
            float halfWidth = halfHeight * aspectRatio;

            // Near/far for 2D
            float nearZ = -1000.0f;
            float farZ = 1000.0f;

            m_proj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearZ, farZ);
        }

        void Camera::SetFollowTargetByID(uint32_t id)
        {
            m_followTargetID = id;
        }

        // Note: follow logic implemented in Update by resolving the ID to a GameObject
        void Camera::Update(double /*deltaTime*/)
        {
            // If transform moved/rotated/scaled, mark view dirty.
            m_viewDirty = true;

            // If follow target set, resolve and snap camera to target position (preserve Z)
            if (m_followTargetID != 0)
            {
                // Resolve owning scene via GameObject lookup (SceneManager available via GameObject?)
                // Simpler approach: GetGameObject() is a member on Component; use scene lookup from that GameObject.
                auto *owner = GetGameObject();
                if (owner)
                {
                    auto *scene = owner->GetScene();
                    if (scene)
                    {
                        if (auto *target = scene->GetGameObject(m_followTargetID))
                        {
                            if (auto *t = target->GetTransform())
                            {
                                glm::vec3 p = t->GetPosition();
                                // Set camera's GameObject transform to target (preserve Z offset of camera)
                                if (auto *camTr = owner->GetTransform())
                                {
                                    glm::vec3 camPos = camTr->GetPosition();
                                    camPos.x = p.x;
                                    camPos.y = p.y;
                                    camTr->SetPosition(camPos);
                                    InvalidateView();
                                }
                            }
                        }
                    }
                }
            }

            GLint vp[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_VIEWPORT, vp);
            static int lastW = -1, lastH = -1;
            if (vp[2] != lastW || vp[3] != lastH)
            {
                m_projDirty = true;
                lastW = vp[2];
                lastH = vp[3];
            }
        }

    } // namespace Core
} // namespace Kiaak
