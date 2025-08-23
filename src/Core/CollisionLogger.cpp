#include "Core/CollisionLogger.hpp"
#include "Core/GameObject.hpp"
#include "Core/Collider2D.hpp"
#include <iostream>

namespace Kiaak
{
    namespace Core
    {

        static std::string NameOf(Collider2D *col)
        {
            if (!col)
                return "<null>";
            if (auto *go = col->GetGameObject())
                return go->GetName();
            return "<orphan collider>";
        }

        void CollisionLogger::OnCollisionEnter(Collider2D *other) { std::cout << "[Collision Enter] " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }
        void CollisionLogger::OnCollisionStay(Collider2D *other) { std::cout << "[Collision Stay]  " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }
        void CollisionLogger::OnCollisionExit(Collider2D *other) { std::cout << "[Collision Exit]  " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }
        void CollisionLogger::OnTriggerEnter(Collider2D *other) { std::cout << "[Trigger Enter]   " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }
        void CollisionLogger::OnTriggerStay(Collider2D *other) { std::cout << "[Trigger Stay]    " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }
        void CollisionLogger::OnTriggerExit(Collider2D *other) { std::cout << "[Trigger Exit]    " << (GetGameObject() ? GetGameObject()->GetName() : "<owner>") << " with " << NameOf(other) << "\n"; }

    }
} // namespace
