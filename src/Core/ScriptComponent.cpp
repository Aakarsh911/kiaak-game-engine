#include "Core/ScriptComponent.hpp"
#include "Engine.hpp"
#include <sol/sol.hpp>
#include <filesystem>
#include "Core/Project.hpp"
#include <iostream>

namespace Kiaak::Core
{

    void ScriptComponent::Start()
    {
        auto engine = Kiaak::Engine::Get();
        if (!engine)
            return;

        sol::state *lua = engine->GetLua();
        if (!lua)
            return;

        try
        {
            std::string scriptToLoad = m_scriptPath;
            if (Kiaak::Core::Project::HasPath())
            {
                if (!scriptToLoad.empty() && scriptToLoad.front() != '/')
                {
                    scriptToLoad = Kiaak::Core::Project::GetPath() + "/" + scriptToLoad;
                }
            }

            if (!std::filesystem::exists(scriptToLoad))
            {
                std::cerr << "ScriptComponent: script file not found: '" << scriptToLoad << "'\n";
            }

            lua->script_file(scriptToLoad, &sol::script_pass_on_error);
            m_updateFunc = (*lua)["update"];
            std::cout << "ScriptComponent: Start() loaded '" << scriptToLoad << "'\n";
            std::cout << "ScriptComponent: update function " << (m_updateFunc.valid() ? "found" : "missing") << "\n";
        }
        catch (const sol::error &e)
        {
            std::cerr << "Lua load error: " << e.what() << std::endl;
        }
    }

    void ScriptComponent::Update(double dt)
    {
        // Only execute script updates while in play mode
        auto engine = Kiaak::Engine::Get();
        if (engine && engine->IsEditorMode())
            return;

        if (m_updateFunc.valid())
        {
            if (m_updateCalls == 0)
            {
                std::cout << "ScriptComponent: Update() calling update for '" << m_scriptPath << "'\n";
            }
            ++m_updateCalls;
            sol::protected_function_result r = m_updateFunc(dt);
            if (!r.valid())
            {
                sol::error err = r;
                std::cerr << "Lua runtime error: " << err.what() << std::endl;
            }
        }
    }

}
