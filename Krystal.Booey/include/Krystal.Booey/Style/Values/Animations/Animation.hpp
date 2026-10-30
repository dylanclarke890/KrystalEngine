#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationDelay.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationDuration.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationIterationCount.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationName.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationTimeline.hpp"
#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // macro(ownerType, property, type, lowercaseName, uppercaseName)

// clang-format off
#define KRYS_FOR_EACH_ANIMATION_REFERENCE(macro)                                            \
  macro(Animation, AnimationName, SingleAnimationName, name, Name)                          \
  macro(Animation, AnimationTimeline, SingleAnimationTimeline, timeline, Timeline)          \
  macro(Animation, AnimationTimingFunction, EasingFunction, timingFunction, TimingFunction) \
  macro(Animation, AnimationRangeStart, SingleAnimationRangeStart, rangeStart, RangeStart)  \
  macro(Animation, AnimationRangeEnd, SingleAnimationRangeEnd, rangeEnd, RangeEnd)

#define KRYS_FOR_EACH_ANIMATION_VALUE(macro)                                       \
  macro(Animation, AnimationDelay, SingleAnimationDelay, delay, Delay)             \
  macro(Animation, AnimationDuration, SingleAnimationDuration, duration, Duration) \
  macro(Animation, AnimationIterationCount, SingleAnimationIterationCount, iterationCount, IterationCount)

#define KRYS_FOR_EACH_ANIMATION_ENUM(macro)                                      \
  macro(Animation, AnimationDirection, AnimationDirection, direction, Direction) \
  macro(Animation, AnimationFillMode, AnimationFillMode, fillMode, FillMode)     \
  macro(Animation, AnimationPlayState, AnimationPlayState, playState, PlayState) \
  macro(Animation, AnimationComposition, CompositeOperation, compositeOperation, CompositeOperation)

#define KRYS_FOR_EACH_ANIMATION_SHORTHAND(macro) \
  macro(Animation, AnimationRange, SingleAnimationRange, range, Range)

#define KRYS_FOR_EACH_ANIMATION_PROPERTY(macro) \
  KRYS_FOR_EACH_ANIMATION_REFERENCE(macro)      \
  KRYS_FOR_EACH_ANIMATION_VALUE(macro)          \
  KRYS_FOR_EACH_ANIMATION_ENUM(macro)
