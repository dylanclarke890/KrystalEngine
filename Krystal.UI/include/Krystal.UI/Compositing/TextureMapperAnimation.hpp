#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimation.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::boo
{
  class GraphicsLayerAnimationValue;
}

namespace krys::ui
{
  /// @brief A layer's animation of one property, sampled at the times the layer is composited.
  class TextureMapperAnimation
  {
  public:
    enum class State : uint8
    {
      Playing,
      Paused,
      Stopped
    };

    /// @brief The values a layer's animations produce at a time.
    struct ApplicationResult
    {
      Maybe<boo::TransformationMatrix> Transform;
      Maybe<double> Opacity;
      Maybe<boo::FilterOperations> Filters;
      bool HasRunningAnimations {false};
    };

    enum class KeepInternalState : bool
    {
      No,
      Yes
    };

  private:
    utf8_string _name;
    boo::GraphicsLayerKeyframeValueList _keyframes {boo::AnimatedProperty::Invalid};
    RefPtr<boo::TimingFunction> _timingFunction;
    double _iterationCount {0};
    double _duration {0};
    boo::GraphicsLayerAnimation::Direction _direction {boo::GraphicsLayerAnimation::Direction::Normal};
    bool _fillsForwards {false};
    MonotonicTime::duration _startTime {};
    boo::Seconds _pauseTime {};
    boo::Seconds _totalRunningTime {};
    MonotonicTime::duration _lastRefreshedTime {};
    State _state {State::Stopped};

  public:
    TextureMapperAnimation() noexcept = default;
    TextureMapperAnimation(utf8_stringview name, const boo::GraphicsLayerKeyframeValueList &keyframes,
                           const boo::GraphicsLayerAnimation &animation, MonotonicTime::duration startTime,
                           boo::Seconds pauseTime, State state) noexcept;
    ~TextureMapperAnimation() noexcept = default;

    TextureMapperAnimation(const TextureMapperAnimation &other) noexcept;
    TextureMapperAnimation &operator=(const TextureMapperAnimation &other) noexcept;
    TextureMapperAnimation(TextureMapperAnimation &&) noexcept = default;
    TextureMapperAnimation &operator=(TextureMapperAnimation &&) noexcept = default;

    /// @brief Adds the animation's value at @p time to @p applicationResults.
    void Apply(ApplicationResult &applicationResults, MonotonicTime::duration time,
               KeepInternalState keepInternalState) noexcept;

    void Pause(boo::Seconds time) noexcept;
    void Resume(MonotonicTime::duration now) noexcept;

    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }
    KRYS_NODISCARD const boo::GraphicsLayerKeyframeValueList &Keyframes() const noexcept
    {
      return _keyframes;
    }
    KRYS_NODISCARD State GetState() const noexcept
    {
      return _state;
    }
    KRYS_NODISCARD boo::TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }

  private:
    void ApplyInternal(ApplicationResult &applicationResults, const boo::GraphicsLayerAnimationValue &from,
                       const boo::GraphicsLayerAnimationValue &to, float progress) noexcept;
    KRYS_NODISCARD boo::Seconds ComputeTotalRunningTime(MonotonicTime::duration time) noexcept;
  };

  /// @brief A layer's animations: the transform related ones combined in the order the properties apply.
  class TextureMapperAnimations
  {
  private:
    boo::TransformationMatrix _translate;
    boo::TransformationMatrix _rotate;
    boo::TransformationMatrix _scale;
    boo::TransformationMatrix _transform;

    List<TextureMapperAnimation> _animations;

  public:
    void SetTranslate(const boo::TransformationMatrix &transform) noexcept
    {
      _translate = transform;
    }
    void SetRotate(const boo::TransformationMatrix &transform) noexcept
    {
      _rotate = transform;
    }
    void SetScale(const boo::TransformationMatrix &transform) noexcept
    {
      _scale = transform;
    }
    void SetTransform(const boo::TransformationMatrix &transform) noexcept
    {
      _transform = transform;
    }

    void Add(const TextureMapperAnimation &animation) noexcept;
    void Remove(utf8_stringview name) noexcept;
    void Remove(utf8_stringview name, boo::AnimatedProperty property) noexcept;
    void Pause(utf8_stringview name, boo::Seconds offset) noexcept;
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

    KRYS_NODISCARD bool HasActiveAnimationsOfType(boo::AnimatedProperty type) const noexcept;

    KRYS_NODISCARD bool HasRunningAnimations() const noexcept;
    KRYS_NODISCARD bool HasRunningTransformAnimations() const noexcept;
  };
}
