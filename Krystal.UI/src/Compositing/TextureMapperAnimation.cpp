#include "Krystal.UI/Compositing/TextureMapperAnimation.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFilterAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include <algorithm>

namespace krys::ui
{
  namespace
  {
    KRYS_NODISCARD RefPtr<boo::FilterOperation> BlendFunc(const boo::FilterOperation *fromOp,
                                                          boo::FilterOperation &toOp, double progress,
                                                          bool blendToPassthrough = false) noexcept
    {
      return toOp.Blend(fromOp, boo::BlendingContext(progress), blendToPassthrough);
    }

    KRYS_NODISCARD boo::FilterOperations ApplyFilterAnimation(const boo::FilterOperations &from,
                                                              const boo::FilterOperations &to,
                                                              double progress) noexcept
    {
      // First frame of an animation.
      if (!progress)
      {
        return from;
      }

      // Last frame of an animation.
      if (progress == 1)
      {
        return to;
      }

      if (!from.IsEmpty() && !to.IsEmpty() && !from.OperationsMatch(to))
      {
        return to;
      }

      size_t fromSize = from.size();
      size_t toSize = to.size();
      size_t size = std::max(fromSize, toSize);

      List<Ref<boo::FilterOperation>> operations;
      operations.reserve(size);

      for (size_t i = 0; i < size; i++)
      {
        RefPtr<boo::FilterOperation> fromOp;
        if (i < fromSize)
        {
          fromOp = from[i];
        }
        RefPtr<boo::FilterOperation> toOp;
        if (i < toSize)
        {
          toOp = to[i];
        }
        RefPtr<boo::FilterOperation> blendedOp =
          toOp ? BlendFunc(fromOp.get(), *toOp, progress)
               : (fromOp ? BlendFunc(nullptr, *fromOp, progress, true) : nullptr);
        if (blendedOp)
        {
          operations.push_back(ShareRef(*blendedOp));
        }
        else if (progress > 0.5)
        {
          if (toOp)
          {
            operations.push_back(ShareRef(*toOp));
          }
          else
          {
            operations.push_back(boo::PassthroughFilterOperation::Create());
          }
        }
        else
        {
          if (fromOp)
          {
            operations.push_back(ShareRef(*fromOp));
          }
          else
          {
            operations.push_back(boo::PassthroughFilterOperation::Create());
          }
        }
      }

      return boo::FilterOperations(krys::move(operations));
    }

    KRYS_NODISCARD bool ShouldReverseAnimationValue(boo::GraphicsLayerAnimation::Direction direction,
                                                    int32 loopCount) noexcept
    {
      using Direction = boo::GraphicsLayerAnimation::Direction;
      return (direction == Direction::Alternate && loopCount & 1)
             || (direction == Direction::AlternateReverse && !(loopCount & 1))
             || direction == Direction::Reverse;
    }

    KRYS_NODISCARD double NormalizedAnimationValue(double runningTime, double duration,
                                                   boo::GraphicsLayerAnimation::Direction direction,
                                                   double iterationCount) noexcept
    {
      if (!duration)
      {
        return 0;
      }

      const auto loopCount = static_cast<int32>(runningTime / duration);
      const double lastFullLoop = duration * static_cast<double>(loopCount);
      const double remainder = runningTime - lastFullLoop;
      // Ignore remainder when we've reached the end of animation.
      const double normalized = (loopCount == iterationCount) ? 1.0 : (remainder / duration);

      return ShouldReverseAnimationValue(direction, loopCount) ? 1 - normalized : normalized;
    }

    KRYS_NODISCARD double
      NormalizedAnimationValueForFillsForwards(double iterationCount,
                                               boo::GraphicsLayerAnimation::Direction direction) noexcept
    {
      using Direction = boo::GraphicsLayerAnimation::Direction;
      if (direction == Direction::Normal)
      {
        return 1;
      }
      if (direction == Direction::Reverse)
      {
        return 0;
      }
      return ShouldReverseAnimationValue(direction, static_cast<int32>(iterationCount)) ? 1 : 0;
    }

    KRYS_NODISCARD float ApplyOpacityAnimation(float fromOpacity, float toOpacity, double progress) noexcept
    {
      // Optimization: special case the edge values (0 and 1).
      if (progress == 1.0)
      {
        return toOpacity;
      }

      if (!progress)
      {
        return fromOpacity;
      }

      return static_cast<float>(fromOpacity + progress * (toOpacity - fromOpacity));
    }

