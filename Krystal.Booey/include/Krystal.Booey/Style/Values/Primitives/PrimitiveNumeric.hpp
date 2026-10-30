#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericForward.hpp"
#include "Krystal.Booey/Style/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/CompactVariant.hpp"
#include "Krystal.Core/Types/FlatteningVariant.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::style
{
  template <typename>
  struct DimensionPercentageMapping;

  struct PrimitiveNumericEmptyToken
  {
    KRYS_NODISCARD constexpr bool operator==(const PrimitiveNumericEmptyToken &) const noexcept = default;
  };

  template <typename T>
  struct PrimitiveNumericMarkableTraits
  {
    KRYS_NODISCARD static bool IsEmptyValue(const T &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static T EmptyValue() noexcept
    {
      return T(PrimitiveNumericEmptyToken {});
    }
  };

  // Default implementation of `PrimitiveNumeric` for non-composite numeric types.
  template <css::Numeric CSSType>
  struct PrimitiveNumeric
  {
    template <typename>
    friend struct PrimitiveNumericMarkableTraits;

    using CSS = CSSType;
    using Raw = typename CSS::Raw;
    using UnitType = typename CSS::UnitType;
    using UnitTraits = typename CSS::UnitTraits;
    using ResolvedValueType = typename CSS::ResolvedValueType;
    constexpr static auto Range = CSS::Range;
    constexpr static auto Category = CSS::Category;

    constexpr static auto Unit = UnitTraits::Canonical;
    ResolvedValueType Value {0};

    constexpr PrimitiveNumeric(ResolvedValueType value) noexcept : Value {value}
    {
    }

    constexpr PrimitiveNumeric(css::ValueLiteral<UnitTraits::Canonical> literal) noexcept
        : Value {ClampTo<ResolvedValueType>(literal.Value)}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    requires(Range.Min <= 0 && Range.Max >= 0)
    {
      return !Value;
    }

    KRYS_NODISCARD constexpr bool IsKnownZero() const noexcept
    requires(Range.Min <= 0 && Range.Max >= 0)
    {
      return IsZero();
    }

    KRYS_NODISCARD constexpr bool IsPositive() const noexcept
    requires(Range.Max > 0)
    {
      return Value > 0;
    }

    KRYS_NODISCARD constexpr bool IsKnownPositive() const noexcept
    requires(Range.Max > 0)
    {
      return IsPositive();
    }

    KRYS_NODISCARD constexpr bool IsPositiveOrZero() const noexcept
    requires(Range.Max >= 0)
    {
      return Value >= 0;
    }

    KRYS_NODISCARD constexpr bool IsNegative() const noexcept
    requires(Range.Min < 0)
    {
      return Value < 0;
    }

    KRYS_NODISCARD constexpr bool IsKnownNegative() const noexcept
    requires(Range.Min < 0)
    {
      return IsNegative();
    }

    KRYS_NODISCARD constexpr bool IsNegativeOrZero() const noexcept
    requires(Range.Min <= 0)
    {
      return Value <= 0;
    }

    KRYS_NODISCARD constexpr bool operator==(const PrimitiveNumeric &) const noexcept = default;

    KRYS_NODISCARD constexpr bool operator==(ResolvedValueType other) const noexcept
    {
      return Value == other;
    }

    KRYS_NODISCARD constexpr auto operator<=>(const PrimitiveNumeric &) const noexcept = default;

  private:
    // Markable is supported for numeric values that have free bits. These currently include:
    //  - any floating point value (using NaN).
    //  - any numeric value where the minimum allowed value is greater than 0 (using 0).
    //  - any numeric value where the minimum allowed value is equal to 0 and the minimum representable value
    //    is not zero (using -1).
    KRYS_NODISCARD consteval static ResolvedValueType EmptyValue() noexcept
    {
      if constexpr (FloatingPoint<ResolvedValueType>)
      {
        return Limits<ResolvedValueType>::quiet_NaN();
      }
      else if constexpr (Range.Min > 0)
      {
        return 0;
      }
      else if constexpr (Range.Min == 0 && Limits<ResolvedValueType>::min() != 0)
      {
        return -1;
      }
    }

    explicit PrimitiveNumeric(PrimitiveNumericEmptyToken) noexcept : Value {EmptyValue()}
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      if constexpr (FloatingPoint<ResolvedValueType>)
      {
        return std::isnan(Value);
      }
      else
      {
        return Value == EmptyValue();
      }
    }
  };

  // Specialization of `PrimitiveNumeric` for `css::Length` types.
  template <css::Range R, typename V>
  struct PrimitiveNumeric<css::Length<R, V>>
  {
    template <typename>
    friend struct PrimitiveNumericMarkableTraits;

    using CSS = css::Length<R, V>;
    using Raw = typename CSS::Raw;
    using UnitType = typename CSS::UnitType;
    using UnitTraits = typename CSS::UnitTraits;
    using ResolvedValueType = typename CSS::ResolvedValueType;
    constexpr static auto Range = CSS::Range;
    constexpr static auto Category = CSS::Category;

    constexpr static auto Unit = UnitTraits::Canonical;

  private:
    ResolvedValueType _value {0};

  public:
    constexpr PrimitiveNumeric(ResolvedValueType value) noexcept : _value {value}
    {
    }

    constexpr PrimitiveNumeric(css::ValueLiteral<UnitTraits::Canonical> literal) noexcept
        : _value {ClampTo<ResolvedValueType>(literal.Value)}
    {
    }

    KRYS_NODISCARD constexpr auto ResolveZoom(ZoomNeeded) const noexcept
    requires(Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return _value;
    }

    KRYS_NODISCARD constexpr auto ResolveZoom(ZoomFactor zoom) const noexcept
    requires(Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return _value * zoom.Value;
    }

    KRYS_NODISCARD constexpr auto UnresolvedValue() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    requires(Range.Min <= 0 && Range.Max >= 0)
    {
      return !_value;
    }

    KRYS_NODISCARD constexpr bool IsKnownZero() const noexcept
    requires(Range.Min <= 0 && Range.Max >= 0)
    {
      return IsZero();
    }

    KRYS_NODISCARD constexpr bool IsPositive() const noexcept
    requires(Range.Max > 0)
    {
      return _value > 0;
    }

    KRYS_NODISCARD constexpr bool IsKnownPositive() const noexcept
    requires(Range.Max > 0)
    {
      return IsPositive();
    }

    KRYS_NODISCARD constexpr bool IsPositiveOrZero() const noexcept
    requires(Range.Max >= 0)
    {
      return _value >= 0;
    }

    KRYS_NODISCARD constexpr bool IsNegative() const noexcept
    requires(Range.Min < 0)
    {
      return _value < 0;
    }

    KRYS_NODISCARD constexpr bool IsKnownNegative() const noexcept
    requires(Range.Min < 0)
    {
      return IsNegative();
    }

    KRYS_NODISCARD constexpr bool IsNegativeOrZero() const noexcept
    requires(Range.Min <= 0)
    {
      return _value <= 0;
    }

    KRYS_NODISCARD constexpr bool operator==(const PrimitiveNumeric &) const noexcept = default;

    KRYS_NODISCARD constexpr auto operator<=>(const PrimitiveNumeric &) const noexcept = default;

  private:
    // Markable is supported for numeric values that have free bits. These currently include:
    //  - any floating point value (using NaN).
    //  - any numeric value where the minimum allowed value is greater than 0 (using 0).
    //  - any numeric value where the minimum allowed value is equal to 0 and the minimum representable value
    //    is not zero (using -1).
    KRYS_NODISCARD consteval static ResolvedValueType EmptyValue() noexcept
    {
      if constexpr (FloatingPoint<ResolvedValueType>)
      {
        return Limits<ResolvedValueType>::quiet_NaN();
      }
      else if constexpr (Range.Min > 0)
      {
        return 0;
      }
      else if constexpr (Range.Min == 0 && Limits<ResolvedValueType>::min() != 0)
      {
        return -1;
      }
    }

    explicit PrimitiveNumeric(PrimitiveNumericEmptyToken) noexcept : _value {EmptyValue()}
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      if constexpr (FloatingPoint<ResolvedValueType>)
      {
        return std::isnan(_value);
      }
      else
      {
        return _value == EmptyValue();
      }
    }
  };

  // Specialization of `PrimitiveNumeric` for composite dimension-percentage types.
  template <css::DimensionPercentageNumeric CSSType>
  struct PrimitiveNumeric<CSSType>
  {
    using CSS = CSSType;
    using Raw = typename CSS::Raw;
    using UnitType = typename CSS::UnitType;
    using UnitTraits = typename CSS::UnitTraits;
    using ResolvedValueType = typename CSS::ResolvedValueType;
    constexpr static auto Range = CSS::Range;
    constexpr static auto Category = CSS::Category;

    // Composite types only currently support float as the `ResolvedValueType`, allowing unconditional use of
    // `CompactVariant`.
    static_assert(SameType<ResolvedValueType, float>);

    using Dimension = typename DimensionPercentageMapping<CSS>::Dimension;
    using Percentage = typename DimensionPercentageMapping<CSS>::Percentage;
    using Calc = UnevaluatedCalc<CSS>;
    using Representation = CompactVariant<Dimension, Percentage, Calc>;

  private:
    Representation _value;

  public:
    PrimitiveNumeric(Dimension dimension) noexcept : _value {krys::move(dimension)}
    {
    }

    PrimitiveNumeric(Percentage percentage) noexcept : _value {krys::move(percentage)}
    {
    }

    PrimitiveNumeric(Calc calc) noexcept : _value {krys::move(calc)}
    {
    }

    PrimitiveNumeric(css::ValueLiteral<Dimension::UnitTraits::Canonical> literal) noexcept
        : _value {Dimension {literal}}
    {
    }

    PrimitiveNumeric(css::ValueLiteral<Percentage::UnitTraits::Canonical> literal) noexcept
        : _value {Percentage {literal}}
    {
    }

    KRYS_NODISCARD constexpr size_t index() const noexcept
    {
      return _value.index();
    }

    template <typename T>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<T>(_value);
    }

    template <size_t I>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<I>(_value);
    }

    template <typename T>
    KRYS_NODISCARD T get() const noexcept
    {
      return krys::SwitchOn(
        _value, []<SameType<T> U>(const U &alternative) -> T { return alternative; },
        [](const auto &) -> T { krys_unreachable(); });
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...functors) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(functors)...);
    }

    KRYS_NODISCARD constexpr bool IsKnownZero() const noexcept
    requires(Range.Min <= 0 && Range.Max >= 0)
    {
      return krys::SwitchOn(
        _value, []<css::HasIsZero T>(const T &alternative) { return alternative.IsZero(); },
        [](const auto &) { return false; });
    }

    KRYS_NODISCARD bool operator==(const PrimitiveNumeric &) const noexcept = default;
  };

