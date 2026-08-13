#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveData.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveKeywordList.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericConcepts.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRaw.hpp"

namespace Krys::HTML
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
    using Raw = RawType;
    using Calc = UnevaluatedCalc<Raw>;
    using UnitType = typename Raw::UnitType;
    using UnitTraits = typename Raw::UnitTraits;
    using ResolvedValueType = typename Raw::ResolvedValueType;
    using Data = PrimitiveData<PrimitiveNumeric<RawType>>;
    using Index = typename Data::Index;
    static constexpr auto range = Raw::range;
    static constexpr auto category = Raw::category;

    PrimitiveNumeric(Raw raw) : m_data {raw}
    {
    }

    PrimitiveNumeric(Calc calc) : m_data {WTF::move(calc)}
    {
    }

    template <typename T>
    requires std::integral<T>
             || std::floating_point<T>
             PrimitiveNumeric(T value)
             requires(requires {
               { Raw(value) };
             })
        : m_data {Raw {value}}
    {
    }

    template <typename T>
    requires std::integral<T>
             || std::floating_point<T>
             PrimitiveNumeric(UnitEnum auto unit, T value)
             requires(requires {
               { Raw(unit, value) };
             })
        : m_data {Raw {unit, value}}
    {
    }

    template <UnitEnum E, E unitValue>
    constexpr PrimitiveNumeric(ValueLiteral<unitValue> value)
    requires(requires {
      { Raw(value) };
    })
        : m_data {Raw {value}}
    {
    }

    // MARK: Copy/Move Construction/Assignment

    PrimitiveNumeric(const PrimitiveNumeric &other) : m_data {other.m_data}
    {
    }

    PrimitiveNumeric(PrimitiveNumeric &&other) : m_data {WTF::move(other.m_data)}
    {
    }

    PrimitiveNumeric &operator=(const PrimitiveNumeric &other)
    {
      m_data = other.m_data;
      return *this;
    }

    PrimitiveNumeric &operator=(PrimitiveNumeric &&other)
    {
      m_data = WTF::move(other.m_data);
      return *this;
    }

    // MARK: Equality

    bool operator==(const PrimitiveNumeric &other) const
    {
      return m_data == other.m_data;
    }

    bool operator==(const Raw &other) const
    {
      return m_data == other;
    }

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    constexpr bool operator==(const T &other) const
    {
      return m_data == other;
    }

    template <UnitType unitValue>
    bool operator==(const ValueLiteral<unitValue> &other) const
    {
      return m_data == other;
    }

    template <NestedUnitEnumOf<UnitType> E, E unitValue>
    bool operator==(const ValueLiteral<unitValue> &other) const
    {
      return m_data == other;
    }

    // MARK: Conditional Accessors

    std::optional<Raw> raw() const
    {
      return m_data.raw();
    }

    std::optional<Calc> calc() const
    {
      return m_data.calc();
    }

    // MARK: Variant-Like Conformance

    template <typename T>
    bool HoldsAlternative() const
    {
      if constexpr (std::same_as<T, Calc>)
      {
        return isCalc();
      }
      else if constexpr (std::same_as<T, Raw>)
      {
        return isRaw();
      }
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const
    {
      auto visitor = Krys::CreateVisitor(std::forward<F>(f)...);

      if (isCalc())
        return visitor(asCalc());
      return visitor(asRaw());
    }

    bool isKnownZero() const
    {
      return isRaw() && m_data.payload.number == 0;
    }
    bool isKnownNotZero() const
    {
      return isRaw() && m_data.payload.number != 0;
    }

    bool isRaw() const
    {
      return m_data.isRaw();
    }
    bool isCalc() const
    {
      return m_data.isCalc();
    }
    bool isEmpty() const
    {
      return m_data.isEmpty();
    }

  private:
    template <Numeric, PrimitiveKeyword...>
    friend struct PrimitiveNumericOrKeyword;

    PrimitiveNumeric(PrimitiveDataEmptyToken token) : m_data {token}
    {
    }

    Raw asRaw() const
    {
      return m_data.asRaw();
    }
    Calc asCalc() const
    {
      return m_data.asCalc();
    }

    Data m_data;
  };

#pragma endregion

#pragma region Integer Primitive

  template <CSSRange R = All, typename V = int>
  struct Integer : PrimitiveNumeric<IntegerRaw<R, V>>
  {
    using Base = PrimitiveNumeric<IntegerRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Number Primitive

  template <CSSRange R = All, typename V = double>
  struct Number : PrimitiveNumeric<NumberRaw<R, V>>
  {
    using Base = PrimitiveNumeric<NumberRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Percentage Primitive

  template <CSSRange R = All, typename V = double>
  struct Percentage : PrimitiveNumeric<PercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<PercentageRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension Primitive

  template <CSSRange R = All, typename V = double>
  struct Angle : PrimitiveNumeric<AngleRaw<R, V>>
  {
    using Base = PrimitiveNumeric<AngleRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = float>
  struct Length : PrimitiveNumeric<LengthRaw<R, V>>
  {
    using Base = PrimitiveNumeric<LengthRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct Time : PrimitiveNumeric<TimeRaw<R, V>>
  {
    using Base = PrimitiveNumeric<TimeRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct Frequency : PrimitiveNumeric<FrequencyRaw<R, V>>
  {
    using Base = PrimitiveNumeric<FrequencyRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = NonNegative, typename V = double>
  struct Resolution : PrimitiveNumeric<ResolutionRaw<R, V>>
  {
    using Base = PrimitiveNumeric<ResolutionRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct Flex : PrimitiveNumeric<FlexRaw<R, V>>
  {
    using Base = PrimitiveNumeric<FlexRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension + Percentage Primitives

  template <CSSRange R = All, typename V = float>
  struct AnglePercentage : PrimitiveNumeric<AnglePercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<AnglePercentageRaw<R, V>>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = float>
  struct LengthPercentage : PrimitiveNumeric<LengthPercentageRaw<R, V>>
  {
    using Base = PrimitiveNumeric<LengthPercentageRaw<R, V>>;
    using Base::Base;
  };

#pragma endregion

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
}
