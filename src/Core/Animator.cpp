#include "Core/Animator.hpp"
#include "Editor/EditorUI.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/GameObject.hpp"
#include <algorithm>

using namespace Kiaak;
using namespace Core;

void Animator::Start()
{
    ApplyAutoPlay();
}

void Animator::Update(double deltaTime)
{
    if (!m_playing || m_clipIndex < 0)
        return;
    const auto &clips = EditorUI::GetAnimationClips();
    if (m_clipIndex >= (int)clips.size())
        return;
    const auto &clip = clips[m_clipIndex];
    if (clip.sequence.empty() || clip.hFrames <= 0 || clip.vFrames <= 0)
        return;
    float fps = clip.fps > 0.f ? clip.fps : 1.f;
    m_accumulator += deltaTime;
    double frameTime = 1.0 / fps;
    while (m_accumulator >= frameTime)
    {
        m_accumulator -= frameTime;
        m_currentFrameInSequence = (m_currentFrameInSequence + 1) % clip.sequence.size();
    }
    int logicalIndex = clip.sequence[m_currentFrameInSequence];
    int cols = clip.hFrames;
    int r = logicalIndex / cols; // bottom = 0 (as stored)
    int c = logicalIndex % cols;
    // Convert to UVs (remember we inverted for display: row 0 bottom => V range)
    float u0 = (float)c / (float)cols;
    float u1 = (float)(c + 1) / (float)cols;
    float v0 = (float)r / (float)clip.vFrames;
    float v1 = (float)(r + 1) / (float)clip.vFrames;
    // Because texture origin OpenGL is bottom-left already, this mapping should be fine
    auto *sr = GetGameObject()->GetComponent<Graphics::SpriteRenderer>();
    if (sr)
    {
        sr->SetUVRect(glm::vec4(u0, v0, u1, v1));
        if (!clip.texturePath.empty() && (!sr->GetTexture() || sr->GetTexturePath() != clip.texturePath))
        {
            sr->SetTexture(clip.texturePath);
        }
    }
}

// Ensure first frame applied on SetClipIndex (called via ApplyAutoPlay path too)
void Animator::ApplyAutoPlay()
{
    if (m_clipIndex < 0)
        return;
    const auto &clips = EditorUI::GetAnimationClips();
    if (m_clipIndex >= (int)clips.size())
        return;
    const auto &clip = clips[m_clipIndex];
    if (clip.autoPlay)
        m_playing = true;
    // Apply initial frame immediately so scene persistence shows correct sub-rect after reload
    if (!clip.sequence.empty())
    {
        int logicalIndex = clip.sequence[0];
        int cols = clip.hFrames > 0 ? clip.hFrames : 1;
        int r = logicalIndex / cols;
        int c = logicalIndex % cols;
        float u0 = (float)c / (float)cols;
        float u1 = (float)(c + 1) / (float)cols;
        float v0 = (float)r / (float)(clip.vFrames > 0 ? clip.vFrames : 1);
        float v1 = (float)(r + 1) / (float)(clip.vFrames > 0 ? clip.vFrames : 1);
        if (auto *sr = GetGameObject()->GetComponent<Graphics::SpriteRenderer>())
        {
            sr->SetUVRect(glm::vec4(u0, v0, u1, v1));
            if (!clip.texturePath.empty() && (!sr->GetTexture() || sr->GetTexturePath() != clip.texturePath))
                sr->SetTexture(clip.texturePath);
        }
    }
}
