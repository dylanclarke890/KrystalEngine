#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Maths.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename T, size_t N>
  struct ColorComponents
  {
    constexpr static size_t Size = N;

    Array<T, N> Components;

    constexpr ColorComponents() noexcept : Components {}
    {
    }

    template <typename... Ts>
    constexpr ColorComponents(Ts... input) noexcept : Components {{input...}}
    {
      static_assert(sizeof...(Ts) == N);
    }

    template <typename F>
    KRYS_NODISCARD constexpr auto Map(F &&function) const noexcept
      -> ColorComponents<decltype(function(std::declval<T>())), N>;

    KRYS_NODISCARD constexpr ColorComponents Abs() const noexcept;

    template <size_t Start, size_t End>
    KRYS_NODISCARD constexpr ColorComponents<T, End - Start> Subset() const noexcept;

    constexpr ColorComponents &operator+=(const ColorComponents &) noexcept;

    constexpr ColorComponents &operator/=(T) noexcept;

    constexpr ColorComponents operator+(T) const noexcept;

    constexpr ColorComponents operator/(T) const noexcept;

    constexpr ColorComponents operator*(T) const noexcept;

    KRYS_NODISCARD constexpr T &operator[](size_t i) noexcept
    {
      return Components[i];
    }

    KRYS_NODISCARD constexpr const T &operator[](size_t i) const noexcept
    {
      return Components[i];
    }

    template <size_t I>
    KRYS_NODISCARD constexpr T get() const noexcept;

    constexpr size_t size() const noexcept
    {
      return Size;
    }
  };

  template <typename T, typename... Ts>
  ColorComponents(T, Ts...) -> ColorComponents<T, 1 + sizeof...(Ts)>;

  template <typename F, typename T, typename... Ts>
  KRYS_NODISCARD constexpr auto MapColorComponents(F &&function, T component, Ts... Components)
    -> ColorComponents<decltype(function(component[0], Components[0]...)), T::Size>
  {
    static_assert(std::conjunction_v<BoolConstant<Ts::Size == T::Size>...>,
                  "All ColorComponents passed to mapColorComponents must have the same size");

    ColorComponents<decltype(function(component[0], Components[0]...)), T::Size> result;
    for (remove_const_t<decltype(T::Size)> i = 0; i < T::Size; ++i)
    {
      result[i] = function(component[i], Components[i]...);
    }

    return result;
  }

  template <typename T, size_t N>
  template <typename F>
  constexpr auto ColorComponents<T, N>::Map(F &&function) const noexcept
    -> ColorComponents<decltype(function(std::declval<T>())), N>
  {
    return MapColorComponents(std::forward<F>(function), *this);
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> &ColorComponents<T, N>::operator+=(const ColorComponents &rhs) noexcept
  {
    *this = MapColorComponents([](T c1, T c2) { return c1 + c2; }, *this, rhs);
    return *this;
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> &ColorComponents<T, N>::operator/=(T rhs) noexcept
  {
    *this = (*this / rhs);
    return *this;
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> ColorComponents<T, N>::operator+(T rhs) const noexcept
  {
    return Map([rhs](T c) { return c + rhs; });
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> ColorComponents<T, N>::operator/(T denominator) const noexcept
  {
    return Map([denominator](T c) { return c / denominator; });
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> ColorComponents<T, N>::operator*(T factor) const noexcept
  {
    return Map([factor](T c) { return c * factor; });
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> ColorComponents<T, N>::Abs() const noexcept
  {
    return Map([](T c) { return std::abs(c); });
  }

  template <typename T, size_t N>
  template <size_t I>
  constexpr T ColorComponents<T, N>::get() const noexcept
  {
    return Components[I];
  }

  template <typename T, size_t N>
  template <size_t Start, size_t End>
  constexpr ColorComponents<T, End - Start> ColorComponents<T, N>::Subset() const noexcept
  {
    ColorComponents<T, End - Start> result;
    for (size_t i = Start; i < End; ++i)
    {
      result[i - Start] = Components[i];
    }
    return result;
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> PerComponentMax(const ColorComponents<T, N> &a,
                                                  const ColorComponents<T, N> &b) noexcept
  {
    return MapColorComponents([](T c1, T c2) { return std::max(c1, c2); }, a, b);
  }

  template <typename T, size_t N>
  constexpr ColorComponents<T, N> PerComponentMin(const ColorComponents<T, N> &a,
                                                  const ColorComponents<T, N> &b) noexcept
  {
    return MapColorComponents([](T c1, T c2) { return std::min(c1, c2); }, a, b);
  }

  template <typename T, size_t N>
  constexpr bool operator==(const ColorComponents<T, N> &a, const ColorComponents<T, N> &b) noexcept
  {
    for (size_t i = 0uz; i < N; ++i)
    {
      if (a[i] == b[i])
      {
        continue;
      }

      if (IsNaN(a[i]) && IsNaN(b[i]))
      {
        continue;
      }

      return false;
    }

    return true;
  }
}

namespace std
{
  template <typename T, size_t N>
  class tuple_size<Krys::ColorComponents<T, N>> : public std::integral_constant<size_t, N>
  {
  };

  template <size_t I, typename T, size_t N>
  class tuple_element<I, Krys::ColorComponents<T, N>>
  {
  public:
    using type = T;
  };
}