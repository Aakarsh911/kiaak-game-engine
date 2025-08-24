#include "Core/Tilemap.hpp"
#include <algorithm>
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include "Core/Camera.hpp"
#include "Core/Project.hpp"
#include "Core/Scene.hpp"
#include "Core/Collider2D.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>

namespace Kiaak::Core
{
    std::shared_ptr<Kiaak::Shader> Tilemap::s_shader = nullptr;
    std::shared_ptr<Kiaak::VertexArray> Tilemap::s_vao = nullptr;
    std::shared_ptr<Kiaak::VertexBuffer> Tilemap::s_vbo = nullptr;
    int Tilemap::s_instances = 0;

    Tilemap::Tilemap()
        : m_width(16), m_height(16), m_tileWidth(1.0f), m_tileHeight(1.0f), m_hFrames(1), m_vFrames(1)
    {
        m_tiles.assign(m_width * m_height, -1);
        m_tileColliders.assign(m_hFrames * m_vFrames, 0);
        s_instances++;
    }

    void Tilemap::SetMapSize(int w, int h)
    {
        if (w <= 0 || h <= 0)
            return;
        m_width = w;
        m_height = h;
        m_tiles.assign(m_width * m_height, -1);
    }

    void Tilemap::SetTileSize(float w, float h)
    {
        if (w > 0)
            m_tileWidth = w;
        if (h > 0)
            m_tileHeight = h;
    }

    void Tilemap::SetTileset(const std::string &path, int hFrames, int vFrames)
    {
        m_texturePath = path;
        if (hFrames > 0)
            m_hFrames = hFrames;
        if (vFrames > 0)
            m_vFrames = vFrames;
        m_tileColliders.assign(m_hFrames * m_vFrames, 0);
        m_texture.reset();
    }

    void Tilemap::SetTile(int x, int y, int index)
    {
        if (x < 0 || y < 0 || x >= m_width || y >= m_height)
            return;
        m_tiles[y * m_width + x] = index;
    }

