#include "Core/SceneSerialization.hpp"
#include "Core/SceneManager.hpp"
#include "Core/GameObject.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include "Core/Animator.hpp"
#include "Core/Rigidbody2D.hpp"
#include "Editor/EditorUI.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace Kiaak::Core
{
    static void Trim(std::string &s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
    }

    static void WriteGameObject(std::ostream &out, GameObject *go)
    {
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
        if (auto *anim = go->GetComponent<Kiaak::Core::Animator>())
            out << "    ANIMATOR clipIndex " << anim->GetClipIndex() << "\n";
        if (auto *cam = go->GetComponent<Camera>())
            out << "    CAMERA orthoSize " << cam->GetOrthographicSize() << " zoom " << cam->GetZoom() << "\n";

        if (auto *rb = go->GetComponent<Rigidbody2D>())
        {
            const char *typeStr = "Dynamic";
            switch (rb->GetBodyType())
            {
            case Rigidbody2D::BodyType::Static: typeStr = "Static"; break;
            case Rigidbody2D::BodyType::Kinematic: typeStr = "Kinematic"; break;
            case Rigidbody2D::BodyType::Dynamic: default: typeStr = "Dynamic"; break;
            }
            out << "    RIGIDBODY2D "
                << "type " << typeStr
                << " mass " << rb->GetMass()
                << " damping " << rb->GetLinearDamping()
                << " gravityScale " << rb->GetGravityScale()
                << " useGravity " << (rb->GetUseGravity() ? 1 : 0)
                << "\n";
        }
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
            if (scene->GetDesignatedCamera())
            {
                if (auto *go = scene->GetDesignatedCamera()->GetGameObject())
                    out << "  ACTIVE_CAMERA " << go->GetName() << "\n";
            }
            // Per-scene physics settings
            if (auto *phys = scene->GetPhysics2D())
            {
                auto g = phys->GetGravity();
                out << "  PHYSICS2D gravity " << g.x << ' ' << g.y << "\n";
            }
            for (auto *go : scene->GetAllGameObjects())
            {
                if (!go)
                    continue;
                if (go->GetName().rfind("EditorCamera", 0) == 0)
                    continue;
                WriteGameObject(out, go);
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
                    currentScene->ClearAllGameObjects();
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
            else if (token == "PHYSICS2D" && currentScene)
            {
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "gravity")
                    {
                        float gx = 0.0f, gy = -9.81f;
                        iss >> gx >> gy;
                        currentScene->GetPhysics2D()->SetGravity({gx, gy});
                    }
                }
            }
            else if (token == "TRANSFORM" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                glm::vec3 pos{}, rot{}, scale{1.0f};
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "pos")
                        iss >> pos.x >> pos.y >> pos.z;
                    else if (lbl == "rot")
                        iss >> rot.x >> rot.y >> rot.z;
                    else if (lbl == "scale")
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
            else if (token == "ANIMATOR" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string sub; // expect clipIndex
                iss >> sub;
                int idx = -1;
                iss >> idx;
                if (idx >= 0)
                {
                    auto *anim = go->GetComponent<Kiaak::Core::Animator>();
                    if (!anim)
                        anim = go->AddComponent<Kiaak::Core::Animator>();
                    if (anim)
                    {
                        anim->SetClipIndex(idx);
                        Kiaak::EditorUI::SetAssignedClip(go, idx);
                    }
                }
            }
            else if (token == "CAMERA" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                float ortho = 10.0f, zoom = 1.0f;
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "orthoSize")
                        iss >> ortho;
                    else if (lbl == "zoom")
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
            else if (token == "RIGIDBODY2D" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string lbl;
                std::string typeStr = "Dynamic";
                float mass = 1.0f;
                float damping = 0.0f;
                float gravityScale = 1.0f;
                int useGravity = 1;
                while (iss >> lbl)
                {
                    if (lbl == "type")
                        iss >> typeStr;
                    else if (lbl == "mass")
                        iss >> mass;
                    else if (lbl == "damping")
                        iss >> damping;
                    else if (lbl == "gravityScale")
                        iss >> gravityScale;
                    else if (lbl == "useGravity")
                        iss >> useGravity;
                }
                auto *rb = go->GetComponent<Rigidbody2D>();
                if (!rb) rb = go->AddComponent<Rigidbody2D>();
                if (rb)
                {
                    Rigidbody2D::BodyType bt = Rigidbody2D::BodyType::Dynamic;
                    if (typeStr == "Static") bt = Rigidbody2D::BodyType::Static;
                    else if (typeStr == "Kinematic") bt = Rigidbody2D::BodyType::Kinematic;
                    rb->SetBodyType(bt);
                    rb->SetMass(mass);
                    rb->SetLinearDamping(damping);
                    rb->SetGravityScale(gravityScale);
                    rb->SetUseGravity(useGravity != 0);
                }
            }
        }
        for (auto &[sc, goName] : pendingActive)
        {
            if (!sc)
                continue;
            auto *go = sc->GetGameObject(goName);
            if (!go)
                continue;
            if (auto *cam = go->GetComponent<Camera>())
                sc->SetDesignatedCamera(cam);
        }
        // Clean rewrite
        SaveAllScenes(manager, filePath);
        return true;
    }

    bool SceneSerialization::SaveSceneToFile(Scene *scene, const std::string &filePath)
    {
        if (!scene)
            return false;
        std::ofstream out(filePath);
        if (!out.is_open())
            return false;
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
        if (auto *phys = scene->GetPhysics2D())
        {
            auto g = phys->GetGravity();
            out << "  PHYSICS2D gravity " << g.x << ' ' << g.y << "\n";
        }
        for (auto *go : scene->GetAllGameObjects())
        {
            if (!go)
                continue;
            if (go->GetName().rfind("EditorCamera", 0) == 0)
                continue;
            WriteGameObject(out, go);
        }
        return true;
    }

    bool SceneSerialization::LoadSceneFromFile(SceneManager *manager, const std::string &filePath)
    {
        // Delegate to LoadAllScenes conceptually if format is the same, but implement minimally here
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
            else if (token == "PHYSICS2D" && currentScene)
            {
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "gravity")
                    {
                        float gx = 0.0f, gy = -9.81f;
                        iss >> gx >> gy;
                        currentScene->GetPhysics2D()->SetGravity({gx, gy});
                    }
                }
            }
            else if (token == "TRANSFORM" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                glm::vec3 pos{}, rot{}, scale{1.0f};
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "pos")
                        iss >> pos.x >> pos.y >> pos.z;
                    else if (lbl == "rot")
                        iss >> rot.x >> rot.y >> rot.z;
                    else if (lbl == "scale")
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
            else if (token == "ANIMATOR" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string sub;
                iss >> sub;
                int idx = -1;
                iss >> idx;
                if (idx >= 0)
                {
                    auto *anim = go->GetComponent<Kiaak::Core::Animator>();
                    if (!anim)
                        anim = go->AddComponent<Kiaak::Core::Animator>();
                    if (anim)
                    {
                        anim->SetClipIndex(idx);
                        Kiaak::EditorUI::SetAssignedClip(go, idx);
                    }
                }
            }
            else if (token == "CAMERA" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                float ortho = 10.0f, zoom = 1.0f;
                std::string lbl;
                while (iss >> lbl)
                {
                    if (lbl == "orthoSize")
                        iss >> ortho;
                    else if (lbl == "zoom")
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
            else if (token == "RIGIDBODY2D" && currentScene)
            {
                auto objs = currentScene->GetAllGameObjects();
                if (objs.empty())
                    continue;
                auto *go = objs.back();
                std::string lbl;
                std::string typeStr = "Dynamic";
                float mass = 1.0f;
                float damping = 0.0f;
                float gravityScale = 1.0f;
                int useGravity = 1;
                while (iss >> lbl)
                {
                    if (lbl == "type")
                        iss >> typeStr;
                    else if (lbl == "mass")
                        iss >> mass;
                    else if (lbl == "damping")
                        iss >> damping;
                    else if (lbl == "gravityScale")
                        iss >> gravityScale;
                    else if (lbl == "useGravity")
                        iss >> useGravity;
                }
                auto *rb = go->GetComponent<Rigidbody2D>();
                if (!rb) rb = go->AddComponent<Rigidbody2D>();
                if (rb)
                {
                    Rigidbody2D::BodyType bt = Rigidbody2D::BodyType::Dynamic;
                    if (typeStr == "Static") bt = Rigidbody2D::BodyType::Static;
                    else if (typeStr == "Kinematic") bt = Rigidbody2D::BodyType::Kinematic;
                    rb->SetBodyType(bt);
                    rb->SetMass(mass);
                    rb->SetLinearDamping(damping);
                    rb->SetGravityScale(gravityScale);
                    rb->SetUseGravity(useGravity != 0);
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
