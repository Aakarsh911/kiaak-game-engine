#pragma once

#include "Core/Scene.hpp"
#include <string>

namespace Kiaak
{
    namespace Core
    {

        class SceneManager; // fwd

        // Simple prototype text serialization.
        // Format:
        // SCENE <name>
        //   GAMEOBJECT <name>
        //     TRANSFORM pos x y z rot x y z scale x y z
        //     SPRITE texture <path>
        // Added:
        //   CAMERA orthoSize <f> zoom <f>
        //   ACTIVE_CAMERA <gameobject_name>
        // (Blank line separates scenes)
        class SceneSerialization
        {
        public:
            static bool SaveAllScenes(SceneManager *manager, const std::string &filePath);
            static bool LoadAllScenes(SceneManager *manager, const std::string &filePath);
            // New per-scene API (writes one file per scene)
            static bool SaveSceneToFile(Scene *scene, const std::string &filePath);
            static bool LoadSceneFromFile(SceneManager *manager, const std::string &filePath); // derives scene name from file
        };

    }
} // namespace Kiaak::Core
