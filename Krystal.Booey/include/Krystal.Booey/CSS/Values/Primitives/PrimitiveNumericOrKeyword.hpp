#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  template <Numeric, PrimitiveKeyword...>
  struct PrimitiveNumericOrKeyword;
}

namespace krys
{
  template <typename N, typename... Ks>
  struct MarkableTraits<krys::boo::css::PrimitiveNumericOrKeyword<N, Ks...>>;
}

namespace krys::boo::css
{
  // MARK: Primitive Numeric or Keyword

  template <Numeric NumericType, PrimitiveKeyword... Ks>
  struct PrimitiveNumericOrKeyword
  {
    friend struct krys::MarkableTraits<PrimitiveNumericOrKeyword>;

    using Raw = typename NumericType::Raw;
    using Calc = typename NumericType::Calc;
    using UnitType = typename NumericType::UnitType;
    using UnitTraits = typename NumericType::UnitTraits;
    using ResolvedValueType = typename NumericType::ResolvedValueType;
    using Data = PrimitiveData<NumericType, Ks...>;
    using Index = typename Data::Index;
    using Keywords = typename Data::Keywords;
    constexpr static auto Range = NumericType::Range;
    constexpr static auto Category = NumericType::Category;

  private:
    Data _data;

  public:
    // MARK: Constructors

    PrimitiveNumericOrKeyword(Raw raw) noexcept : _data {raw}
    {
    }

    PrimitiveNumericOrKeyword(Calc calc) noexcept : _data {krys::move(calc)}
    {
    }

    template <typename T>
    requires(Integral<T> || FloatingPoint<T>)
    PrimitiveNumericOrKeyword(T value) noexcept
    requires(requires {
      { Raw(value) };
    })
        : _data {Raw {value}}
    {
    }

    template <typename T>
    requires(Integral<T> || FloatingPoint<T>)
    PrimitiveNumericOrKeyword(UnitEnum auto unit, T value) noexcept
    requires(requires {
      { Raw(unit, value) };
    })
        : _data {Raw {unit, value}}
    {
    }

    template <UnitEnum E, E UnitValue>
    PrimitiveNumericOrKeyword(ValueLiteral<UnitValue> value) noexcept
    requires(requires {
      { Raw(value) };
    })
        : _data {Raw {value}}
    {
    }

    template <typename... U>
    PrimitiveNumericOrKeyword(Variant<U...> &&variant) noexcept
        : _data {krys::SwitchOn(
            krys::move(variant), [](NumericType &&numeric) { return Data {krys::move(numeric._data)}; },
            [](ValidKeywordForList<Keywords> auto keyword) { return Data {keyword}; })}
    {
    }

    // MARK: Copy/Move Construction/Assignment

    PrimitiveNumericOrKeyword(const PrimitiveNumericOrKeyword &other) noexcept : _data {other._data}
    {
    }

    PrimitiveNumericOrKeyword(PrimitiveNumericOrKeyword &&other) noexcept : _data {krys::move(other._data)}
    {
    }

    PrimitiveNumericOrKeyword &operator=(const PrimitiveNumericOrKeyword &other) noexcept
    {
      _data = other._data;
      return *this;
    }

    PrimitiveNumericOrKeyword &operator=(PrimitiveNumericOrKeyword &&other) noexcept
    {
      _data = krys::move(other._data);
      return *this;
    }

    // MARK: Construction/Assignment from `NumericType`

    PrimitiveNumericOrKeyword(const NumericType &other) noexcept : _data {other._data}
    {
    }

    PrimitiveNumericOrKeyword(NumericType &&other) noexcept : _data {krys::move(other._data)}
    {
    }

    PrimitiveNumericOrKeyword &operator=(const NumericType &other) noexcept
    {
      _data = other._data;
      return *this;
    }

    PrimitiveNumericOrKeyword &operator=(NumericType &&other) noexcept
    {
      _data = krys::move(other._data);
      return *this;
    }

