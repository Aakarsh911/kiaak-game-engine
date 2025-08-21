// Core/Camera.hpp
#pragma once
#include "Core/Component.hpp"
#include "Core/Transform.hpp"
#include <glm/glm.hpp>
#include <string>

namespace Kiaak
{
    namespace Core
    {

        class Camera : public Component
        {
        public:
            enum class ProjectionType
            {
                Orthographic /*, Perspective (future) */
            };

            Camera();
            ~Camera() override = default;

            // ---- Component API ----
            std::string GetTypeName() const override { return "Camera"; } // <-- fixed
            void Start() override;
            void Update(double deltaTime) override;

            // ---- Camera API ----
            void SetActive();
            static Camera *GetActive();

            void SetProjectionType(ProjectionType t);
            ProjectionType GetProjectionType() const { return m_type; }

            void SetZoom(float zoom);
            float GetZoom() const { return m_zoom; }

            void SetOrthographicSize(float size);
            float GetOrthographicSize() const { return m_orthographicSize; }

            const glm::mat4 &GetView() const;
            const glm::mat4 &GetProjection() const;
            glm::mat4 GetViewProjection() const { return GetProjection() * GetView(); }

        private:
            void RecalculateView() const;
            void RecalculateProjection() const;

            static Camera *s_active;

            ProjectionType m_type{ProjectionType::Orthographic};
            float m_zoom{1.0f};
            float m_orthographicSize{5.0f}; // Half-height of camera view in world units

            mutable bool m_viewDirty{true};
            mutable bool m_projDirty{true};
            mutable glm::mat4 m_view{1.0f};
            mutable glm::mat4 m_proj{1.0f};
        };

    } // namespace Core
} // namespace Kiaak