    KRYS_NODISCARD boo::TransformationMatrix ApplyTransformAnimation(const boo::TransformOperations &from,
                                                                     const boo::TransformOperations &to,
                                                                     double progress) noexcept
    {
      boo::TransformationMatrix matrix;

      // First frame of an animation.
      if (!progress)
      {
        from.Apply(matrix);
        return matrix;
      }

      // Last frame of an animation.
      if (progress == 1)
      {
        to.Apply(matrix);
        return matrix;
      }

      boo::Blend(from, to, boo::BlendingContext(progress)).Apply(matrix);
      return matrix;
    }

    KRYS_NODISCARD const boo::TimingFunction &
      TimingFunctionForAnimationValue(const boo::GraphicsLayerAnimationValue &animationValue,
                                      const TextureMapperAnimation &animation) noexcept
    {
      if (const auto *function = animationValue.GetTimingFunction())
      {
        return *function;
      }
      if (const auto *function = animation.GetTimingFunction())
      {
        return *function;
      }
      return boo::CubicBezierTimingFunction::DefaultTimingFunction();
    }
  }

  TextureMapperAnimation::TextureMapperAnimation(utf8_stringview name,
                                                 const boo::GraphicsLayerKeyframeValueList &keyframes,
                                                 const boo::GraphicsLayerAnimation &animation,
                                                 MonotonicTime::duration startTime, boo::Seconds pauseTime,
                                                 State state) noexcept
      : _name(name), _keyframes(keyframes),
        _timingFunction(animation.DefaultTimingFunctionForKeyframes()
                          ? animation.DefaultTimingFunctionForKeyframes()->Clone()
                          : animation.GetTimingFunction()->Clone()),
        _iterationCount(animation.IterationCount()), _duration(animation.Duration().value_or(0)),
        _direction(animation.GetDirection()), _fillsForwards(animation.FillsForwards()),
        _startTime(startTime), _pauseTime(pauseTime), _lastRefreshedTime(startTime), _state(state)
  {
  }

  TextureMapperAnimation::TextureMapperAnimation(const TextureMapperAnimation &other) noexcept
      : _name(other._name), _keyframes(other._keyframes), _timingFunction(other._timingFunction->Clone()),
        _iterationCount(other._iterationCount), _duration(other._duration), _direction(other._direction),
        _fillsForwards(other._fillsForwards), _startTime(other._startTime), _pauseTime(other._pauseTime),
        _totalRunningTime(other._totalRunningTime), _lastRefreshedTime(other._lastRefreshedTime),
        _state(other._state)
  {
  }

  TextureMapperAnimation &TextureMapperAnimation::operator=(const TextureMapperAnimation &other) noexcept
  {
    _name = other._name;
    _keyframes = other._keyframes;
    _timingFunction = other._timingFunction->Clone();
    _iterationCount = other._iterationCount;
    _duration = other._duration;
    _direction = other._direction;
    _fillsForwards = other._fillsForwards;
    _startTime = other._startTime;
    _pauseTime = other._pauseTime;
    _totalRunningTime = other._totalRunningTime;
    _lastRefreshedTime = other._lastRefreshedTime;
    _state = other._state;
    return *this;
  }