    // MARK: Construction/Assignment from `Keywords...`

    PrimitiveNumericOrKeyword(ValidKeywordForList<Keywords> auto keyword) noexcept : _data {keyword}
    {
    }

    // MARK: Equality

    KRYS_NODISCARD bool operator==(const PrimitiveNumericOrKeyword &other) const noexcept
    {
      return _data == other._data;
    }

    KRYS_NODISCARD bool operator==(ValidKeywordForList<Keywords> auto const &other) const noexcept
    {
      return _data == other;
    }

    KRYS_NODISCARD bool operator==(const Raw &other) const noexcept
    {
      return _data == other;
    }

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    KRYS_NODISCARD constexpr bool operator==(const T &other) const noexcept
    {
      return _data == other;
    }

    template <UnitType UnitValue>
    KRYS_NODISCARD bool operator==(const ValueLiteral<UnitValue> &other) const noexcept
    {
      return _data == other;
    }

    template <NestedUnitEnumOf<UnitType> E, E UnitValue>
    KRYS_NODISCARD bool operator==(const ValueLiteral<UnitValue> &other) const noexcept
    {
      return _data == other;
    }

    // MARK: Conditional Accessors

    KRYS_NODISCARD Maybe<Raw> GetRaw() const noexcept
    {
      return _data.GetRaw();
    }

    KRYS_NODISCARD Maybe<Calc> GetCalc() const noexcept
    {
      return _data.GetCalc();
    }

    // MARK: Variant-Like Conformance

    template <typename T>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept
    {
      if constexpr (SameType<T, NumericType>)
      {
        return IsCalc() || IsRaw();
      }
      else if constexpr (SameType<T, Calc>)
      {
        return IsCalc();
      }
      else if constexpr (SameType<T, Raw>)
      {
        return IsRaw();
      }
      else if constexpr (ValidKeywordForList<T, Keywords>)
      {
        return IsKeyword<T>();
      }
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return _data.Visit(krys::CreateVisitor(std::forward<F>(f)...));
    }

    KRYS_NODISCARD bool IsKnownZero() const noexcept
    {
      return IsRaw() && _data._payload.Number == 0;
    }

    KRYS_NODISCARD bool IsKnownNotZero() const noexcept
    {
      return IsRaw() && _data._payload.Number != 0;
    }

    KRYS_NODISCARD bool IsRaw() const noexcept
    {
      return _data.IsRaw();
    }

    KRYS_NODISCARD bool IsCalc() const noexcept
    {
      return _data.IsCalc();
    }

    template <ValidKeywordForList<Keywords> Keyword>
    KRYS_NODISCARD bool IsKeyword() const noexcept
    {
      return _data.IsKeyword(Keyword {});
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _data.IsEmpty();
    }

  private:
    PrimitiveNumericOrKeyword(PrimitiveDataEmptyToken token) noexcept : _data {token}
    {
    }

    KRYS_NODISCARD Raw AsRaw() const noexcept
    {
      return _data.AsRaw();
    }

    KRYS_NODISCARD Calc AsCalc() const noexcept
    {
      return _data.AsCalc();
    }
  };

  template <typename N, typename... Ks>
  constexpr auto TreatAsVariantLike<PrimitiveNumericOrKeyword<N, Ks...>> = true;
}

namespace krys
{
  template <typename N, typename... Ks>
  struct MarkableTraits<krys::boo::css::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    KRYS_NODISCARD static bool
      IsEmptyValue(const krys::boo::css::PrimitiveNumericOrKeyword<N, Ks...> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static krys::boo::css::PrimitiveNumericOrKeyword<N, Ks...> EmptyValue() noexcept
    {
      return krys::boo::css::PrimitiveNumericOrKeyword<N, Ks...> {krys::boo::css::PrimitiveDataEmptyToken {}};
    }
  };
}
