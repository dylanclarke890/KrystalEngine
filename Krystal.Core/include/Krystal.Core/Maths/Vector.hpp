#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Traits.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <cmath>
#include <compare>
#include <functional>
#include <utility>

namespace krys
{
  /// @brief A vector of @p N components (1 to 4) of type @p T, named `x`, `y`, `z` and `w` as in shaders.
  template <Number T, size_t N>
  struct Vector;

  namespace detail
  {
    /// @brief The component of @p vector at @p index.
    template <typename TVector>
    KRYS_NODISCARD constexpr auto &VectorComponent(TVector &vector, size_t index) noexcept
    {
      constexpr size_t length = TVector::Length;
      krys_debug_assert(index < length);

      if constexpr (length > 3)
      {
        if (index == 3)
        {
          return vector.w;
        }
      }
      if constexpr (length > 2)
      {
        if (index == 2)
        {
          return vector.z;
        }
      }
      if constexpr (length > 1)
      {
        if (index == 1)
        {
          return vector.y;
        }
      }
      return vector.x;
    }
  }

  template <Number T>
  struct Vector<T, 1>
  {
    constexpr static size_t Length = 1;
    using Type = Vector<T, 1>;
    using ComponentType = T;

    T x {0};

    constexpr Vector() noexcept = default;

    constexpr explicit Vector(T value) noexcept : x(value)
    {
    }

    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 1> &other) noexcept : x(static_cast<T>(other.x))
    {
    }

    KRYS_NODISCARD constexpr T operator[](size_t index) const noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr T &operator[](size_t index) noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr auto operator<=>(const Vector &) const noexcept = default;
  };

  template <Number T>
  struct Vector<T, 2>
  {
    constexpr static size_t Length = 2;
    using Type = Vector<T, 2>;
    using ComponentType = T;

    T x {0};
    T y {0};

    constexpr Vector() noexcept = default;

    constexpr explicit Vector(T value) noexcept : x(value), y(value)
    {
    }

    constexpr Vector(T xValue, T yValue) noexcept : x(xValue), y(yValue)
    {
    }

    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 2> &other) noexcept
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
    {
    }

    KRYS_NODISCARD constexpr T operator[](size_t index) const noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr T &operator[](size_t index) noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr auto operator<=>(const Vector &) const noexcept = default;
  };

  template <Number T>
  struct Vector<T, 3>
  {
    constexpr static size_t Length = 3;
    using Type = Vector<T, 3>;
    using ComponentType = T;

    T x {0};
    T y {0};
    T z {0};

    constexpr Vector() noexcept = default;

    constexpr explicit Vector(T value) noexcept : x(value), y(value), z(value)
    {
    }

    constexpr Vector(T xValue, T yValue, T zValue) noexcept : x(xValue), y(yValue), z(zValue)
    {
    }

    constexpr Vector(const Vector<T, 2> &xy, T zValue) noexcept : x(xy.x), y(xy.y), z(zValue)
    {
    }

    /// @brief The first three components of @p xyzw.
    constexpr Vector(const Vector<T, 4> &xyzw) noexcept : x(xyzw.x), y(xyzw.y), z(xyzw.z)
    {
    }

    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 3> &other) noexcept
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z))
    {
    }

    KRYS_NODISCARD constexpr T operator[](size_t index) const noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr T &operator[](size_t index) noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr auto operator<=>(const Vector &) const noexcept = default;
  };

  template <Number T>
  struct Vector<T, 4>
  {
    constexpr static size_t Length = 4;
    using Type = Vector<T, 4>;
    using ComponentType = T;

    T x {0};
    T y {0};
    T z {0};
    T w {0};

    constexpr Vector() noexcept = default;

    constexpr explicit Vector(T value) noexcept : x(value), y(value), z(value), w(value)
    {
    }

    constexpr Vector(T xValue, T yValue, T zValue, T wValue) noexcept
        : x(xValue), y(yValue), z(zValue), w(wValue)
    {
    }

    constexpr Vector(const Vector<T, 3> &xyz, T wValue) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(wValue)
    {
    }

    constexpr Vector(const Vector<T, 2> &xy, const Vector<T, 2> &zw) noexcept
        : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
    {
    }

    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 4> &other) noexcept
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)),
          w(static_cast<T>(other.w))
    {
    }

    KRYS_NODISCARD constexpr T operator[](size_t index) const noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr T &operator[](size_t index) noexcept
    {
      return detail::VectorComponent(*this, index);
    }

    KRYS_NODISCARD constexpr auto operator<=>(const Vector &) const noexcept = default;
  };

  template <Number T, size_t N>
  struct Traits<Vector<T, N>>
  {
    using ComponentType = T;
    constexpr static bool IsVector = true;
    constexpr static bool IsMatrix = false;

    constexpr static size_t TotalComponents = N;
    constexpr static size_t Rows = N;
    constexpr static size_t Columns = 1;
  };

  template <typename T>
  concept VectorType = Traits<T>::IsVector;

