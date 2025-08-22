#pragma once

#include <string>
#include <filesystem>

namespace Kiaak::Core
{

    // Simple global project context (prototype).
    // A project directory contains subfolders:
    //   assets/  (imported assets)
    //   scenes/  (individual scene files <SceneName>.scene)
    class Project
    {
    public:
        static void SetPath(const std::string &path);
        static const std::string &GetPath();
        static bool HasPath();

        static std::string GetAssetsPath();
        static std::string GetScenesPath();

        static bool EnsureStructure(); // creates assets & scenes directories

    private:
        static std::string s_path; // normalized (no trailing slash)
    };

} // namespace Kiaak::Core
