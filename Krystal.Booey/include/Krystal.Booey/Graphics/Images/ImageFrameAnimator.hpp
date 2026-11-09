/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class BitmapImageSource;

  /// @brief Advances the frames of an animated image on a timer, in the host's time, for as many loops as
  /// the image repeats.
  class ImageFrameAnimator
  {
    KRYS_NON_COPY_MOVABLE(ImageFrameAnimator);

  private:
    BitmapImageSource &_source;
    uint32 _frameCount {0};
    RepetitionCount _repetitionCount {RepetitionCountNone};

    UniquePtr<Timer> _frameTimer;
    SubsamplingLevel _nextFrameSubsamplingLevel {SubsamplingLevel::Default};
    DecodingOptions _nextFrameOptions {DecodingMode::Asynchronous};

    uint32 _currentFrameIndex {0};
    RepetitionCount _repetitionsComplete {RepetitionCountNone};
    // When the current frame should have started showing, in the host's time; null until the animation first
    // starts.
    Maybe<MonotonicTime::duration> _desiredFrameStartTime;

  public:
    explicit ImageFrameAnimator(BitmapImageSource &source) noexcept;
    ~ImageFrameAnimator() noexcept;

    bool ImageFrameDecodeAtIndexHasFinished(uint32 index, ImageAnimatingState, DecodingStatus) noexcept;

    bool StartAnimation(SubsamplingLevel, const DecodingOptions &) noexcept;
    void AdvanceAnimation() noexcept;
    void StopAnimation() noexcept;
    void ResetAnimation() noexcept;
    KRYS_NODISCARD bool IsAnimating() const noexcept
    {
      return static_cast<bool>(_frameTimer);
    }
    KRYS_NODISCARD bool IsAnimationAllowed() const noexcept;

    KRYS_NODISCARD bool HasEverAnimated() const noexcept
    {
      return _desiredFrameStartTime.has_value();
    }
    KRYS_NODISCARD uint32 CurrentFrameIndex() const noexcept
    {
      return _currentFrameIndex;
    }

    void Dump(TextStream &) const noexcept;

  private:
    void DestroyDecodedData(bool destroyAll) noexcept;

    void StartTimer(MonotonicTime::duration delay) noexcept;
    void ClearTimer() noexcept;
    void TimerFired() noexcept;

    KRYS_NODISCARD uint32 NextFrameIndex() const noexcept
    {
      return (_currentFrameIndex + 1) % _frameCount;
    }
  };
}
