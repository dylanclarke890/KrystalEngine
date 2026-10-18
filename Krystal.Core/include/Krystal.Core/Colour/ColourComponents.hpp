#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys
{
  template <typename T, size_t N>
  struct ColourComponents
  {
    constexpr static size_t Size = N;

    Array<T, N> Components;

    constexpr ColourComponents() noexcept : Components {}
    {
    }

    template <typename... Ts>
    constexpr ColourComponents(Ts... input) noexcept : Components {{input...}}
    {
      static_assert(sizeof...(Ts) == N);
    }

    template <typename F>
    KRYS_NODISCARD constexpr auto Map(F &&function) const noexcept
      -> ColourComponents<decltype(function(std::declval<T>())), N>;

    KRYS_NODISCARD constexpr ColourComponents Abs() const noexcept;

    template <size_t Start, size_t End>
    KRYS_NODISCARD constexpr ColourComponents<T, End - Start> Subset() const noexcept;

    constexpr ColourComponents &operator+=(const ColourComponents &) noexcept;

    constexpr ColourComponents &operator/=(T) noexcept;

    constexpr ColourComponents operator+(T) const noexcept;

    constexpr ColourComponents operator/(T) const noexcept;

    constexpr ColourComponents operator*(T) const noexcept;

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
  ColourComponents(T, Ts...) -> ColourComponents<T, 1 + sizeof...(Ts)>;

  template <typename F, typename T, typename... Ts>
  KRYS_NODISCARD constexpr auto MapColourComponents(F &&function, T component, Ts... Components)
    -> ColourComponents<decltype(function(component[0], Components[0]...)), T::Size>
  {
    static_assert(std::conjunction_v<BoolConstant<Ts::Size == T::Size>...>,
                  "All ColourComponents passed to mapColorComponents must have the same size");

    ColourComponents<decltype(function(component[0], Components[0]...)), T::Size> result;
    for (remove_const_t<decltype(T::Size)> i = 0; i < T::Size; ++i)
    {
      result[i] = function(component[i], Components[i]...);
    }

    return result;
  }

  template <typename T, size_t N>
  template <typename F>
  constexpr auto ColourComponents<T, N>::Map(F &&function) const noexcept
    -> ColourComponents<decltype(function(std::declval<T>())), N>
  {
    return MapColourComponents(std::forward<F>(function), *this);
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> &ColourComponents<T, N>::operator+=(const ColourComponents &rhs) noexcept
  {
    *this = MapColourComponents([](T c1, T c2) { return c1 + c2; }, *this, rhs);
    return *this;
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> &ColourComponents<T, N>::operator/=(T rhs) noexcept
  {
    *this = (*this / rhs);
    return *this;
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> ColourComponents<T, N>::operator+(T rhs) const noexcept
  {
    return Map([rhs](T c) { return c + rhs; });
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> ColourComponents<T, N>::operator/(T denominator) const noexcept
  {
    return Map([denominator](T c) { return c / denominator; });
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> ColourComponents<T, N>::operator*(T factor) const noexcept
  {
    return Map([factor](T c) { return c * factor; });
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> ColourComponents<T, N>::Abs() const noexcept
  {
    return Map([](T c) { return std::abs(c); });
  }

  template <typename T, size_t N>
  template <size_t I>
  constexpr T ColourComponents<T, N>::get() const noexcept
  {
    return Components[I];
  }

  template <typename T, size_t N>
  template <size_t Start, size_t End>
  constexpr ColourComponents<T, End - Start> ColourComponents<T, N>::Subset() const noexcept
  {
    ColourComponents<T, End - Start> result;
    for (size_t i = Start; i < End; ++i)
    {
      result[i - Start] = Components[i];
    }
    return result;
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> PerComponentMax(const ColourComponents<T, N> &a,
                                                   const ColourComponents<T, N> &b) noexcept
  {
    return MapColourComponents([](T c1, T c2) { return std::max(c1, c2); }, a, b);
  }

  template <typename T, size_t N>
  constexpr ColourComponents<T, N> PerComponentMin(const ColourComponents<T, N> &a,
                                                   const ColourComponents<T, N> &b) noexcept
  {
    return MapColourComponents([](T c1, T c2) { return std::min(c1, c2); }, a, b);
  }

  template <typename T, size_t N>
  constexpr bool operator==(const ColourComponents<T, N> &a, const ColourComponents<T, N> &b) noexcept
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
  class tuple_size<krys::ColourComponents<T, N>> : public std::integral_constant<size_t, N>
  {
  };

  template <size_t I, typename T, size_t N>
  class tuple_element<I, krys::ColourComponents<T, N>>
  {
  public:
    using type = T;
  };
}