// clang-format on

  /// @brief One animation of an element's `animation-*` coordinated value list.
  struct Animation
  {
  private:
    struct AnimationData : RefCounted<AnimationData>
    {
      SingleAnimationName _name;
      SingleAnimationDelay _delay;
      SingleAnimationDuration _duration;
      SingleAnimationIterationCount _iterationCount;
      SingleAnimationTimeline _timeline;
      EasingFunction _timingFunction;
      Maybe<EasingFunction> _defaultTimingFunctionForKeyframes;
      SingleAnimationRangeStart _rangeStart;
      SingleAnimationRangeEnd _rangeEnd;
      uint8 _direction : 2;
      uint8 _fillMode : 2;
      uint8 _playState : 2;
      uint8 _compositeOperation : 2;

      KRYS_FOR_EACH_ANIMATION_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS)

      KRYS_NODISCARD static Ref<AnimationData> Create() noexcept
      {
        return AdoptRef(*new AnimationData());
      }
      KRYS_NODISCARD static Ref<AnimationData> Create(const AnimationData &other) noexcept
      {
        return AdoptRef(*new AnimationData(other));
      }

      AnimationData() noexcept;
      AnimationData(const AnimationData &) noexcept;

      KRYS_NODISCARD bool operator==(const AnimationData &) const noexcept;
    };

    Ref<AnimationData> _data;

  public:
    Animation() noexcept;
    Animation(SingleAnimationName &&) noexcept;

    KRYS_NODISCARD const SingleAnimationName &GetName() const noexcept
    {
      return _data->_name;
    }
    KRYS_NODISCARD SingleAnimationDelay GetDelay() const noexcept
    {
      return _data->_delay;
    }
    KRYS_NODISCARD AnimationDirection GetDirection() const noexcept
    {
      return static_cast<AnimationDirection>(_data->_direction);
    }
    KRYS_NODISCARD SingleAnimationDuration GetDuration() const noexcept
    {
      return _data->_duration;
    }
    KRYS_NODISCARD AnimationFillMode GetFillMode() const noexcept
    {
      return static_cast<AnimationFillMode>(_data->_fillMode);
    }
    KRYS_NODISCARD SingleAnimationIterationCount GetIterationCount() const noexcept
    {
      return _data->_iterationCount;
    }
    KRYS_NODISCARD AnimationPlayState GetPlayState() const noexcept
    {
      return static_cast<AnimationPlayState>(_data->_playState);
    }
    KRYS_NODISCARD const SingleAnimationTimeline &GetTimeline() const noexcept
    {
      return _data->_timeline;
    }
    KRYS_NODISCARD const EasingFunction &GetTimingFunction() const noexcept
    {
      return _data->_timingFunction;
    }
    KRYS_NODISCARD CompositeOperation GetCompositeOperation() const noexcept
    {
      return static_cast<CompositeOperation>(_data->_compositeOperation);
    }
    KRYS_NODISCARD const SingleAnimationRangeStart &GetRangeStart() const noexcept
    {
      return _data->_rangeStart;
    }
    KRYS_NODISCARD const SingleAnimationRangeEnd &GetRangeEnd() const noexcept
    {
      return _data->_rangeEnd;
    }

    KRYS_NODISCARD static SingleAnimationName InitialName() noexcept
    {
      return css::keywords::None {};
    }
    KRYS_NODISCARD static SingleAnimationDelay InitialDelay() noexcept
    {
      return SingleAnimationDelay {0};
    }
    KRYS_NODISCARD static AnimationDirection InitialDirection() noexcept
    {
      return AnimationDirection::Normal;
    }
    KRYS_NODISCARD static SingleAnimationDuration InitialDuration() noexcept
    {
      return css::keywords::Auto {};
    }
    KRYS_NODISCARD static AnimationFillMode InitialFillMode() noexcept
    {
      return AnimationFillMode::None;
    }
    KRYS_NODISCARD static SingleAnimationIterationCount InitialIterationCount() noexcept
    {
      return 1.0;
    }
    KRYS_NODISCARD static AnimationPlayState InitialPlayState() noexcept
    {
      return AnimationPlayState::Running;
    }
    KRYS_NODISCARD static CompositeOperation InitialCompositeOperation() noexcept
    {
      return CompositeOperation::Replace;
    }
    KRYS_NODISCARD static SingleAnimationTimeline InitialTimeline() noexcept
    {
      return css::keywords::Auto {};
    }
    KRYS_NODISCARD static EasingFunction InitialTimingFunction() noexcept
    {
      return EasingFunction {CubicBezierTimingFunction::Create()};
    }
    KRYS_NODISCARD static SingleAnimationRangeStart InitialRangeStart() noexcept
    {
      return css::keywords::Normal {};
    }
    KRYS_NODISCARD static SingleAnimationRangeEnd InitialRangeEnd() noexcept
    {
      return css::keywords::Normal {};
    }

    KRYS_NODISCARD const Maybe<EasingFunction> &DefaultTimingFunctionForKeyframes() const noexcept
    {
      return _data->_defaultTimingFunctionForKeyframes;
    }
    void SetDefaultTimingFunctionForKeyframes(Maybe<EasingFunction> &&function) noexcept
    {
      _data->_defaultTimingFunctionForKeyframes = krys::move(function);
    }

    KRYS_FOR_EACH_ANIMATION_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE)
    KRYS_FOR_EACH_ANIMATION_VALUE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_VALUE)
    KRYS_FOR_EACH_ANIMATION_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM)

    // Support for the `animation-range` shorthand.
    KRYS_NODISCARD static SingleAnimationRange InitialRange() noexcept
    {
      return {InitialRangeStart(), InitialRangeEnd()};
    }
    KRYS_NODISCARD SingleAnimationRange GetRange() const noexcept
    {
      return {GetRangeStart(), GetRangeEnd()};
    }
    void SetRange(SingleAnimationRange &&range) noexcept
    {
      SetRangeStart(krys::move(range.Start));
      SetRangeEnd(krys::move(range.End));
    }
    void FillRange(SingleAnimationRange &&range) noexcept
    {
      FillRangeStart(krys::move(range.Start));
      FillRangeEnd(krys::move(range.End));
    }
    void ClearRange() noexcept
    {
      ClearRangeStart();
      ClearRangeEnd();
    }
    KRYS_NODISCARD bool IsRangeUnset() const noexcept
    {
      return IsRangeStartUnset() && IsRangeEndUnset();
    }
    KRYS_NODISCARD bool IsRangeSet() const noexcept
    {
      return IsRangeStartSet() || IsRangeEndSet();
    }
    KRYS_NODISCARD bool IsRangeFilled() const noexcept
    {
      return IsRangeStartFilled() || IsRangeEndFilled();
    }

    /// @brief Used for animation composite order sorting needed when backing a `CSSAnimation` object.
    /// https://drafts.csswg.org/css-animations-2/#animation-composite-order
    KRYS_NODISCARD uint64 SortingIdentity() const noexcept
    {
      return reinterpret_cast<uint64>(_data.get());
    }

    KRYS_NODISCARD bool operator==(const Animation &other) const noexcept
    {
      return _data.get() == other._data.get() || *_data == *other._data;
    }

    // CoordinatedValueList interface.

    constexpr static auto ComputedValueUsesUsedValues = false;
    constexpr static auto BaseProperty = css::PropertyNameConstant<css::PropertyId::AnimationName> {};
    constexpr static auto Properties =
      std::tuple {KRYS_FOR_EACH_ANIMATION_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY)};

    KRYS_NODISCARD static Animation Clone(const Animation &other) noexcept
    {
      return Animation {AnimationData::Create(*other._data)};
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _data->_name.IsNone();
    }

  private:
    // Needed by macros to access members.
    KRYS_NODISCARD AnimationData &Data() noexcept
    {
      return *_data;
    }
    KRYS_NODISCARD const AnimationData &Data() const noexcept
    {
      return *_data;
    }

    explicit Animation(Ref<AnimationData> &&data) noexcept : _data {krys::move(data)}
    {
    }
  };

  KRYS_FOR_EACH_ANIMATION_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE)
  KRYS_FOR_EACH_ANIMATION_VALUE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE)
  KRYS_FOR_EACH_ANIMATION_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM)
  KRYS_FOR_EACH_ANIMATION_SHORTHAND(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_SHORTHAND)

#undef KRYS_FOR_EACH_ANIMATION_REFERENCE
#undef KRYS_FOR_EACH_ANIMATION_VALUE
#undef KRYS_FOR_EACH_ANIMATION_ENUM
#undef KRYS_FOR_EACH_ANIMATION_SHORTHAND
#undef KRYS_FOR_EACH_ANIMATION_PROPERTY
}