    int Tilemap::GetTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_width || y >= m_height)
            return -1;
        return m_tiles[y * m_width + x];
    }

    void Tilemap::SetTileColliderFlag(int frameIndex, bool solid)
    {
        if (frameIndex < 0 || frameIndex >= (int)m_tileColliders.size())
            return;
        m_tileColliders[frameIndex] = solid ? 1 : 0;
    }

    bool Tilemap::GetTileColliderFlag(int frameIndex) const
    {
        if (frameIndex < 0 || frameIndex >= (int)m_tileColliders.size())
            return false;
        return m_tileColliders[frameIndex] != 0;
    }

    void Tilemap::EnsureResources()
    {
        if (!s_shader)
        {
            const char *vs = R"(#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
uniform mat4 uMVP;
out vec2 vUV;
void main(){gl_Position = uMVP * vec4(aPos,0.0,1.0); vUV=aUV;} )";
            const char *fs = R"(#version 330 core
in vec2 vUV; out vec4 FragColor; uniform sampler2D uTex; uniform vec4 uTint; void main(){ FragColor = texture(uTex,vUV)*uTint; })";
            s_shader = std::make_shared<Kiaak::Shader>();
            s_shader->LoadFromString(vs, fs);
        }
        if (!s_vao)
        {
            float verts[] = {-0.5f, -0.5f, 0, 0, 0.5f, -0.5f, 1, 0, 0.5f, 0.5f, 1, 1, -0.5f, -0.5f, 0, 0, 0.5f, 0.5f, 1, 1, -0.5f, 0.5f, 0, 1};
            s_vbo = std::make_shared<Kiaak::VertexBuffer>(verts, sizeof(verts));
            s_vao = std::make_shared<Kiaak::VertexArray>();
            s_vao->Bind();
            s_vbo->Bind();
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
            s_vao->Unbind();
        }
    }

    void Tilemap::EnsureTexture()
    {
        if (!m_texturePath.empty() && !m_texture)
        {
            std::string resolved = m_texturePath;
            if (!std::filesystem::exists(resolved))
            {
                // Try project assets path
                if (Core::Project::HasPath())
                {
                    auto candidate = Core::Project::GetAssetsPath() + "/" + m_texturePath;
                    if (std::filesystem::exists(candidate))
                        resolved = candidate;
                }
                else
                {
                    std::string candidate = std::string("assets/") + m_texturePath;
                    if (std::filesystem::exists(candidate))
                        resolved = candidate;
                }
            }
            if (std::filesystem::exists(resolved))
                m_texture = std::make_shared<Texture>(resolved);
        }
        if (!m_texture && !s_shader)
        {
        } // silence
    }

    void Tilemap::Start()
    {
        EnsureResources();
        EnsureTexture();
        RebuildColliders();
    }

    void Tilemap::UpdateUV(float u0, float v0, float u1, float v1)
    {
        if (!s_vbo)
            return;
        float verts[] = {-0.5f, -0.5f, u0, v0, 0.5f, -0.5f, u1, v0, 0.5f, 0.5f, u1, v1, -0.5f, -0.5f, u0, v0, 0.5f, 0.5f, u1, v1, -0.5f, 0.5f, u0, v1};
        s_vbo->Bind();
        s_vbo->SetData(verts, sizeof(verts));
    }

    void Tilemap::Render()
    {
        EnsureResources();
        EnsureTexture();
        if (!s_shader || !s_vao || !m_texture)
            return;
        auto *tr = GetGameObject()->GetTransform();
        if (!tr)
            return;
        glm::mat4 VP(1.0f);
        if (auto *cam = Core::Camera::GetActive())
            VP = cam->GetViewProjection();
        else
        {
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, vp);
            float w = (float)vp[2];
            float h = (float)vp[3];
            VP = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, -1.0f, 1.0f);
        }
        glm::vec3 pos = tr->GetPosition();
        glm::mat4 base = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
        float texW = (float)m_texture->GetWidth();
        float texH = (float)m_texture->GetHeight();
        float frameW = texW / (float)m_hFrames;
        float frameH = texH / (float)m_vFrames;

        // Disable depth test so transparent pixels don't occlude background
        GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        if (depthEnabled)
            glDisable(GL_DEPTH_TEST);
        GLboolean depthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        if (depthMask)
            glDepthMask(GL_FALSE);
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                int idx = m_tiles[y * m_width + x];
                if (idx < 0)
                    continue;
                int fx = idx % m_hFrames;
                int fy = idx / m_hFrames;
                float u0 = (fx * frameW) / texW;
                float v0 = (fy * frameH) / texH;
                float u1 = ((fx + 1) * frameW) / texW;
                float v1 = ((fy + 1) * frameH) / texH;
                UpdateUV(u0, v0, u1, v1);
                glm::mat4 model = glm::translate(base, glm::vec3((x + 0.5f) * m_tileWidth, (y + 0.5f) * m_tileHeight, 0.0f));
                model = glm::scale(model, glm::vec3(m_tileWidth, m_tileHeight, 1.0f));
                s_shader->Use();
                s_shader->SetMat4("uMVP", VP * model);
                s_shader->SetVec4("uTint", glm::vec4(1, 1, 1, 1));
                m_texture->Bind(0);
                s_shader->SetInt("uTex", 0);
                s_vao->Bind();
                glDrawArrays(GL_TRIANGLES, 0, 6);
                s_vao->Unbind();
            }
        }
        if (depthMask)
            glDepthMask(GL_TRUE);
        if (depthEnabled)
            glEnable(GL_DEPTH_TEST);
    }

    void Tilemap::RebuildColliders()
    {
        auto *go = GetGameObject();
        if (!go)
            return;
        auto *scene = go->GetScene();
        if (!scene)
            return;
        // Remove old collider objects tracked previously
        for (auto id : m_colliderObjectIDs)
        {
            if (auto *old = scene->GetGameObject(id))
            {
                scene->RemoveGameObject(old);
            }
        }
        m_colliderObjectIDs.clear();
        // Also remove any existing children named TileCollider (in case they were serialized earlier)
        {
            // Copy list to avoid mutation during iteration
            auto children = go->GetChildren();
            for (auto *child : children)
            {
                if (!child)
                    continue;
                if (child->GetName().rfind("TileCollider", 0) == 0)
                {
                    scene->RemoveGameObject(child);
                }
            }
        }
        // Build new ones
        int frameCount = m_hFrames * m_vFrames;
        if (frameCount <= 0)
            return;
        auto *tr = go->GetTransform();
        if (!tr)
            return;
        glm::vec3 basePos = tr->GetPosition();
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                int idx = m_tiles[y * m_width + x];
                if (idx < 0 || idx >= frameCount)
                    continue;
                if (!GetTileColliderFlag(idx))
                    continue;
                auto *colGO = scene->CreateGameObject("TileCollider");
                if (!colGO)
                    continue;
                colGO->GetTransform()->SetPosition(glm::vec3(basePos.x + (x + 0.5f) * m_tileWidth, basePos.y + (y + 0.5f) * m_tileHeight, basePos.z));
                if (auto *box = colGO->AddComponent<Core::BoxCollider2D>())
                {
                    box->SetSize(glm::vec2(m_tileWidth, m_tileHeight));
                }
                colGO->SetParent(go); // parent under tilemap for hierarchy cleanliness
                m_colliderObjectIDs.push_back(colGO->GetID());
            }
        }
    }
}
