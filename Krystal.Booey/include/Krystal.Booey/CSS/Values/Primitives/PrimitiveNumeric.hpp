#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericRaw.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveKeywordList.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::css
{
  template <NumericRaw>
  struct PrimitiveNumeric;

  template <Numeric, PrimitiveKeyword...>
  struct PrimitiveNumericOrKeyword;

#pragma region Primitive Numeric(Raw + UnevaluatedCalc)

  // NOTE: As is the case for `PrimitiveNumericRaw`, `ResolvedValueType` here only affects the type
  // the CSS value gets resolved to. Unresolved CSS primitive numeric types always use a `double` as
  // its internal representation.

  template <NumericRaw RawType>
  struct PrimitiveNumeric
  {
    template <typename>
    friend struct PrimitiveDataMarkableTraits;

    template <Numeric, PrimitiveKeyword...>
    friend struct PrimitiveNumericOrKeyword;

    using Raw = RawType;
    using Calc = UnevaluatedCalc<Raw>;
    using UnitType = typename Raw::UnitType;
    using UnitTraits = typename Raw::UnitTraits;
    using ResolvedValueType = typename Raw::ResolvedValueType;
    using Data = PrimitiveData<PrimitiveNumeric<RawType>>;
    using Index = typename Data::Index;

    constexpr static auto Range = Raw::Range;
    constexpr static auto Category = Raw::Category;

  private:
    Data _data;

    constexpr explicit PrimitiveNumeric(PrimitiveDataEmptyToken token) noexcept : _data {token}
    {
    }

  public:
    constexpr PrimitiveNumeric(Raw raw) noexcept : _data {raw}
    {
    }

    constexpr PrimitiveNumeric(Calc calc) noexcept : _data {krys::move(calc)}
    {
    }

    template <typename T>
    requires(Integral<T> || FloatingPoint<T>)
    constexpr PrimitiveNumeric(T value) noexcept
    requires(requires {
      { Raw(value) };
    })
        : _data {Raw {value}}
    {
    }

    template <typename T>
    requires(Integral<T> || FloatingPoint<T>)
    constexpr PrimitiveNumeric(UnitEnum auto unit, T value) noexcept
    requires(requires {
      { Raw(unit, value) };
    })
        : _data {Raw {unit, value}}
    {
    }

    template <UnitEnum E, E UnitValue>
    constexpr PrimitiveNumeric(ValueLiteral<UnitValue> value) noexcept
    requires(requires {
      { Raw(value) };
    })
        : _data {Raw {value}}
    {
    }

    // MARK: Copy/Move Construction/Assignment

    constexpr PrimitiveNumeric(const PrimitiveNumeric &other) noexcept : _data {other._data}
    {
    }

    constexpr PrimitiveNumeric(PrimitiveNumeric &&other) noexcept : _data {krys::move(other._data)}
    {
    }

    constexpr PrimitiveNumeric &operator=(const PrimitiveNumeric &other) noexcept
    {
      _data = other._data;
      return *this;
    }

    constexpr PrimitiveNumeric &operator=(PrimitiveNumeric &&other) noexcept
    {
      _data = krys::move(other._data);
      return *this;
    }

    // MARK: Equality

    constexpr bool operator==(const PrimitiveNumeric &other) const noexcept
    {
      return _data == other._data;
    }

    constexpr bool operator==(const Raw &other) const noexcept
    {
      return _data == other;
    }

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    constexpr bool operator==(const T &other) const noexcept
    {
      return _data == other;
    }

    template <UnitType UnitValue>
    constexpr bool operator==(const ValueLiteral<UnitValue> &other) const noexcept
    {
      return _data == other;
    }

    template <NestedUnitEnumOf<UnitType> E, E UnitValue>
    constexpr bool operator==(const ValueLiteral<UnitValue> &other) const noexcept
    {
      return _data == other;
    }

    // MARK: Conditional Accessors

    KRYS_NODISCARD constexpr Maybe<Raw> raw() const noexcept
    {
      return _data.raw();
    }

    KRYS_NODISCARD constexpr Maybe<Calc> calc() const noexcept
    {
      return _data.calc();
    }

    // MARK: Variant-Like Conformance

    template <typename T>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      if constexpr (SameType<T, Calc>)
      {
        return IsCalc();
      }
      else if constexpr (SameType<T, Raw>)
      {
        return IsRaw();
      }
    }

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsCalc())
      {
        return visitor(AsCalc());
      }

      return visitor(AsRaw());
    }

    KRYS_NODISCARD constexpr bool IsKnownZero() const noexcept
    {
      return IsRaw() && _data.payload.number == 0;
    }

    KRYS_NODISCARD constexpr bool IsKnownNotZero() const noexcept
    {
      return IsRaw() && _data.payload.number != 0;
    }

    KRYS_NODISCARD constexpr bool IsRaw() const noexcept
    {
      return _data.IsRaw();
    }

    KRYS_NODISCARD constexpr bool IsCalc() const noexcept
    {
      return _data.IsCalc();
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _data.IsEmpty();
    }

  private:
    KRYS_NODISCARD constexpr Raw AsRaw() const noexcept
    {
      return _data.AsRaw();
    }

    KRYS_NODISCARD constexpr Calc AsCalc() const noexcept
    {
      return _data.AsCalc();
    }
  };