#pragma region Aliases

  template <Number T>
  using Vector1 = Vector<T, 1>;
  using Vec1 = Vector<float, 1>;
  using Vec1b = Vector<bool, 1>;
  using Vec1f = Vector<float, 1>;
  using Vec1d = Vector<double, 1>;
  using Vec1i = Vector<int, 1>;
  using Vec1u = Vector<unsigned int, 1>;

  template <Number T>
  using Vector2 = Vector<T, 2>;
  using Vec2 = Vector<float, 2>;
  using Vec2b = Vector<bool, 2>;
  using Vec2f = Vector<float, 2>;
  using Vec2d = Vector<double, 2>;
  using Vec2i = Vector<int, 2>;
  using Vec2u = Vector<unsigned int, 2>;

  template <Number T>
  using Vector3 = Vector<T, 3>;
  using Vec3 = Vector<float, 3>;
  using Vec3b = Vector<bool, 3>;
  using Vec3f = Vector<float, 3>;
  using Vec3d = Vector<double, 3>;
  using Vec3i = Vector<int, 3>;
  using Vec3u = Vector<unsigned int, 3>;

  template <Number T>
  using Vector4 = Vector<T, 4>;
  using Vec4 = Vector<float, 4>;
  using Vec4b = Vector<bool, 4>;
  using Vec4f = Vector<float, 4>;
  using Vec4d = Vector<double, 4>;
  using Vec4i = Vector<int, 4>;
  using Vec4u = Vector<unsigned int, 4>;

#pragma endregion

