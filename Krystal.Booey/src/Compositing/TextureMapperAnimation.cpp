/*
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Compositing/TextureMapperAnimation.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Web/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerAnimationValue.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerFilterAnimationValue.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Web/Graphics/Transforms/TransformOperations.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD RefPtr<web::FilterOperation> BlendFunc(const web::FilterOperation *fromOp,
                                                          web::FilterOperation &toOp, double progress,
                                                          bool blendToPassthrough = false) noexcept
    {
      return toOp.Blend(fromOp, web::BlendingContext(progress), blendToPassthrough);
    }

    KRYS_NODISCARD web::FilterOperations ApplyFilterAnimation(const web::FilterOperations &from,
                                                              const web::FilterOperations &to,
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

      List<Ref<web::FilterOperation>> operations;
      operations.reserve(size);

      for (size_t i = 0; i < size; i++)
      {
        RefPtr<web::FilterOperation> fromOp;
        if (i < fromSize)
        {
          fromOp = from[i];
        }
        RefPtr<web::FilterOperation> toOp;
        if (i < toSize)
        {
          toOp = to[i];
        }
        RefPtr<web::FilterOperation> blendedOp =
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
            operations.emplace_back(web::PassthroughFilterOperation::Create());
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
            operations.emplace_back(web::PassthroughFilterOperation::Create());
          }
        }
      }

      return web::FilterOperations(krys::move(operations));
    }

    KRYS_NODISCARD bool ShouldReverseAnimationValue(web::GraphicsLayerAnimation::Direction direction,
                                                    int32 loopCount) noexcept
    {
      using Direction = web::GraphicsLayerAnimation::Direction;
      return (direction == Direction::Alternate && loopCount & 1)
             || (direction == Direction::AlternateReverse && !(loopCount & 1))
             || direction == Direction::Reverse;
    }

    KRYS_NODISCARD double NormalizedAnimationValue(double runningTime, double duration,
                                                   web::GraphicsLayerAnimation::Direction direction,
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
                                               web::GraphicsLayerAnimation::Direction direction) noexcept
    {
      using Direction = web::GraphicsLayerAnimation::Direction;
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

    KRYS_NODISCARD TransformationMatrix ApplyTransformAnimation(const web::TransformOperations &from,
                                                                const web::TransformOperations &to,
                                                                double progress) noexcept
    {
      TransformationMatrix matrix;

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

      web::Blend(from, to, web::BlendingContext(progress)).Apply(matrix);
      return matrix;
    }

    KRYS_NODISCARD const web::TimingFunction &
      TimingFunctionForAnimationValue(const web::GraphicsLayerAnimationValue &animationValue,
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
      return web::CubicBezierTimingFunction::DefaultTimingFunction();
    }
  }

  TextureMapperAnimation::TextureMapperAnimation(utf8_stringview name,
                                                 // NOLINTNEXTLINE(modernize-pass-by-value): explicit copy.
                                                 const web::GraphicsLayerKeyframeValueList &keyframes,
                                                 const web::GraphicsLayerAnimation &animation,
                                                 MonotonicTime::duration startTime, web::Seconds pauseTime,
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
        _state(other._state), _suspended(other._suspended)
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
    _suspended = other._suspended;
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
      web::Seconds TotalRunningTime;
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

    if (_iterationCount != web::GraphicsLayerAnimation::IterationCountInfinite
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

  void TextureMapperAnimation::Pause(web::Seconds offset) noexcept
  {
    _state = State::Paused;
    _pauseTime = offset;
    _suspended = false;
  }

  void TextureMapperAnimation::Suspend(MonotonicTime::duration time) noexcept
  {
    if (_state != State::Playing)
    {
      return;
    }
    Pause(ComputeTotalRunningTime(time));
    _suspended = true;
  }

  void TextureMapperAnimation::Resume(MonotonicTime::duration now) noexcept
  {
    if (!_suspended)
    {
      return;
    }
    // It goes on from the running time it was held at.
    _state = State::Playing;
    _totalRunningTime = _pauseTime;
    _pauseTime = {};
    _lastRefreshedTime = now;
    _suspended = false;
  }

  web::Seconds TextureMapperAnimation::ComputeTotalRunningTime(MonotonicTime::duration time) noexcept
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
                                             const web::GraphicsLayerAnimationValue &from,
                                             const web::GraphicsLayerAnimationValue &to,
                                             float progress) noexcept
  {
    switch (_keyframes.Property())
    {
      case web::AnimatedProperty::Translate:
      case web::AnimatedProperty::Rotate:
      case web::AnimatedProperty::Scale:
      case web::AnimatedProperty::Transform:
      {
        krys_debug_assert(applicationResults.Transform);
        auto transform = ApplyTransformAnimation(
          static_cast<const web::GraphicsLayerTransformAnimationValue &>(from).Value(),
          static_cast<const web::GraphicsLayerTransformAnimationValue &>(to).Value(), progress);
        applicationResults.Transform->Multiply(transform);
        return;
      }
      case web::AnimatedProperty::Opacity:
        applicationResults.Opacity = ApplyOpacityAnimation(
          static_cast<const web::GraphicsLayerFloatAnimationValue &>(from).Value(),
          static_cast<const web::GraphicsLayerFloatAnimationValue &>(to).Value(), progress);
        return;
      case web::AnimatedProperty::Filter:
      case web::AnimatedProperty::WebkitBackdropFilter:
        applicationResults.Filters = ApplyFilterAnimation(
          static_cast<const web::GraphicsLayerFilterAnimationValue &>(from).Value(),
          static_cast<const web::GraphicsLayerFilterAnimationValue &>(to).Value(), progress);
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

  void TextureMapperAnimations::Remove(utf8_stringview name, web::AnimatedProperty property) noexcept
  {
    std::erase_if(_animations, [&](const auto &animation)
                  { return animation.Name() == name && animation.Keyframes().Property() == property; });
  }

  void TextureMapperAnimations::Pause(utf8_stringview name, web::Seconds offset) noexcept
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
    for (auto &animation : _animations)
    {
      animation.Suspend(time);
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
        case web::AnimatedProperty::Translate: translateAnimations.push_back(&animation); break;
        case web::AnimatedProperty::Rotate:    rotateAnimations.push_back(&animation); break;
        case web::AnimatedProperty::Scale:     scaleAnimations.push_back(&animation); break;
        case web::AnimatedProperty::Transform: transformAnimations.push_back(&animation); break;
        default:                               leafAnimations.push_back(&animation);
      }
    }

    if (!translateAnimations.empty() || !rotateAnimations.empty() || !scaleAnimations.empty()
        || !transformAnimations.empty())
    {
      applicationResults.Transform = TransformationMatrix();

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

  bool TextureMapperAnimations::HasActiveAnimationsOfType(web::AnimatedProperty type) const noexcept
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
                                   case web::AnimatedProperty::Translate:
                                   case web::AnimatedProperty::Rotate:
                                   case web::AnimatedProperty::Scale:
                                   case web::AnimatedProperty::Transform: break;
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
