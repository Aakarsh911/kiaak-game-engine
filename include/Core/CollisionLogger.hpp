#pragma once
#include "Component.hpp"
#include <string>
namespace Kiaak
{
    namespace Core
    {
        class Collider2D;
        // Simple component that logs collision & trigger events to stdout.
        class CollisionLogger : public Component
        {
        public:
            std::string GetTypeName() const override { return "CollisionLogger"; }
            virtual ~CollisionLogger() = default;
            void OnCollisionEnter(Collider2D *other) override;
            void OnCollisionStay(Collider2D *other) override;
            void OnCollisionExit(Collider2D *other) override;
            void OnTriggerEnter(Collider2D *other) override;
            void OnTriggerStay(Collider2D *other) override;
            void OnTriggerExit(Collider2D *other) override;
        };
    }
} // namespace