#pragma region Integer Primitive

  template <css::Range R, typename V>
  struct Integer : PrimitiveNumeric<css::Integer<R, V>>
  {
    using Base = PrimitiveNumeric<css::Integer<R, V>>;
    using Base::Base;

    // Allow <integer> values to be initialized with number literals as well as integer literals.
    constexpr Integer(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Integer {ClampTo<typename Base::ResolvedValueType>(literal.Value)}
    {
    }
  };

#pragma endregion

#pragma region Number Primitive

  template <css::Range R, typename V>
  struct Number : PrimitiveNumeric<css::Number<R, V>>
  {
    using Base = PrimitiveNumeric<css::Number<R, V>>;
    using Base::Base;

    // Allow <number> values to be initialized with integer literals as well as number literals.
    constexpr Number(css::ValueLiteral<css::IntegerUnit::Integer> literal) noexcept
        : Number {ClampTo<typename Base::ResolvedValueType>(literal.Value)}
    {
    }
  };

#pragma endregion

#pragma region Percentage Primitive

  template <css::Range R, typename V>
  struct Percentage : PrimitiveNumeric<css::Percentage<R, V>>
  {
    using Base = PrimitiveNumeric<css::Percentage<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension Primitives

  template <css::Range R, typename V>
  struct Angle : PrimitiveNumeric<css::Angle<R, V>>
  {
    using Base = PrimitiveNumeric<css::Angle<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct Length : PrimitiveNumeric<css::Length<R, V>>
  {
    using Base = PrimitiveNumeric<css::Length<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct Time : PrimitiveNumeric<css::Time<R, V>>
  {
    using Base = PrimitiveNumeric<css::Time<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct Frequency : PrimitiveNumeric<css::Frequency<R, V>>
  {
    using Base = PrimitiveNumeric<css::Frequency<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct Resolution : PrimitiveNumeric<css::Resolution<R, V>>
  {
    using Base = PrimitiveNumeric<css::Resolution<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct Flex : PrimitiveNumeric<css::Flex<R, V>>
  {
    using Base = PrimitiveNumeric<css::Flex<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension + Percentage Primitives

  template <css::Range R, typename V>
  struct AnglePercentage : PrimitiveNumeric<css::AnglePercentage<R, V>>
  {
    using Base = PrimitiveNumeric<css::AnglePercentage<R, V>>;
    using Base::Base;
  };

  template <css::Range R, typename V>
  struct LengthPercentage : PrimitiveNumeric<css::LengthPercentage<R, V>>
  {
    using Base = PrimitiveNumeric<css::LengthPercentage<R, V>>;
    using Base::Base;
  };

  template <auto R, typename V>
  struct DimensionPercentageMapping<css::AnglePercentage<R, V>>
  {
    using Dimension = style::Angle<R, V>;
    using Percentage = style::Percentage<R, V>;
  };

  template <auto R, typename V>
  struct DimensionPercentageMapping<css::LengthPercentage<R, V>>
  {
    using Dimension = style::Length<R, V>;
    using Percentage = style::Percentage<R, V>;
  };

  template <typename T>
  KRYS_NODISCARD T get(DimensionPercentageNumeric auto const &dimensionPercentage) noexcept
  {
    return dimensionPercentage.template get<T>();
  }

#pragma endregion

#pragma region CSS -> Style

  template <auto R, typename V>
  struct ToStyleMapping<css::Integer<R, V>>
  {
    using type = Integer<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Number<R, V>>
  {
    using type = Number<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Percentage<R, V>>
  {
    using type = Percentage<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Angle<R, V>>
  {
    using type = Angle<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Length<R, V>>
  {
    using type = Length<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Time<R, V>>
  {
    using type = Time<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Frequency<R, V>>
  {
    using type = Frequency<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Resolution<R, V>>
  {
    using type = Resolution<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::Flex<R, V>>
  {
    using type = Flex<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::AnglePercentage<R, V>>
  {
    using type = AnglePercentage<R, V>;
  };

  template <auto R, typename V>
  struct ToStyleMapping<css::LengthPercentage<R, V>>
  {
    using type = LengthPercentage<R, V>;
  };

#pragma endregion

#pragma region Style -> CSS

  template <Numeric T>
  struct ToCSSMapping<T>
  {
    using type = typename T::CSS;
  };

#pragma endregion

#pragma region Utility Concepts

  template <typename T>
  concept IsPercentage = SameType<T, Percentage<T::Range, typename T::ResolvedValueType>>;

  template <typename T>
  concept IsCalc = SameType<T, UnevaluatedCalc<typename T::CSS>>;

  template <typename T>
  concept IsPercentageOrCalc = IsPercentage<T> || IsCalc<T>;

#pragma endregion
}

namespace krys
{
  template <boo::style::DimensionPercentageNumeric T>
  struct FlatteningVariantTraits<T>
  {
    using TypeList = typename FlatteningVariantTraits<typename T::Representation>::TypeList;
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Integer<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Integer<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Number<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Number<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Percentage<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Percentage<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Angle<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Angle<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Length<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Length<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Time<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Time<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Frequency<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Frequency<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Resolution<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Resolution<R, V>>
  {
  };

  template <auto R, typename V>
  struct MarkableTraits<boo::style::Flex<R, V>>
      : boo::style::PrimitiveNumericMarkableTraits<boo::style::Flex<R, V>>
  {
  };
}

namespace krys::boo::css
{
  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<style::AnglePercentage<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<style::LengthPercentage<R, V>> = true;
}
