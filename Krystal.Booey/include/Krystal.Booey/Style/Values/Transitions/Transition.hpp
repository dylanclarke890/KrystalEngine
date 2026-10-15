#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Transitions/SingleTransitionDelay.hpp"
#include "Krystal.Booey/Style/Values/Transitions/SingleTransitionDuration.hpp"
#include "Krystal.Booey/Style/Values/Transitions/SingleTransitionProperty.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // macro(ownerType, property, type, lowercaseName, uppercaseName)

// clang-format off
#define KRYS_FOR_EACH_TRANSITION_REFERENCE(macro)                                     \
  macro(Transition, TransitionProperty, SingleTransitionProperty, property, Property) \
  macro(Transition, TransitionTimingFunction, EasingFunction, timingFunction, TimingFunction)

#define KRYS_FOR_EACH_TRANSITION_VALUE(macro)                             \
  macro(Transition, TransitionDelay, SingleTransitionDelay, delay, Delay) \
  macro(Transition, TransitionDuration, SingleTransitionDuration, duration, Duration)

#define KRYS_FOR_EACH_TRANSITION_ENUM(macro) \
  macro(Transition, TransitionBehavior, TransitionBehavior, behavior, Behavior)

#define KRYS_FOR_EACH_TRANSITION_PROPERTY(macro) \
  KRYS_FOR_EACH_TRANSITION_REFERENCE(macro)      \
  KRYS_FOR_EACH_TRANSITION_VALUE(macro)          \
  KRYS_FOR_EACH_TRANSITION_ENUM(macro)
// clang-format on

  /// @brief One transition of an element's `transition-*` coordinated value list.
  struct Transition
  {
  private:
    struct TransitionData : RefCounted<TransitionData>
    {
      SingleTransitionProperty _property;
      SingleTransitionDelay _delay;
      SingleTransitionDuration _duration;
      EasingFunction _timingFunction;
      uint8 _behavior : 1;

      KRYS_FOR_EACH_TRANSITION_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS)

      KRYS_NODISCARD static Ref<TransitionData> Create() noexcept
      {
        return AdoptRef(*new TransitionData());
      }
      KRYS_NODISCARD static Ref<TransitionData> Create(const TransitionData &other) noexcept
      {
        return AdoptRef(*new TransitionData(other));
      }

      TransitionData() noexcept;
      TransitionData(const TransitionData &) noexcept;

      KRYS_NODISCARD bool operator==(const TransitionData &) const noexcept;
    };

    Ref<TransitionData> _data;

  public:
    Transition() noexcept;
    Transition(SingleTransitionProperty &&) noexcept;

    KRYS_NODISCARD const SingleTransitionProperty &GetProperty() const noexcept
    {
      return _data->_property;
    }
    KRYS_NODISCARD SingleTransitionDelay GetDelay() const noexcept
    {
      return _data->_delay;
    }
    KRYS_NODISCARD SingleTransitionDuration GetDuration() const noexcept
    {
      return _data->_duration;
    }
    KRYS_NODISCARD const EasingFunction &GetTimingFunction() const noexcept
    {
      return _data->_timingFunction;
    }
    KRYS_NODISCARD TransitionBehavior GetBehavior() const noexcept
    {
      return static_cast<TransitionBehavior>(_data->_behavior);
    }

    KRYS_NODISCARD static SingleTransitionProperty InitialProperty() noexcept
    {
      return css::keywords::All {};
    }
    KRYS_NODISCARD static SingleTransitionDelay InitialDelay() noexcept
    {
      return SingleTransitionDelay {0};
    }
    KRYS_NODISCARD static SingleTransitionDuration InitialDuration() noexcept
    {
      return SingleTransitionDuration {0};
    }
    KRYS_NODISCARD static EasingFunction InitialTimingFunction() noexcept
    {
      return EasingFunction {CubicBezierTimingFunction::Create()};
    }
    KRYS_NODISCARD static TransitionBehavior InitialBehavior() noexcept
    {
      return TransitionBehavior::Normal;
    }

    KRYS_FOR_EACH_TRANSITION_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE)
    KRYS_FOR_EACH_TRANSITION_VALUE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_VALUE)
    KRYS_FOR_EACH_TRANSITION_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM)

    KRYS_NODISCARD bool operator==(const Transition &other) const noexcept
    {
      return _data.get() == other._data.get() || *_data == *other._data;
    }

    // CoordinatedValueList interface.

    constexpr static auto ComputedValueUsesUsedValues = false;
    constexpr static auto BaseProperty = css::PropertyNameConstant<css::PropertyId::TransitionProperty> {};
    constexpr static auto Properties =
      std::tuple {KRYS_FOR_EACH_TRANSITION_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY)};

    KRYS_NODISCARD static Transition Clone(const Transition &other) noexcept
    {
      return Transition {TransitionData::Create(*other._data)};
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _data->_property.IsAll();
    }

  private:
    // Needed by macros to access members.
    KRYS_NODISCARD TransitionData &Data() noexcept
    {
      return *_data;
    }
    KRYS_NODISCARD const TransitionData &Data() const noexcept
    {
      return *_data;
    }

    explicit Transition(Ref<TransitionData> &&data) noexcept : _data {krys::move(data)}
    {
    }
  };

  KRYS_FOR_EACH_TRANSITION_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE)
  KRYS_FOR_EACH_TRANSITION_VALUE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_VALUE)
  KRYS_FOR_EACH_TRANSITION_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM)

#undef KRYS_FOR_EACH_TRANSITION_REFERENCE
#undef KRYS_FOR_EACH_TRANSITION_VALUE
#undef KRYS_FOR_EACH_TRANSITION_ENUM
#undef KRYS_FOR_EACH_TRANSITION_PROPERTY
}
