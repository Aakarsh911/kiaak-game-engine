#include "Core/Project.hpp"
#include <algorithm>

namespace Kiaak::Core
{

    std::string Project::s_path;

    static std::string Normalize(const std::string &p)
    {
        if (p.empty())
            return p;
        std::string r = p;
        // remove trailing slashes
#ifdef _WIN32
        while (!r.empty() && (r.back() == '/' || r.back() == '\\'))
            r.pop_back();
#else
        while (!r.empty() && r.back() == '/')
            r.pop_back();
#endif
        return r;
    }

    void Project::SetPath(const std::string &path)
    {
        s_path = Normalize(path);
    }

    const std::string &Project::GetPath() { return s_path; }

    bool Project::HasPath() { return !s_path.empty(); }

    std::string Project::GetAssetsPath() { return HasPath() ? s_path + "/assets" : "assets"; }
    std::string Project::GetScenesPath() { return HasPath() ? s_path + "/scenes" : "scenes"; }

    bool Project::EnsureStructure()
    {
        namespace fs = std::filesystem;
        try
        {
            if (!HasPath())
                return false;
            auto assets = GetAssetsPath();
            auto scenes = GetScenesPath();
            if (!fs::exists(scenes))
                fs::create_directories(scenes);
            if (!fs::exists(assets))
                fs::create_directories(assets);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

} // namespace Kiaak::Core
