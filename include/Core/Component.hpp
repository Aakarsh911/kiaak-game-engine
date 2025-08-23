#pragma once

#include <string>
#include <typeinfo>
#include <memory>

namespace Kiaak
{
    namespace Core
    {

        // Forward declaration
        class GameObject;

        /**
         * Base class for all components in the game engine
         * Components define behavior and data that can be attached to GameObjects
         */
        class Component
        {
        public:
            Component() = default;
            virtual ~Component() = default;

            // Component lifecycle
            virtual void Start() {}
            virtual void Update(double deltaTime) {}
            virtual void FixedUpdate(double fixedDeltaTime) {}
            virtual void OnDestroy() {}

            // Collision / Trigger callbacks (default no-op)
            // other points to the other collider involved in the interaction.
            // For triggers, only trigger callbacks fire; for solid collisions only collision callbacks fire.
            virtual void OnCollisionEnter(class Collider2D *other) {}
            virtual void OnCollisionStay(class Collider2D *other) {}
            virtual void OnCollisionExit(class Collider2D *other) {}
            virtual void OnTriggerEnter(class Collider2D *other) {}
            virtual void OnTriggerStay(class Collider2D *other) {}
            virtual void OnTriggerExit(class Collider2D *other) {}

            // Component state
            void SetEnabled(bool enabled) { m_enabled = enabled; }
            bool IsEnabled() const { return m_enabled; }

            // GameObject reference
            GameObject *GetGameObject() const { return m_gameObject; }

            // Component type identification
            virtual std::string GetTypeName() const = 0;

            // Template method to get type name for derived classes
            template <typename T>
            static std::string GetComponentTypeName()
            {
                return typeid(T).name();
            }

        protected:
            bool m_enabled = true;
            GameObject *m_gameObject = nullptr;

            friend class GameObject; // Allow GameObject to set the gameObject reference
        };

    } // namespace Core
} // namespace Kiaak
