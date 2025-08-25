#pragma once

#include "Core/Component.hpp"
#include <string>

namespace Kiaak::Core
{

    class ScriptComponent : public Component
    {
    public:
        ScriptComponent() = default;
        explicit ScriptComponent(const std::string &path) : m_scriptPath(path) {}
        ~ScriptComponent() override = default;

        std::string GetTypeName() const override { return "Script"; }

        void SetScriptPath(const std::string &p) { m_scriptPath = p; }
        const std::string &GetScriptPath() const { return m_scriptPath; }

        void Start() override {}
        void Update(double) override {}

    private:
        std::string m_scriptPath; // relative to project root (e.g., scripts/MyScript.lua)
    };

} // namespace Kiaak::Core