  void TextureMapperAnimation::Apply(ApplicationResult &applicationResults, MonotonicTime::duration time,
                                     KeepInternalState keepInternalState) noexcept
  {
    auto oldLastRefreshedTime = _lastRefreshedTime;
    auto oldTotalRunningTime = _totalRunningTime;
    auto oldState = _state;

    struct RestoreInternalState
    {
      TextureMapperAnimation &Animation;
      KeepInternalState Keep;
      MonotonicTime::duration LastRefreshedTime;
      boo::Seconds TotalRunningTime;
      State OldState;

      ~RestoreInternalState() noexcept
      {
        if (Keep == KeepInternalState::Yes)
        {
          Animation._lastRefreshedTime = LastRefreshedTime;
          Animation._totalRunningTime = TotalRunningTime;
          Animation._state = OldState;
        }
      }
    } maybeRestoreInternalState {*this, keepInternalState, oldLastRefreshedTime, oldTotalRunningTime,
                                 oldState};

    // Even when _state == State::Stopped && !_fillsForwards, we should calculate the last value to avoid a
    // flash.

    auto totalRunningTime = ComputeTotalRunningTime(time);
    double normalizedValue =
      NormalizedAnimationValue(totalRunningTime.count(), _duration, _direction, _iterationCount);

    if (_iterationCount != boo::GraphicsLayerAnimation::IterationCountInfinite
        && totalRunningTime.count() >= _duration * _iterationCount)
    {
      _state = State::Stopped;
      _pauseTime = {};
      normalizedValue = NormalizedAnimationValueForFillsForwards(_iterationCount, _direction);
    }

    applicationResults.HasRunningAnimations |= (_state == State::Playing);

    if (!normalizedValue)
    {
      ApplyInternal(applicationResults, _keyframes.At(0), _keyframes.At(1), 0);
      return;
    }

    if (normalizedValue == 1.0)
    {
      ApplyInternal(applicationResults, _keyframes.At(_keyframes.Size() - 2),
                    _keyframes.At(_keyframes.Size() - 1), 1);
      return;
    }
    if (_keyframes.Size() == 2)
    {
      const auto &timingFunction = TimingFunctionForAnimationValue(_keyframes.At(0), *this);
      normalizedValue = timingFunction.TransformProgress(normalizedValue, _duration);
      ApplyInternal(applicationResults, _keyframes.At(0), _keyframes.At(1),
                    static_cast<float>(normalizedValue));
      return;
    }

    for (size_t i = 0; i < _keyframes.Size() - 1; ++i)
    {
      const auto &from = _keyframes.At(i);
      const auto &to = _keyframes.At(i + 1);
      if (from.KeyTime() > normalizedValue || to.KeyTime() < normalizedValue)
      {
        continue;
      }

      normalizedValue = (normalizedValue - from.KeyTime()) / (to.KeyTime() - from.KeyTime());
      const auto &timingFunction = TimingFunctionForAnimationValue(from, *this);
      normalizedValue = timingFunction.TransformProgress(normalizedValue, _duration);
      ApplyInternal(applicationResults, from, to, static_cast<float>(normalizedValue));
      break;
    }
  }

  void TextureMapperAnimation::Pause(boo::Seconds time) noexcept
  {
    _state = State::Paused;
    _pauseTime = time;
  }

  void TextureMapperAnimation::Resume(MonotonicTime::duration now) noexcept
  {
    _state = State::Playing;
    // FIXME: This seems wrong. _totalRunningTime is cleared.
    _pauseTime = {};
    _totalRunningTime = _pauseTime;
    _lastRefreshedTime = now;
  }

  boo::Seconds TextureMapperAnimation::ComputeTotalRunningTime(MonotonicTime::duration time) noexcept
  {
    if (_state == State::Paused)
    {
      return _pauseTime;
    }

    auto oldLastRefreshedTime = _lastRefreshedTime;
    _lastRefreshedTime = time;
    _totalRunningTime += _lastRefreshedTime - oldLastRefreshedTime;
    return _totalRunningTime;
  }

  void TextureMapperAnimation::ApplyInternal(ApplicationResult &applicationResults,
                                             const boo::GraphicsLayerAnimationValue &from,
                                             const boo::GraphicsLayerAnimationValue &to,
                                             float progress) noexcept
  {
    switch (_keyframes.Property())
    {
      case boo::AnimatedProperty::Translate:
      case boo::AnimatedProperty::Rotate:
      case boo::AnimatedProperty::Scale:
      case boo::AnimatedProperty::Transform:
      {
        krys_debug_assert(applicationResults.Transform);
        auto transform = ApplyTransformAnimation(
          static_cast<const boo::GraphicsLayerTransformAnimationValue &>(from).Value(),
          static_cast<const boo::GraphicsLayerTransformAnimationValue &>(to).Value(), progress);
        applicationResults.Transform->Multiply(transform);
        return;
      }
      case boo::AnimatedProperty::Opacity:
        applicationResults.Opacity = ApplyOpacityAnimation(
          static_cast<const boo::GraphicsLayerFloatAnimationValue &>(from).Value(),
          static_cast<const boo::GraphicsLayerFloatAnimationValue &>(to).Value(), progress);
        return;
      case boo::AnimatedProperty::Filter:
      case boo::AnimatedProperty::WebkitBackdropFilter:
        applicationResults.Filters = ApplyFilterAnimation(
          static_cast<const boo::GraphicsLayerFilterAnimationValue &>(from).Value(),
          static_cast<const boo::GraphicsLayerFilterAnimationValue &>(to).Value(), progress);
        return;
      default: krys_debug_assert(false);
    }
  }

  void TextureMapperAnimations::Add(const TextureMapperAnimation &animation) noexcept
  {
    // Remove the old state if we are resuming a paused animation.
    Remove(animation.Name(), animation.Keyframes().Property());

    _animations.push_back(animation);
  }

  void TextureMapperAnimations::Remove(utf8_stringview name) noexcept
  {
    std::erase_if(_animations, [&](const auto &animation) { return animation.Name() == name; });
  }