#pragma endregion

#pragma region Integer Primitive

  template <Range R = All, typename V = int>
  struct Integer : PrimitiveNumeric<IntegerRaw<R, V>>
  {
    using Base = PrimitiveNumeric<IntegerRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Number Primitive

  template <Range R = All, typename V = double>
  struct Number : PrimitiveNumeric<NumberRaw<R, V>>
  {
    using Base = PrimitiveNumeric<NumberRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Percentage Primitive

  template <Range R = All, typename V = double>
  struct Percentage : PrimitiveNumeric<PercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<PercentageRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension Primitive

  template <Range R = All, typename V = double>
  struct Angle : PrimitiveNumeric<AngleRaw<R, V>>
  {
    using Base = PrimitiveNumeric<AngleRaw<R, V>>;
    using Base::Base;
  };

  template <Range R = All, typename V = float>
  struct Length : PrimitiveNumeric<LengthRaw<R, V>>
  {
    using Base = PrimitiveNumeric<LengthRaw<R, V>>;
    using Base::Base;
  };

  template <Range R = All, typename V = double>
  struct Time : PrimitiveNumeric<TimeRaw<R, V>>
  {
    using Base = PrimitiveNumeric<TimeRaw<R, V>>;
    using Base::Base;
  };

  template <Range R = All, typename V = double>
  struct Frequency : PrimitiveNumeric<FrequencyRaw<R, V>>
  {
    using Base = PrimitiveNumeric<FrequencyRaw<R, V>>;
    using Base::Base;
  };

  template <Range R = NonNegative, typename V = double>
  struct Resolution : PrimitiveNumeric<ResolutionRaw<R, V>>
  {
    using Base = PrimitiveNumeric<ResolutionRaw<R, V>>;
    using Base::Base;
  };

  template <Range R = All, typename V = double>
  struct Flex : PrimitiveNumeric<FlexRaw<R, V>>
  {
    using Base = PrimitiveNumeric<FlexRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension + Percentage Primitives

  template <Range R = All, typename V = float>
  struct AnglePercentage : PrimitiveNumeric<AnglePercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<AnglePercentageRaw<R, V>>;
    using Base::Base;
    using MarkableTraits = PrimitiveDataMarkableTraits<AnglePercentage<R, V>>;
  };

  template <Range R = All, typename V = float>
  struct LengthPercentage : PrimitiveNumeric<LengthPercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<LengthPercentageRaw<R, V>>;
    using Base::Base;
    using MarkableTraits = PrimitiveDataMarkableTraits<LengthPercentage<R, V>>;
  };

#pragma endregion

#pragma region TreatAsVariantLike Specializations

  template <typename Raw>
  constexpr auto TreatAsVariantLike<PrimitiveNumeric<Raw>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Integer<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Number<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Percentage<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Angle<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Length<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Time<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Frequency<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Resolution<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<Flex<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<AnglePercentage<R, V>> = true;

  template <auto R, typename V>
  constexpr auto TreatAsVariantLike<LengthPercentage<R, V>> = true;

#pragma endregion
}

#pragma region MarkableTraits Specializations

namespace krys
{
  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Integer<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Integer<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Number<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Number<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Percentage<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Percentage<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Angle<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Angle<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Length<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Length<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Time<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Time<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Frequency<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Frequency<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Resolution<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Resolution<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::Flex<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::Flex<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::AnglePercentage<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::AnglePercentage<R, V>>
  {
  };

  template <krys::boo::css::Range R, typename V>
  struct MarkableTraits<krys::boo::css::LengthPercentage<R, V>>
      : public krys::boo::css::PrimitiveDataMarkableTraits<krys::boo::css::LengthPercentage<R, V>>
  {
  };
}

#pragma endregion
