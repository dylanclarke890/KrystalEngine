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

#pragma once

#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Web/Animation/Timing/WebAnimationTime.hpp"
#include "Krystal.Web/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Web/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerAnimation.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerKeyframeValueList.hpp"

namespace krys::web
{
  class GraphicsLayerAnimationValue;
}

namespace krys::boo
{
  /// @brief A layer's animation of one property, sampled at the times the layer is composited.
  class TextureMapperAnimation
  {
  public:
    enum class State : uint8
    {
      Playing,
      Paused,
      Stopped,
    };

    /// @brief The values a layer's animations produce at a time.
    struct ApplicationResult
    {
      Maybe<TransformationMatrix> Transform;
      Maybe<double> Opacity;
      Maybe<web::FilterOperations> Filters;
      bool HasRunningAnimations {false};
    };

    enum class KeepInternalState : bool
    {
      No,
      Yes,
    };

  private:
    utf8_string _name;
    web::GraphicsLayerKeyframeValueList _keyframes {web::AnimatedProperty::Invalid};
    RefPtr<web::TimingFunction> _timingFunction;
    double _iterationCount {0};
    double _duration {0};
    web::GraphicsLayerAnimation::Direction _direction {web::GraphicsLayerAnimation::Direction::Normal};
    bool _fillsForwards {false};
    MonotonicTime::duration _startTime {};
    web::Seconds _pauseTime {};
    web::Seconds _totalRunningTime {};
    MonotonicTime::duration _lastRefreshedTime {};
    State _state {State::Stopped};
    /// @brief Whether it's paused by a suspension, rather than by its animation.
    bool _suspended {false};

  public:
    TextureMapperAnimation() noexcept = default;

    // NOLINTNEXTLINE(modernize-pass-by-value): its copy constructor is explicit.
    TextureMapperAnimation(utf8_stringview name, const web::GraphicsLayerKeyframeValueList &keyframes,
                           const web::GraphicsLayerAnimation &animation, MonotonicTime::duration startTime,
                           web::Seconds pauseTime, State state) noexcept;

    ~TextureMapperAnimation() noexcept = default;

    TextureMapperAnimation(const TextureMapperAnimation &other) noexcept;

    TextureMapperAnimation &operator=(const TextureMapperAnimation &other) noexcept;

    TextureMapperAnimation(TextureMapperAnimation &&) noexcept = default;

    TextureMapperAnimation &operator=(TextureMapperAnimation &&) noexcept = default;

    /// @brief Adds the animation's value at @p time to @p applicationResults.
    void Apply(ApplicationResult &applicationResults, MonotonicTime::duration time,
               KeepInternalState keepInternalState) noexcept;

    /// @brief Holds it at @p offset into the animation.
    void Pause(web::Seconds offset) noexcept;

    /// @brief Holds it, if it's playing, where it is at @p time, until it's resumed.
    void Suspend(MonotonicTime::duration time) noexcept;

    /// @brief Plays it on from where a suspension held it, from @p now. Paused animations stay paused.
    void Resume(MonotonicTime::duration now) noexcept;

    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const web::GraphicsLayerKeyframeValueList &Keyframes() const noexcept
    {
      return _keyframes;
    }

    KRYS_NODISCARD State GetState() const noexcept
    {
      return _state;
    }

    KRYS_NODISCARD web::TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }

  private:
    void ApplyInternal(ApplicationResult &applicationResults, const web::GraphicsLayerAnimationValue &from,
                       const web::GraphicsLayerAnimationValue &to, float progress) noexcept;

    KRYS_NODISCARD web::Seconds ComputeTotalRunningTime(MonotonicTime::duration time) noexcept;
  };

  /// @brief A layer's animations: the transform related ones combined in the order the properties apply.
  class TextureMapperAnimations
  {
  private:
    TransformationMatrix _translate;
    TransformationMatrix _rotate;
    TransformationMatrix _scale;
    TransformationMatrix _transform;

    List<TextureMapperAnimation> _animations;

  public:
    void SetTranslate(const TransformationMatrix &transform) noexcept
    {
      _translate = transform;
    }

    void SetRotate(const TransformationMatrix &transform) noexcept
    {
      _rotate = transform;
    }

    void SetScale(const TransformationMatrix &transform) noexcept
    {
      _scale = transform;
    }

    void SetTransform(const TransformationMatrix &transform) noexcept
    {
      _transform = transform;
    }

    void Add(const TextureMapperAnimation &animation) noexcept;

    void Remove(utf8_stringview name) noexcept;

    void Remove(utf8_stringview name, web::AnimatedProperty property) noexcept;

    void Pause(utf8_stringview name, web::Seconds offset) noexcept;

    void Suspend(MonotonicTime::duration time) noexcept;

    void Resume(MonotonicTime::duration now) noexcept;

    void Apply(TextureMapperAnimation::ApplicationResult &applicationResults, MonotonicTime::duration time,
               TextureMapperAnimation::KeepInternalState keepInternalState =
                 TextureMapperAnimation::KeepInternalState::No) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _animations.empty();
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _animations.size();
    }

    KRYS_NODISCARD const List<TextureMapperAnimation> &Animations() const noexcept
    {
      return _animations;
    }

    KRYS_NODISCARD List<TextureMapperAnimation> &Animations() noexcept
    {
      return _animations;
    }

    KRYS_NODISCARD bool HasActiveAnimationsOfType(web::AnimatedProperty type) const noexcept;

    KRYS_NODISCARD bool HasRunningAnimations() const noexcept;

    KRYS_NODISCARD bool HasRunningTransformAnimations() const noexcept;
  };
}
