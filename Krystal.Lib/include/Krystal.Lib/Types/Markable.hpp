#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <cassert>
#include <cmath>
#include <limits>

namespace Krys
{
  template <typename>
  struct MarkableTraits;

  /// @example
  ///     enum class Type { Value1, Value2, Value3 };
  ///     Markable<Type, EnumMarkableTraits<Type, 42>> optional;
  template <typename EnumType,
            underlying_t<EnumType> Constant = std::numeric_limits<underlying_t<EnumType>>::max()>
  struct EnumMarkableTraits
  {
    static_assert(IsEnum<EnumType>, "EnumMarkableTraits can only be used with enum types");

    using UnderlyingType = underlying_t<EnumType>;

    KRYS_NODISCARD constexpr static bool IsEmptyValue(EnumType value) noexcept
    {
      return static_cast<UnderlyingType>(value) == Constant;
    }

    KRYS_NODISCARD constexpr static EnumType EmptyValue() noexcept
    {
      return static_cast<EnumType>(Constant);
    }
  };

  template <IsEnum T>
  struct MarkableTraits<T> : public EnumMarkableTraits<T>
  {
  };

  template <typename IntegralType, IntegralType Constant>
  struct IntegralMarkableTraits
  {
    static_assert(Integral<IntegralType>);

    KRYS_NODISCARD constexpr static bool IsEmptyValue(IntegralType value) noexcept
    {
      return value == Constant;
    }

    KRYS_NODISCARD constexpr static IntegralType EmptyValue() noexcept
    {
      return Constant;
    }
  };

  template <Integral T>
  struct MarkableTraits<T> : public IntegralMarkableTraits<T, static_cast<T>(-1)>
  {
  };

  template <typename T>
  requires(FloatingPoint<T>)
  struct MarkableTraits<T>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(T value) noexcept
    {
      return std::isnan(value);
    }

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return std::numeric_limits<T>::quiet_NaN();
    }
  };

  /// @brief The goal of Markable is offering a Maybe-style type without sacrificing storage efficiency.
  /// Markable takes Traits, which should have IsEmptyValue and EmptyValue functions. By using
  /// one value of T as an empty value, we can remove the required flag in Maybe. This class is intended to be
  /// used as a member of a class to compact the size of the class. Otherwise, you should use Maybe.
  /// @note This strategy is similar to Krys::HashTable, which uses two values of T as an empty value and a
  /// deleted value.
  template <typename T, typename Traits = MarkableTraits<T>>
  class Markable
  {
  private:
    T _value;

  public:
    constexpr Markable() noexcept : _value(Traits::EmptyValue())
    {
    }

    constexpr Markable(std::nullopt_t) noexcept : Markable()
    {
    }

    constexpr Markable(T &&value) noexcept : _value(Krys::Move(value))
    {
    }

    constexpr Markable(const T &value) noexcept : _value(value)
    {
    }

    template <typename... Args>
    constexpr explicit Markable(std::in_place_t, Args &&...args) noexcept
        : _value(std::forward<Args>(args)...)
    {
    }

    constexpr Markable(const Maybe<T> &value) noexcept : _value(bool(value) ? *value : Traits::EmptyValue())
    {
    }

    constexpr Markable(Maybe<T> &&value) noexcept
        : _value(bool(value) ? Krys::Move(*value) : Traits::EmptyValue())
    {
    }

    constexpr explicit operator bool() const noexcept
    {
      return !Traits::IsEmptyValue(_value);
    }

    void reset() noexcept
    {
      _value = Traits::EmptyValue();
    }

    KRYS_NODISCARD constexpr const T &value() const & noexcept
    {
      assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &value() & noexcept
    {
      assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &&value() && noexcept
    {
      assert(bool(*this));
      return Krys::Move(_value);
    }

    KRYS_NODISCARD constexpr const T *operator->() const noexcept
    {
      assert(bool(*this));
      return std::addressof(_value);
    }

    KRYS_NODISCARD constexpr T *operator->() noexcept
    {
      assert(bool(*this));
      return std::addressof(_value);
    }

    KRYS_NODISCARD constexpr const T &operator*() const & noexcept
    {
      assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &operator*() & noexcept
    {
      assert(bool(*this));
      return _value;
    }

    template <class U>
    KRYS_NODISCARD constexpr T value_or(U &&fallback) const noexcept
    {
      if (bool(*this))
      {
        return _value;
      }

      return static_cast<T>(std::forward<U>(fallback));
    }

    KRYS_NODISCARD operator Maybe<T>() && noexcept
    {
      if (bool(*this))
      {
        return Krys::Move(_value);
      }

      return Null;
    }

    KRYS_NODISCARD operator Maybe<T>() const & noexcept
    {
      if (bool(*this))
        return _value;
      return Null;
    }

    KRYS_NODISCARD Maybe<T> AsMaybe() const noexcept
    {
      return Maybe<T>(*this);
    }
  };

  template <typename T, typename Traits>
  KRYS_NODISCARD constexpr bool operator==(const Markable<T, Traits> &x,
                                           const Markable<T, Traits> &y) noexcept
  {
    if (bool(x) != bool(y))
    {
      return false;
    }

    if (!bool(x))
    {
      return true;
    }

    return x.value() == y.value();
  }

  template <typename T, typename Traits>
  KRYS_NODISCARD constexpr bool operator==(const Markable<T, Traits> &x, const T &v) noexcept
  {
    return bool(x) && x.value() == v;
  }
}