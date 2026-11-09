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

#include "Krystal.Booey/Graphics/Images/ImageFrameAnimator.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImageSource.hpp"
#include "Krystal.Booey/Platform/ThreadTimers.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    // The host's time: the time timers fire by.
    KRYS_NODISCARD MonotonicTime::duration CurrentTime() noexcept
    {
      return ThreadTimers::Current().CurrentTime();
    }
  }

  ImageFrameAnimator::ImageFrameAnimator(BitmapImageSource &source) noexcept
      : _source(source), _frameCount(source.FrameCount()), _repetitionCount(source.GetRepetitionCount()),
        _currentFrameIndex(source.PrimaryFrameIndex())
  {
    krys_debug_assert(_frameCount > 0);
    krys_debug_assert(_repetitionCount != RepetitionCountNone);
    krys_debug_assert(_currentFrameIndex < _frameCount);
  }

  ImageFrameAnimator::~ImageFrameAnimator() noexcept
  {
    ClearTimer();
  }

  void ImageFrameAnimator::DestroyDecodedData(bool destroyAll) noexcept
  {
    // Animated images over a certain size are considered large enough that we'll
    // only hang on to one frame at a time.
    static constexpr uint32 LargeAnimationCutoff = 30 * 1'024 * 1'024;

    if (_source.DecodedSize() < LargeAnimationCutoff)
    {
      return;
    }

    _source.DestroyDecodedData(destroyAll);
  }

  void ImageFrameAnimator::StartTimer(MonotonicTime::duration delay) noexcept
  {
    krys_debug_assert(!_frameTimer);
    _frameTimer = CreateUnique<Timer>(*this, &ImageFrameAnimator::TimerFired);
    _frameTimer->StartOneShot(delay);
  }

  void ImageFrameAnimator::ClearTimer() noexcept
  {
    _frameTimer = nullptr;
  }

  void ImageFrameAnimator::TimerFired() noexcept
  {
    ClearTimer();

    // Don't advance to nextFrame if the next frame is being decoded.
    if (_source.IsPendingDecodingAtIndex(NextFrameIndex(), _nextFrameSubsamplingLevel, _nextFrameOptions))
    {
      return;
    }

    AdvanceAnimation();
    _source.ImageFrameAtIndexAvailable(_currentFrameIndex, ImageAnimatingState::Yes,
                                       _source.FrameDecodingStatusAtIndex(_currentFrameIndex));
  }

  bool ImageFrameAnimator::ImageFrameDecodeAtIndexHasFinished(uint32 index,
                                                              ImageAnimatingState animatingState,
                                                              DecodingStatus decodingStatus) noexcept
  {
    if (animatingState != ImageAnimatingState::Yes)
    {
      return false;
    }

    if (index != NextFrameIndex())
    {
      return false;
    }

    if (!HasEverAnimated())
    {
      return false;
    }

    // Don't advance to nextFrame if the timer has not fired yet.
    if (IsAnimating())
    {
      return true;
    }

    AdvanceAnimation();
    _source.ImageFrameAtIndexAvailable(_currentFrameIndex, animatingState, decodingStatus);
    return true;
  }

  bool ImageFrameAnimator::StartAnimation(SubsamplingLevel subsamplingLevel,
                                          const DecodingOptions &options) noexcept
  {
    if (_frameTimer)
    {
      return true;
    }

    // ImageObserver may disallow animation.
    if (!_source.IsAnimationAllowed())
    {
      return false;
    }

    _nextFrameSubsamplingLevel = subsamplingLevel;
    _nextFrameOptions = options;

    // Without the host's work queue, the next frame decodes when it's drawn.
    if (options.GetDecodingMode() == DecodingMode::Asynchronous && _source.CanDecodeAsynchronously())
    {
      (void)_source.RequestNativeImageAtIndexIfNeeded(NextFrameIndex(), subsamplingLevel,
                                                      ImageAnimatingState::Yes, options);
    }

    auto time = CurrentTime();

    // Handle initial state.
    if (!_desiredFrameStartTime)
    {
      _desiredFrameStartTime = time;
    }

    auto duration =
      std::chrono::duration_cast<MonotonicTime::duration>(_source.FrameDurationAtIndex(_currentFrameIndex));

    // Setting _desiredFrameStartTime to time means we are late; otherwise we are early.
    _desiredFrameStartTime = std::max(time, *_desiredFrameStartTime + duration);

    StartTimer(*_desiredFrameStartTime - time);
    return true;
  }

  void ImageFrameAnimator::AdvanceAnimation() noexcept
  {
    _currentFrameIndex = NextFrameIndex();
    if (_currentFrameIndex == _frameCount - 1)
    {
      ++_repetitionsComplete;
    }

    DestroyDecodedData(false);
  }

  void ImageFrameAnimator::StopAnimation() noexcept
  {
    ClearTimer();
  }

  void ImageFrameAnimator::ResetAnimation() noexcept
  {
    StopAnimation();

    _currentFrameIndex = _source.PrimaryFrameIndex();
    _repetitionsComplete = RepetitionCountNone;
    _desiredFrameStartTime = null;

    // For extremely large animations, when the animation is reset, we just throw everything away.
    DestroyDecodedData(true);
  }

  bool ImageFrameAnimator::IsAnimationAllowed() const noexcept
  {
    return _repetitionCount == RepetitionCountInfinite || _repetitionsComplete < _repetitionCount;
  }

  void ImageFrameAnimator::Dump(TextStream &ts) const noexcept
  {
    ts.DumpProperty(u8"current-frame-index", _currentFrameIndex);
    ts.DumpProperty(u8"repetitions-complete", _repetitionsComplete);
  }
}