  void TextureMapperAnimations::Remove(utf8_stringview name, boo::AnimatedProperty property) noexcept
  {
    std::erase_if(_animations, [&](const auto &animation)
                  { return animation.Name() == name && animation.Keyframes().Property() == property; });
  }

  void TextureMapperAnimations::Pause(utf8_stringview name, boo::Seconds offset) noexcept
  {
    for (auto &animation : _animations)
    {
      if (animation.Name() == name)
      {
        animation.Pause(offset);
      }
    }
  }

  void TextureMapperAnimations::Suspend(MonotonicTime::duration time) noexcept
  {
    // FIXME: This seems wrong. `Pause` takes a time offset, not a time.
    for (auto &animation : _animations)
    {
      animation.Pause(time);
    }
  }

  void TextureMapperAnimations::Resume(MonotonicTime::duration now) noexcept
  {
    for (auto &animation : _animations)
    {
      animation.Resume(now);
    }
  }

  void TextureMapperAnimations::Apply(TextureMapperAnimation::ApplicationResult &applicationResults,
                                      MonotonicTime::duration time,
                                      TextureMapperAnimation::KeepInternalState keepInternalState) noexcept
  {
    List<TextureMapperAnimation *> translateAnimations;
    List<TextureMapperAnimation *> rotateAnimations;
    List<TextureMapperAnimation *> scaleAnimations;
    List<TextureMapperAnimation *> transformAnimations;
    List<TextureMapperAnimation *> leafAnimations;

    for (auto &animation : _animations)
    {
      switch (animation.Keyframes().Property())
      {
        case boo::AnimatedProperty::Translate: translateAnimations.push_back(&animation); break;
        case boo::AnimatedProperty::Rotate:    rotateAnimations.push_back(&animation); break;
        case boo::AnimatedProperty::Scale:     scaleAnimations.push_back(&animation); break;
        case boo::AnimatedProperty::Transform: transformAnimations.push_back(&animation); break;
        default:                               leafAnimations.push_back(&animation);
      }
    }

    if (!translateAnimations.empty() || !rotateAnimations.empty() || !scaleAnimations.empty()
        || !transformAnimations.empty())
    {
      applicationResults.Transform = boo::TransformationMatrix();

      if (translateAnimations.empty())
      {
        applicationResults.Transform->Multiply(_translate);
      }
      else
      {
        for (auto *animation : translateAnimations)
        {
          animation->Apply(applicationResults, time, keepInternalState);
        }
      }

      if (rotateAnimations.empty())
      {
        applicationResults.Transform->Multiply(_rotate);
      }
      else
      {
        for (auto *animation : rotateAnimations)
        {
          animation->Apply(applicationResults, time, keepInternalState);
        }
      }

      if (scaleAnimations.empty())
      {
        applicationResults.Transform->Multiply(_scale);
      }
      else
      {
        for (auto *animation : scaleAnimations)
        {
          animation->Apply(applicationResults, time, keepInternalState);
        }
      }

      if (transformAnimations.empty())
      {
        applicationResults.Transform->Multiply(_transform);
      }
      else
      {
        transformAnimations.back()->Apply(applicationResults, time, keepInternalState);
      }
    }

    for (auto *animation : leafAnimations)
    {
      animation->Apply(applicationResults, time, keepInternalState);
    }
  }

  bool TextureMapperAnimations::HasActiveAnimationsOfType(boo::AnimatedProperty type) const noexcept
  {
    return std::ranges::any_of(_animations, [&](const auto &animation)
                               { return animation.Keyframes().Property() == type; });
  }

  bool TextureMapperAnimations::HasRunningAnimations() const noexcept
  {
    return std::ranges::any_of(_animations, [](const auto &animation)
                               { return animation.GetState() == TextureMapperAnimation::State::Playing; });
  }

  bool TextureMapperAnimations::HasRunningTransformAnimations() const noexcept
  {
    return std::ranges::any_of(_animations,
                               [](const auto &animation)
                               {
                                 switch (animation.Keyframes().Property())
                                 {
                                   case boo::AnimatedProperty::Translate:
                                   case boo::AnimatedProperty::Rotate:
                                   case boo::AnimatedProperty::Scale:
                                   case boo::AnimatedProperty::Transform: break;
                                   default:                               return false;
                                 }

                                 switch (animation.GetState())
                                 {
                                   case TextureMapperAnimation::State::Playing:
                                   case TextureMapperAnimation::State::Paused:  break;
                                   default:                                     return false;
                                 }

                                 return true;
                               });
  }
}
