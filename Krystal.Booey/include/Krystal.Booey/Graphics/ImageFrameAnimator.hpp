#pragma once

#include "Krystal.Booey/Graphics/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/ImageTypes.hpp"
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
