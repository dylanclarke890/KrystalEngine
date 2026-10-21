#pragma once

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief The timing of an animation the compositor runs on a graphics layer.
  class GraphicsLayerAnimation : public RefCounted<GraphicsLayerAnimation>
  {
  public:
    static constexpr double IterationCountInfinite = -1;

    enum class Direction : uint8
    {
      Normal,
      Alternate,
      Reverse,
      AlternateReverse
    };

    enum class FillMode : uint8
    {
      None,
      Forwards,
      Backwards,
      Both
    };

    enum class PlayState : bool
    {
      Running,
      Paused
    };

  private:
    double _delay {0};
    Maybe<double> _duration;
    double _iterationCount {1.0};
    double _playbackRate {1};
    RefPtr<TimingFunction> _timingFunction;
    RefPtr<TimingFunction> _defaultTimingFunctionForKeyframes;
    CompositeOperation _compositeOperation {CompositeOperation::Replace};
    Direction _direction {Direction::Normal};
    FillMode _fillMode {FillMode::None};
    PlayState _playState {PlayState::Running};

  public:
    KRYS_NODISCARD static Ref<GraphicsLayerAnimation> Create() noexcept
    {
      return AdoptRef(*new GraphicsLayerAnimation);
    }
    KRYS_NODISCARD static Ref<GraphicsLayerAnimation> Create(const GraphicsLayerAnimation &other) noexcept
    {
      return AdoptRef(*new GraphicsLayerAnimation(other));
    }

    ~GraphicsLayerAnimation() noexcept;

    KRYS_NODISCARD CompositeOperation GetCompositeOperation() const noexcept
    {
      return _compositeOperation;
    }
    void SetCompositeOperation(CompositeOperation compositeOperation) noexcept
    {
      _compositeOperation = compositeOperation;
    }

    KRYS_NODISCARD double Delay() const noexcept
    {
      return _delay;
    }
    void SetDelay(double delay) noexcept
    {
      _delay = delay;
    }

    KRYS_NODISCARD Direction GetDirection() const noexcept
    {
      return _direction;
    }
    void SetDirection(Direction direction) noexcept
    {
      _direction = direction;
    }

    void SetDuration(Maybe<double> duration) noexcept
    {
      krys_debug_assert(!duration || *duration >= 0);
      _duration = duration;
    }
    KRYS_NODISCARD Maybe<double> Duration() const noexcept
    {
      return _duration;
    }

    KRYS_NODISCARD FillMode GetFillMode() const noexcept
    {
      return _fillMode;
    }
    void SetFillMode(FillMode fillMode) noexcept
    {
      _fillMode = fillMode;
    }

    KRYS_NODISCARD double PlaybackRate() const noexcept
    {
      return _playbackRate;
    }
    void SetPlaybackRate(double playbackRate) noexcept
    {
      _playbackRate = playbackRate;
    }

    KRYS_NODISCARD double IterationCount() const noexcept
    {
      return _iterationCount;
    }
    void SetIterationCount(double iterationCount) noexcept
    {
      _iterationCount = iterationCount;
    }

    KRYS_NODISCARD PlayState GetPlayState() const noexcept
    {
      return _playState;
    }
    void SetPlayState(PlayState playState) noexcept
    {
      _playState = playState;
    }

    KRYS_NODISCARD RefPtr<TimingFunction> GetTimingFunction() const noexcept
    {
      return _timingFunction;
    }
    void SetTimingFunction(RefPtr<TimingFunction> &&function) noexcept
    {
      _timingFunction = krys::move(function);
    }

    KRYS_NODISCARD RefPtr<TimingFunction> DefaultTimingFunctionForKeyframes() const noexcept
    {
      return _defaultTimingFunctionForKeyframes;
    }
    void SetDefaultTimingFunctionForKeyframes(RefPtr<TimingFunction> &&function) noexcept
    {
      _defaultTimingFunctionForKeyframes = krys::move(function);
    }

    KRYS_NODISCARD bool IsZeroDuration() const noexcept
    {
      return (!_duration || !*_duration) && _delay <= 0;
    }
    KRYS_NODISCARD bool FillsBackwards() const noexcept
    {
      return _fillMode == FillMode::Backwards || _fillMode == FillMode::Both;
    }
    KRYS_NODISCARD bool FillsForwards() const noexcept
    {
      return _fillMode == FillMode::Forwards || _fillMode == FillMode::Both;
    }
    KRYS_NODISCARD bool DirectionIsForwards() const noexcept
    {
      return _direction == Direction::Normal || _direction == Direction::Alternate;
    }

    KRYS_NODISCARD bool operator==(const GraphicsLayerAnimation &) const noexcept;

  private:
    GraphicsLayerAnimation() noexcept;
    GraphicsLayerAnimation(const GraphicsLayerAnimation &) noexcept;
  };

  TextStream &operator<<(TextStream &ts, const GraphicsLayerAnimation &animation) noexcept;
  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::Direction direction) noexcept;
  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::FillMode fillMode) noexcept;
  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::PlayState playState) noexcept;
}
