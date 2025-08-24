#pragma once
#include "Core/Component.hpp"
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Graphics/Texture.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/VertexBuffer.hpp"

namespace Kiaak::Core
{
    class Tilemap : public Component
    {
    public:
        Tilemap();
        std::string GetTypeName() const override { return "Tilemap"; }
        void Start() override;
        void Render();
        void RebuildColliders(); // create per-tile collider GameObjects for frames flagged solid
        void SetMapSize(int w, int h);
        void SetTileSize(float w, float h);
        void SetTileset(const std::string &path, int hFrames, int vFrames);
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        float GetTileWidth() const { return m_tileWidth; }
        float GetTileHeight() const { return m_tileHeight; }
        const std::string &GetTexturePath() const { return m_texturePath; }
        int GetHFrames() const { return m_hFrames; }
        int GetVFrames() const { return m_vFrames; }
        const std::vector<int> &GetTiles() const { return m_tiles; }
        std::vector<int> &GetTiles() { return m_tiles; }
        const std::vector<uint8_t> &GetColliderFlags() const { return m_tileColliders; }
        std::vector<uint8_t> &GetColliderFlags() { return m_tileColliders; }
        void SetTile(int x, int y, int index);
        int GetTile(int x, int y) const;
        void SetTileColliderFlag(int frameIndex, bool solid);
        bool GetTileColliderFlag(int frameIndex) const;

    private:
        int m_width;
        int m_height;
        float m_tileWidth;
        float m_tileHeight;
        std::string m_texturePath;
        int m_hFrames;
        int m_vFrames;
        std::vector<int> m_tiles;
        std::vector<uint8_t> m_tileColliders;
        std::vector<uint32_t> m_colliderObjectIDs; // spawned collider GO ids
        std::shared_ptr<Texture> m_texture;
        static std::shared_ptr<Kiaak::Shader> s_shader;
        static std::shared_ptr<Kiaak::VertexArray> s_vao;
        static std::shared_ptr<Kiaak::VertexBuffer> s_vbo;
        static int s_instances;
        void EnsureResources();
        void EnsureTexture();
        void UpdateUV(float u0, float v0, float u1, float v1);
    };
}
