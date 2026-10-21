#include "Krystal.Booey/Graphics/GraphicsLayerAnimation.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  GraphicsLayerAnimation::~GraphicsLayerAnimation() noexcept = default;

  GraphicsLayerAnimation::GraphicsLayerAnimation() noexcept
      : _timingFunction(CubicBezierTimingFunction::Create())
  {
  }

  GraphicsLayerAnimation::GraphicsLayerAnimation(const GraphicsLayerAnimation &o) noexcept
      : RefCounted<GraphicsLayerAnimation>(), _delay(o._delay), _duration(o._duration),
        _iterationCount(o._iterationCount), _playbackRate(o._playbackRate),
        _timingFunction(o._timingFunction),
        _defaultTimingFunctionForKeyframes(o._defaultTimingFunctionForKeyframes),
        _compositeOperation(o._compositeOperation), _direction(o._direction), _fillMode(o._fillMode),
        _playState(o._playState)
  {
  }

  bool GraphicsLayerAnimation::operator==(const GraphicsLayerAnimation &other) const noexcept
  {
    return _compositeOperation == other._compositeOperation && _delay == other._delay
           && _direction == other._direction && _duration == other._duration && _fillMode == other._fillMode
           && _iterationCount == other._iterationCount && _playState == other._playState
           && ArePointingToEqualData(_timingFunction.get(), other._timingFunction.get());
  }

  TextStream &operator<<(TextStream &ts, const GraphicsLayerAnimation &animation) noexcept
  {
    ts.DumpProperty(u8"delay", animation.Delay());
    ts.DumpProperty(u8"direction", animation.GetDirection());
    ts.DumpProperty(u8"duration", animation.Duration());
    ts.DumpProperty(u8"fill-mode", animation.GetFillMode());
    ts.DumpProperty(u8"iteration count", animation.IterationCount());
    ts.DumpProperty(u8"play-state", animation.GetPlayState());
    if (animation.GetTimingFunction())
    {
      ts.DumpProperty(u8"timing function", *animation.GetTimingFunction());
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::Direction direction) noexcept
  {
    switch (direction)
    {
      case GraphicsLayerAnimation::Direction::Normal:           ts << "normal"; break;
      case GraphicsLayerAnimation::Direction::Alternate:        ts << "alternate"; break;
      case GraphicsLayerAnimation::Direction::Reverse:          ts << "reverse"; break;
      case GraphicsLayerAnimation::Direction::AlternateReverse: ts << "alternate-reverse"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::FillMode fillMode) noexcept
  {
    switch (fillMode)
    {
      case GraphicsLayerAnimation::FillMode::None:      ts << "none"; break;
      case GraphicsLayerAnimation::FillMode::Forwards:  ts << "forwards"; break;
      case GraphicsLayerAnimation::FillMode::Backwards: ts << "backwards"; break;
      case GraphicsLayerAnimation::FillMode::Both:      ts << "both"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, GraphicsLayerAnimation::PlayState playState) noexcept
  {
    switch (playState)
    {
      case GraphicsLayerAnimation::PlayState::Running: ts << "running"; break;
      case GraphicsLayerAnimation::PlayState::Paused:  ts << "paused"; break;
    }
    return ts;
  }
}
