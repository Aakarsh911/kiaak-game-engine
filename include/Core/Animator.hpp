#pragma once

#include "Core/Component.hpp"

namespace Kiaak
{
    namespace Core
    {

        class Animator : public Component
        {
        public:
            Animator() = default;
            std::string GetTypeName() const override { return "Animator"; }
            void Start() override;
            void Update(double deltaTime) override;
            void SetClipIndex(int idx)
            {
                m_clipIndex = idx;
                m_currentFrameInSequence = 0;
                m_accumulator = 0.0;
                // auto-play decision deferred to Start/SetClipIndex using clip metadata
                ApplyAutoPlay();
            }
            int GetClipIndex() const { return m_clipIndex; }
            void Play() { m_playing = true; }
            void Stop() { m_playing = false; }
            bool IsPlaying() const { return m_playing; }

        private:
            int m_clipIndex = -1; // index into editor clip list
            double m_accumulator = 0.0;
            size_t m_currentFrameInSequence = 0;
            bool m_playing = false;
            void ApplyAutoPlay();
        };

    }
}
