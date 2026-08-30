#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"

namespace Krys::HTML
{
  // MARK: Primitive Numeric or Keyword

  template <Numeric NumericType, PrimitiveKeyword... Ks>
  struct PrimitiveNumericOrKeyword
  {
    using Raw = typename NumericType::Raw;
    using Calc = typename NumericType::Calc;
    using UnitType = typename NumericType::UnitType;
    using UnitTraits = typename NumericType::UnitTraits;
    using ResolvedValueType = typename NumericType::ResolvedValueType;
    using Data = PrimitiveData<NumericType, Ks...>;
    using Index = typename Data::Index;
    using Keywords = typename Data::Keywords;
    static constexpr auto tokens = NumericType::tokens;
    static constexpr auto category = NumericType::category;

    // MARK: Constructors

    PrimitiveNumericOrKeyword(Raw raw) : m_data {raw}
    {
    }

    PrimitiveNumericOrKeyword(Calc calc) : m_data {Krys::Move(calc)}
    {
    }

    template <typename T>
    requires std::integral<T>
             || std::floating_point<T>
             PrimitiveNumericOrKeyword(T value)
             requires(requires {
               { Raw(value) };
             })
        : m_data {Raw {value}}
    {
    }

    template <typename T>
    requires std::integral<T>
             || std::floating_point<T>
             PrimitiveNumericOrKeyword(UnitEnum auto unit, T value)
             requires(requires {
               { Raw(unit, value) };
             })
        : m_data {Raw {unit, value}}
    {
    }

    template <UnitEnum E, E unitValue>
    constexpr PrimitiveNumericOrKeyword(ValueLiteral<unitValue> value)
    requires(requires {
      { Raw(value) };
    })
        : m_data {Raw {value}}
    {
    }

    template <typename... U>
    constexpr PrimitiveNumericOrKeyword(Variant<U...> &&variant)
        : m_data {WTF::switchOn(
            Krys::Move(variant), [](NumericType &&numeric) { return Data {Krys::Move(numeric.m_data)}; },
            [](ValidKeywordForList<Keywords> auto keyword) { return Data {keyword}; })}
    {
    }

    // MARK: Copy/Move Construction/Assignment

    PrimitiveNumericOrKeyword(const PrimitiveNumericOrKeyword &other) : m_data {other.m_data}
    {
    }

    PrimitiveNumericOrKeyword(PrimitiveNumericOrKeyword &&other) : m_data {Krys::Move(other.m_data)}
    {
    }

    PrimitiveNumericOrKeyword &operator=(const PrimitiveNumericOrKeyword &other)
    {
      m_data = other.m_data;
      return *this;
    }

    PrimitiveNumericOrKeyword &operator=(PrimitiveNumericOrKeyword &&other)
    {
      m_data = Krys::Move(other.m_data);
      return *this;
    }

    // MARK: Construction/Assignment from `NumericType`

    PrimitiveNumericOrKeyword(const NumericType &other) : m_data {other.m_data}
    {
    }

    PrimitiveNumericOrKeyword(NumericType &&other) : m_data {Krys::Move(other.m_data)}
    {
    }

    PrimitiveNumericOrKeyword &operator=(const NumericType &other)
    {
      m_data = other;
      return *this;
    }

    PrimitiveNumericOrKeyword &operator=(NumericType &&other)
    {
      m_data = Krys::Move(other);
      return *this;
    }

    // MARK: Construction/Assignment from `Keywords...`

    PrimitiveNumericOrKeyword(ValidKeywordForList<Keywords> auto keyword) : m_data {keyword}
    {
    }

    // MARK: Equality

    bool operator==(const PrimitiveNumericOrKeyword &other) const
    {
      return m_data == other.m_data;
    }

    bool operator==(ValidKeywordForList<Keywords> auto const &other) const
    {
      return m_data == other;
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
    KRYS_NODISCARD bool HoldsAlternative() const noexcept
    {
      if constexpr (std::same_as<T, NumericType>)
      {
        return isCalc() || isRaw();
      }
      else if constexpr (std::same_as<T, Calc>)
      {
        return isCalc();
      }
      else if constexpr (std::same_as<T, Raw>)
      {
        return isRaw();
      }
      else if constexpr (ValidKeywordForList<T, Keywords>)
      {
        return isKeyword<T>();
      }
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return m_data.Visit(Krys::CreateVisitor(std::forward<F>(f)...));
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
    template <ValidKeywordForList<Keywords> Keyword>
    bool isKeyword() const
    {
      return m_data.template isKeyword<Keyword>();
    }
    bool isEmpty() const
    {
      return m_data.isEmpty();
    }

  private:
    PrimitiveNumericOrKeyword(PrimitiveDataEmptyToken token) : m_data {token}
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
}

namespace Krys
{
  template <typename N, typename... Ks>
  struct MarkableTraits<Krys::HTML::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    static bool IsEmptyValue(const Krys::HTML::PrimitiveNumericOrKeyword<N, Ks...> &value)
    {
      return value.isEmpty();
    }

    static Krys::HTML::PrimitiveNumericOrKeyword<N, Ks...> EmptyValue()
    {
      return {Krys::HTML::PrimitiveDataEmptyToken {}};
    }
  };
}