#include "Core/SceneSerialization.hpp"
#include "Core/SceneManager.hpp"
#include "Core/GameObject.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace Kiaak::Core
{

    static void Trim(std::string &s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
    }

    bool SceneSerialization::SaveAllScenes(SceneManager *manager, const std::string &filePath)
    {
        if (!manager)
            return false;
        std::ofstream out(filePath);
        if (!out.is_open())
            return false;

        auto names = manager->GetSceneNames();
        for (size_t i = 0; i < names.size(); ++i)
        {
            auto *scene = manager->GetScene(names[i]);
            if (!scene)
                continue;
            out << "SCENE " << names[i] << "\n";

            // Active camera (designated) first
            if (scene->GetDesignatedCamera())
            {
                if (auto *go = scene->GetDesignatedCamera()->GetGameObject())
                {
                    out << "  ACTIVE_CAMERA " << go->GetName() << "\n";
                }
            }

            auto objects = scene->GetAllGameObjects();
            for (auto *go : objects)
            {
                if (!go)
                    continue;
                // Skip any editor camera variants (prefix match)
                const std::string &gn = go->GetName();
                if (gn.rfind("EditorCamera", 0) == 0)
                    continue; // skip editor camera(s)
                out << "  GAMEOBJECT " << go->GetName() << "\n";
                if (auto *tr = go->GetTransform())
                {
                    auto p = tr->GetPosition();
                    auto r = tr->GetRotation();
                    auto sc = tr->GetScale();
                    out << "    TRANSFORM pos " << p.x << ' ' << p.y << ' ' << p.z
                        << " rot " << r.x << ' ' << r.y << ' ' << r.z
                        << " scale " << sc.x << ' ' << sc.y << ' ' << sc.z << "\n";
                }
                if (auto *sr = go->GetComponent<Graphics::SpriteRenderer>())
                {
                    out << "    SPRITE texture " << sr->GetTexturePath() << "\n";
                }
                if (auto *cam = go->GetComponent<Camera>())
                {
                    out << "    CAMERA orthoSize " << cam->GetOrthographicSize() << " zoom " << cam->GetZoom() << "\n";
                }
            }
            if (i + 1 < names.size())
                out << "\n";
        }
        return true;
    }

    bool SceneSerialization::LoadAllScenes(SceneManager *manager, const std::string &filePath)
    {
        if (!manager)
            return false;
        std::ifstream in(filePath);
        if (!in.is_open())
            return false;

        std::vector<std::pair<Scene *, std::string>> pendingActive;

        std::string line;
        Scene *currentScene = nullptr;
        while (std::getline(in, line))
        {
            Trim(line);
            if (line.empty())
                continue;
            std::istringstream iss(line);
            std::string token;
            iss >> token;
            if (token == "SCENE")
            {
                std::string sceneName;
                iss >> sceneName;
                bool existed = manager->HasScene(sceneName);
                if (!existed)
                    manager->CreateScene(sceneName);
                manager->SwitchToScene(sceneName);
                currentScene = manager->GetCurrentScene();
                if (currentScene && existed)
                {
                    // Clear existing game objects before reloading to prevent duplication
                    currentScene->ClearAllGameObjects();
                }
            }
            else if (token == "ACTIVE_CAMERA" && currentScene)
            {
                std::string goName;
                iss >> goName;
                pendingActive.emplace_back(currentScene, goName);
            }
            else if (token == "GAMEOBJECT" && currentScene)
            {
                std::string goName;
                iss >> goName;
                if (goName.rfind("EditorCamera", 0) == 0)
                    continue; // skip editor-only object(s) from saved file
                currentScene->CreateGameObject(goName);
            }
            else if (token == "TRANSFORM" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                glm::vec3 pos{}, rot{}, scale{1.0f};
                std::string label;
                while (iss >> label)
                {
                    if (label == "pos")
                        iss >> pos.x >> pos.y >> pos.z;
                    else if (label == "rot")
                        iss >> rot.x >> rot.y >> rot.z;
                    else if (label == "scale")
                        iss >> scale.x >> scale.y >> scale.z;
                }
                if (auto *tr = go->GetTransform())
                {
                    tr->SetPosition(pos);
                    tr->SetRotation(rot);
                    tr->SetScale(scale);
                }
            }
            else if (token == "SPRITE" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string sub;
                iss >> sub;
                std::string path;
                iss >> path;
                if (!path.empty())
                {
                    auto *sr = go->GetComponent<Graphics::SpriteRenderer>();
                    if (!sr)
                        sr = go->AddComponent<Graphics::SpriteRenderer>();
                    if (sr)
                        sr->SetTexture(path);
                }
            }
            else if (token == "CAMERA" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                float orthoSize = 10.0f;
                float zoom = 1.0f;
                std::string label;
                while (iss >> label)
                {
                    if (label == "orthoSize")
                        iss >> orthoSize;
                    else if (label == "zoom")
                        iss >> zoom;
                }
                auto *cam = go->GetComponent<Camera>();
                if (!cam)
                    cam = go->AddComponent<Camera>();
                if (cam)
                {
                    cam->SetOrthographicSize(orthoSize);
                    cam->SetZoom(zoom);
                }
            }
        }

        // Resolve pending active cameras
        for (auto &[scene, goName] : pendingActive)
        {
            if (!scene)
                continue;
            auto *go = scene->GetGameObject(goName);
            if (!go)
                continue;
            if (auto *cam = go->GetComponent<Camera>())
                scene->SetDesignatedCamera(cam);
        }
        // After load, immediately rewrite file to purge any previously serialized editor-only objects
        SaveAllScenes(manager, filePath);
        return true;
    }

    // --- New per-scene file format helpers (reuse same text structure but single scene per file) ---
    bool SceneSerialization::SaveSceneToFile(Scene *scene, const std::string &filePath)
    {
        if (!scene)
            return false;
        // We need a scene name; since Scene itself doesn't store name, caller passes path encoded with it.
        // We'll infer name from filename (strip extension) when loading.
        std::ofstream out(filePath);
        if (!out.is_open())
            return false;
        // Derive scene name
        std::string sceneName;
        {
            std::filesystem::path p(filePath);
            sceneName = p.stem().string();
        }
        out << "SCENE " << sceneName << "\n";
        if (scene->GetDesignatedCamera())
        {
            if (auto *go = scene->GetDesignatedCamera()->GetGameObject())
                out << "  ACTIVE_CAMERA " << go->GetName() << "\n";
        }
        auto objects = scene->GetAllGameObjects();
        for (auto *go : objects)
        {
            if (!go)
                continue;
            const std::string &gn = go->GetName();
            if (gn.rfind("EditorCamera", 0) == 0)
                continue;
            out << "  GAMEOBJECT " << go->GetName() << "\n";
            if (auto *tr = go->GetTransform())
            {
                auto p = tr->GetPosition();
                auto r = tr->GetRotation();
                auto sc = tr->GetScale();
                out << "    TRANSFORM pos " << p.x << ' ' << p.y << ' ' << p.z
                    << " rot " << r.x << ' ' << r.y << ' ' << r.z
                    << " scale " << sc.x << ' ' << sc.y << ' ' << sc.z << "\n";
            }
            if (auto *sr = go->GetComponent<Graphics::SpriteRenderer>())
                out << "    SPRITE texture " << sr->GetTexturePath() << "\n";
            if (auto *cam = go->GetComponent<Camera>())
                out << "    CAMERA orthoSize " << cam->GetOrthographicSize() << " zoom " << cam->GetZoom() << "\n";
        }
        return true;
    }

    bool SceneSerialization::LoadSceneFromFile(SceneManager *manager, const std::string &filePath)
    {
        if (!manager)
            return false;
        std::ifstream in(filePath);
        if (!in.is_open())
            return false;
        std::string line;
        Scene *currentScene = nullptr;
        std::vector<std::pair<Scene *, std::string>> pendingActive;
        while (std::getline(in, line))
        {
            Trim(line);
            if (line.empty())
                continue;
            std::istringstream iss(line);
            std::string token;
            iss >> token;
            if (token == "SCENE")
            {
                std::string sceneName;
                iss >> sceneName;
                if (!manager->HasScene(sceneName))
                    manager->CreateScene(sceneName);
                manager->SwitchToScene(sceneName);
                currentScene = manager->GetCurrentScene();
            }
            else if (token == "ACTIVE_CAMERA" && currentScene)
            {
                std::string goName;
                iss >> goName;
                pendingActive.emplace_back(currentScene, goName);
            }
            else if (token == "GAMEOBJECT" && currentScene)
            {
                std::string goName;
                iss >> goName;
                if (goName.rfind("EditorCamera", 0) == 0)
                    continue;
                currentScene->CreateGameObject(goName);
            }
            else if (token == "TRANSFORM" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                glm::vec3 pos{}, rot{}, scale{1.0f};
                std::string label;
                while (iss >> label)
                {
                    if (label == "pos")
                        iss >> pos.x >> pos.y >> pos.z;
                    else if (label == "rot")
                        iss >> rot.x >> rot.y >> rot.z;
                    else if (label == "scale")
                        iss >> scale.x >> scale.y >> scale.z;
                }
                if (auto *tr = go->GetTransform())
                {
                    tr->SetPosition(pos);
                    tr->SetRotation(rot);
                    tr->SetScale(scale);
                }
            }
            else if (token == "SPRITE" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string sub;
                iss >> sub;
                std::string path;
                iss >> path;
                if (!path.empty())
                {
                    auto *sr = go->GetComponent<Graphics::SpriteRenderer>();
                    if (!sr)
                        sr = go->AddComponent<Graphics::SpriteRenderer>();
                    if (sr)
                        sr->SetTexture(path);
                }
            }
            else if (token == "CAMERA" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                float ortho = 10.0f, zoom = 1.0f;
                std::string label;
                while (iss >> label)
                {
                    if (label == "orthoSize")
                        iss >> ortho;
                    else if (label == "zoom")
                        iss >> zoom;
                }
                auto *cam = go->GetComponent<Camera>();
                if (!cam)
                    cam = go->AddComponent<Camera>();
                if (cam)
                {
                    cam->SetOrthographicSize(ortho);
                    cam->SetZoom(zoom);
                }
            }
        }
        for (auto &[sc, name] : pendingActive)
        {
            if (!sc)
                continue;
            auto *go = sc->GetGameObject(name);
            if (!go)
                continue;
            if (auto *cam = go->GetComponent<Camera>())
                sc->SetDesignatedCamera(cam);
        }
        return true;
    }

} // namespace Kiaak::Core