#pragma region Component-wise algorithms

  /// @brief A vector of @p func applied to each component of @p vec.
  template <Number T, size_t N, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto MapEach(const Vector<T, N> &vec, const F &func) noexcept
  {
    using U = invoke_result_t<F, T>;
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return Vector<U, N>(func(vec[I])...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief Calls @p func with each component of @p vec, in order.
  template <Number T, size_t N, RegularCallable<T> F>
  constexpr void ForEach(const Vector<T, N> &vec, const F &func) noexcept
  {
    [&]<size_t... I>(std::index_sequence<I...>)
    {
      (func(vec[I]), ...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief A vector of @p func applied to each pair of components of @p lhs and @p rhs.
  template <Number T, size_t N, RegularCallable<T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const Vector<T, N> &lhs, const Vector<T, N> &rhs, const F &func) noexcept
  {
    using U = invoke_result_t<F, T, T>;
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return Vector<U, N>(func(lhs[I], rhs[I])...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief A vector of @p func applied to each triple of components of @p a, @p b and @p c.
  template <Number T, size_t N, RegularCallable<T, T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const Vector<T, N> &a, const Vector<T, N> &b, const Vector<T, N> &c,
                                    const F &func) noexcept
  {
    using U = invoke_result_t<F, T, T, T>;
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return Vector<U, N>(func(a[I], b[I], c[I])...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief The sum of the components of @p v.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Sum(const Vector<T, N> &v) noexcept
  {
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return (T(0) + ... + v[I]);
    }(std::make_index_sequence<N> {});
  }

  /// @brief The sum of @p func applied to each component of @p v.
  template <Number T, size_t N, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto Sum(const Vector<T, N> &v, const F &func) noexcept
  {
    using U = invoke_result_t<F, T>;
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return (U(0) + ... + func(v[I]));
    }(std::make_index_sequence<N> {});
  }

  /// @brief Whether any component of @p v satisfies @p predicate.
  template <Number T, size_t N, Predicate<T> F>
  KRYS_NODISCARD constexpr bool AnyOf(const Vector<T, N> &v, const F &predicate) noexcept
  {
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return (static_cast<bool>(predicate(v[I])) || ...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief Whether every component of @p v satisfies @p predicate.
  template <Number T, size_t N, Predicate<T> F>
  KRYS_NODISCARD constexpr bool AllOf(const Vector<T, N> &v, const F &predicate) noexcept
  {
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
      return (static_cast<bool>(predicate(v[I])) && ...);
    }(std::make_index_sequence<N> {});
  }

  /// @brief Whether no component of @p v satisfies @p predicate.
  template <Number T, size_t N, Predicate<T> F>
  KRYS_NODISCARD constexpr bool NoneOf(const Vector<T, N> &v, const F &predicate) noexcept
  {
    return !AnyOf(v, predicate);
  }

#pragma endregion

#pragma region Operators

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator+(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::plus<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator-(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::minus<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator*(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::multiplies<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator/(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::divides<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator>>(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, [](T l, T r) { return l >> r; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator<<(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, [](T l, T r) { return l << r; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator|(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_or<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator&(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_and<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator^(const Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_xor<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator+(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v + scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator-(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v - scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator*(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v * scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator/(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v / scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator>>(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v >> scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator<<(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v << scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator|(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v | scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator&(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v & scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator^(const Vector<T, N> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v ^ scalar; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator+(T scalar, const Vector<T, N> &rhs) noexcept
  {
    return rhs + scalar;
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator*(T scalar, const Vector<T, N> &rhs) noexcept
  {
    return rhs * scalar;
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator-(const Vector<T, N> &v) noexcept
  {
    return MapEach(v, std::negate<> {});
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator+(const Vector<T, N> &v) noexcept
  {
    return MapEach(v, [](T value) { return +value; });
  }

  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto operator~(const Vector<T, N> &v) noexcept
  {
    return MapEach(v, std::bit_not<> {});
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator+=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs + rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator+=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs + scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator-=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs - rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator-=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs - scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator*=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs * rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator*=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs * scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator/=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs / rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator/=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs / scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator>>=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs >> rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator>>=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs >> scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator<<=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs << rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator<<=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs << scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator|=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs | rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator|=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs | scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator&=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs & rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator&=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs & scalar;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator^=(Vector<T, N> &lhs, const Vector<T, N> &rhs) noexcept
  {
    return lhs = lhs ^ rhs;
  }

  template <Number T, size_t N>
  constexpr Vector<T, N> &operator^=(Vector<T, N> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs ^ scalar;
  }

#pragma endregion

#pragma region Geometry

  /// @brief The dot product of @p a and @p b.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Dot(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Sum(a * b);
  }

  /// @brief The length (magnitude) of @p v.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Length(const Vector<T, N> &v) noexcept
  {
    return static_cast<T>(std::sqrt(Dot(v, v)));
  }

  /// @brief The squared length of @p v, which avoids a square root.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr T LengthSquared(const Vector<T, N> &v) noexcept
  {
    return Dot(v, v);
  }

  /// @brief @p v scaled to length 1.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Normalize(const Vector<T, N> &v) noexcept
  {
    return v * static_cast<T>(1 / std::sqrt(Dot(v, v)));
  }

  /// @brief The distance between @p a and @p b.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr T Distance(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return Length(a - b);
  }

  /// @brief The squared distance between @p a and @p b.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr T DistanceSquared(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return LengthSquared(a - b);
  }

  /// @brief @p a projected onto @p b.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Project(const Vector<T, N> &a, const Vector<T, N> &b) noexcept
  {
    return b * (Dot(a, b) / Dot(b, b));
  }

  /// @brief @p v reflected across the plane with normal @p n.
  /// @param n The plane's normal, normalised.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Reflect(const Vector<T, N> &v, const Vector<T, N> &n) noexcept
  {
    return v - n * Dot(n, v) * T(2);
  }

  /// @brief @p v refracted through a surface with normal @p n.
  /// @param n The surface's normal, normalised.
  /// @param eta The ratio of the refractive indices.
  /// @return The zero vector on total internal reflection.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto Refract(const Vector<T, N> &v, const Vector<T, N> &n, T eta) noexcept
  {
    T dot = Dot(v, n);
    T k = T(1) - eta * eta * (T(1) - dot * dot);
    if (k < T(0))
    {
      return Vector<T, N>(T(0));
    }
    return eta * v - (eta * dot + static_cast<T>(std::sqrt(k))) * n;
  }

  /// @brief The cross product of @p a and @p b.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Cross(const Vector3<T> &a, const Vector3<T> &b) noexcept
  {
    return Vector3<T> {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
  }

  /// @return @p n if `Dot(nRef, i)` is negative, otherwise `-n`.
  template <Number T, size_t N>
  KRYS_NODISCARD constexpr auto FaceForward(const Vector<T, N> &n, const Vector<T, N> &i,
                                            const Vector<T, N> &nRef) noexcept
  {
    return Dot(nRef, i) < T(0) ? n : -n;
  }

  /// @brief The unit normal of the triangle @p p1, @p p2, @p p3.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto TriangleNormal(const Vector3<T> &p1, const Vector3<T> &p2,
                                               const Vector3<T> &p3) noexcept
  {
    return Normalize(Cross(p1 - p2, p1 - p3));
  }

#pragma endregion
}
