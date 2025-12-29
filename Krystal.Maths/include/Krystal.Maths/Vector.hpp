#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Traits.hpp"
#include <cassert>
#include <cmath>
#include <compare>
#include <type_traits>

#pragma region Helper Macros

#define VECTOR_TEMPLATE_PARAMS Krys::Number T, int N
#define VECTOR_TEMPLATE_ARGS T, N
#define VECTOR_TYPE Krys::Maths::Vector<VECTOR_TEMPLATE_ARGS>

#define VECTOR_TYPE_ALIASES(length)                                                                          \
  template <Number T>                                                                                        \
  using Vector##length = Vector<T, length>;                                                                  \
  using Vec##length = Vector<float, length>;                                                                 \
  using Vec##length##b = Vector<bool, length>;                                                               \
  using Vec##length##f = Vector<float, length>;                                                              \
  using Vec##length##d = Vector<double, length>;                                                             \
  using Vec##length##i = Vector<int, length>;                                                                \
  using Vec##length##u = Vector<unsigned int, length>;

#define VECTOR_INTROSPECTION(length)                                                                         \
  static constexpr int Length = length;                                                                      \
  using Type = Vector<T, Length>;                                                                            \
  using ComponentType = T;

#define VECTOR_BINARY_OPERATOR(OP)                                                                           \
  template <VECTOR_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const Vector<T, N> &lhs,                           \
                                                          const Vector<T, N> &rhs) noexcept                  \
  {                                                                                                          \
    return Zip(lhs, rhs, [](auto l, auto r) { return l OP r; });                                             \
  }                                                                                                          \
  template <VECTOR_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const Vector<T, N> &lhs, T scalar) noexcept        \
  {                                                                                                          \
    return MapEach(lhs, [&](auto v) { return v OP scalar; });                                                \
  }

#define VECTOR_UNARY_OPERATOR(OP)                                                                            \
  template <VECTOR_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const Vector<T, N> &rhs) noexcept                  \
  {                                                                                                          \
    return MapEach(rhs, [](auto v) { return OP v; });                                                        \
  }

#define VECTOR_OPERATOR_ASSIGNMENT(OP)                                                                       \
  constexpr auto &KRYS_CONCAT(operator, KRYS_CONCAT(OP, =))(const Vector &other) noexcept                    \
  {                                                                                                          \
    *this = *this OP other;                                                                                  \
    return *this;                                                                                            \
  }                                                                                                          \
  constexpr auto &KRYS_CONCAT(operator, KRYS_CONCAT(OP, =))(T scalar) noexcept                               \
  {                                                                                                          \
    *this = *this OP scalar;                                                                                 \
    return *this;                                                                                            \
  }

#define VECTOR_COMMON_MEMBER_FUNCTIONS()                                                                     \
  constexpr ~Vector() noexcept = default;                                                                    \
  constexpr Vector(const Vector &other) noexcept = default;                                                  \
  constexpr Vector(Vector &&other) noexcept = default;                                                       \
  constexpr Vector &operator=(const Vector &other) noexcept = default;                                       \
  constexpr Vector &operator=(Vector &&other) noexcept = default;                                            \
  constexpr auto operator<=>(const Vector &other) const noexcept = default;                                  \
  VECTOR_OPERATOR_ASSIGNMENT(+)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(-)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(*)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(/)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(>>)                                                                             \
  VECTOR_OPERATOR_ASSIGNMENT(<<)                                                                             \
  VECTOR_OPERATOR_ASSIGNMENT(|)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(&)                                                                              \
  VECTOR_OPERATOR_ASSIGNMENT(^)

#pragma endregion

namespace Krys::Maths
{
  template <VECTOR_TEMPLATE_PARAMS>
  struct Vector;

  template <Number T>
  struct Vector<T, 1>
  {
    T x;

    VECTOR_INTROSPECTION(1);

    constexpr Vector() noexcept : x(T(0))
    {
    }
    constexpr explicit Vector(T value) noexcept : x(value)
    {
    }
    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 1> &other) noexcept : x(static_cast<T>(other.x))
    {
    }

    KRYS_NODISCARD constexpr auto operator[](int index) const noexcept
    {
      assert(index == 0);
      return x;
    }

    KRYS_NODISCARD constexpr auto &operator[](int index) noexcept
    {
      assert(index == 0);
      return x;
    }

    VECTOR_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Vector<T, 2>
  {
    T x, y;

    VECTOR_INTROSPECTION(2);

    constexpr Vector() noexcept : x(T(0)), y(T(0))
    {
    }
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

    KRYS_NODISCARD constexpr auto operator[](int index) const noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else
        return y;
    }

    KRYS_NODISCARD constexpr auto &operator[](int index) noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else
        return y;
    }

    VECTOR_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Vector<T, 3>
  {
    T x, y, z;

    VECTOR_INTROSPECTION(3);

    constexpr Vector() noexcept : x(T(0)), y(T(0)), z(T(0))
    {
    }

    constexpr explicit Vector(T value) noexcept : x(value), y(value), z(value)
    {
    }

    constexpr Vector(T xValue, T yValue, T zValue) noexcept : x(xValue), y(yValue), z(zValue)
    {
    }

    constexpr Vector(const Vector<T, 2> &vec2, T zValue) noexcept : x(vec2.x), y(vec2.y), z(zValue)
    {
    }

    constexpr Vector(const Vector<T, 4> &vec4) noexcept : x(vec4.x), y(vec4.y), z(vec4.z)
    {
    }

    template <Number U>
    constexpr Vector(const Vector<U, 3> &other) noexcept
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z))
    {
    }

    KRYS_NODISCARD constexpr auto operator[](int index) const noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else if (index == 1)
        return y;
      else
        return z;
    }

    KRYS_NODISCARD constexpr auto &operator[](int index) noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else if (index == 1)
        return y;
      else
        return z;
    }

    VECTOR_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Vector<T, 4>
  {
    T x, y, z, w;

    VECTOR_INTROSPECTION(4);

    constexpr Vector() noexcept : x(T(0)), y(T(0)), z(T(0)), w(T(0))
    {
    }

    constexpr explicit Vector(T value) noexcept : x(value), y(value), z(value), w(value)
    {
    }

    constexpr Vector(T xValue, T yValue, T zValue, T wValue) noexcept
        : x(xValue), y(yValue), z(zValue), w(wValue)
    {
    }

    constexpr Vector(const Vector<T, 3> &vec3, T wValue) noexcept : x(vec3.x), y(vec3.y), z(vec3.z), w(wValue)
    {
    }

    constexpr Vector(const Vector<T, 2> &vec2a, const Vector<T, 2> &vec2b) noexcept
        : x(vec2a.x), y(vec2a.y), z(vec2b.x), w(vec2b.y)
    {
    }

    template <ConvertibleTo<T> U>
    constexpr Vector(const Vector<U, 4> &other) noexcept
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)),
          w(static_cast<T>(other.w))
    {
    }

    KRYS_NODISCARD constexpr auto operator[](int index) const noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else if (index == 1)
        return y;
      else if (index == 2)
        return z;
      else
        return w;
    }

    KRYS_NODISCARD constexpr auto &operator[](int index) noexcept
    {
      assert(index >= 0 && index < Length);
      if (index == 0)
        return x;
      else if (index == 1)
        return y;
      else if (index == 2)
        return z;
      else
        return w;
    }

    VECTOR_COMMON_MEMBER_FUNCTIONS()
  };

  template <VECTOR_TEMPLATE_PARAMS>
  struct Traits<VECTOR_TYPE>
  {
    using ComponentType = T;
    static constexpr bool IsVector = true;
    static constexpr bool IsMatrix = false;

    static constexpr int TotalComponents = N;
    static constexpr int Rows = N;
    static constexpr int Columns = 1;
  };

  template <typename T>
  concept VectorType = Traits<T>::IsVector;

  VECTOR_TYPE_ALIASES(1);
  VECTOR_TYPE_ALIASES(2);
  VECTOR_TYPE_ALIASES(3);
  VECTOR_TYPE_ALIASES(4);

  /// @brief Map each component of the vector using the provided function.
  template <VECTOR_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto MapEach(const VECTOR_TYPE &vec, const F &func) noexcept
  {
    static_assert(N > 0 && N < 5, "Unsupported vector length");

    using U = std::invoke_result_t<F, T>;
    using Vec = Vector<U, N>;

    if constexpr (N == 1)
    {
      auto x = func(vec.x);
      return Vec(x);
    }
    else if constexpr (N == 2)
    {
      auto x = func(vec.x);
      auto y = func(vec.y);
      return Vec(x, y);
    }
    else if constexpr (N == 3)
    {
      auto x = func(vec.x);
      auto y = func(vec.y);
      auto z = func(vec.z);
      return Vec(x, y, z);
    }
    else if constexpr (N == 4)
    {
      auto x = func(vec.x);
      auto y = func(vec.y);
      auto z = func(vec.z);
      auto w = func(vec.w);
      return Vec(x, y, z, w);
    }
    else
    {
      static_assert(false, "Unsupported vector length");
    }
  }

  /// @brief Apply a function to each component of the vector.
  template <VECTOR_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr void ForEach(const VECTOR_TYPE &vec, const F &func) noexcept
  {
    static_assert(N > 0 && N < 5, "Unsupported vector length");

    if constexpr (N == 1)
    {
      func(vec.x);
    }
    else if constexpr (N == 2)
    {
      func(vec.x);
      func(vec.y);
    }
    else if constexpr (N == 3)
    {
      func(vec.x);
      func(vec.y);
      func(vec.z);
    }
    else if constexpr (N == 4)
    {
      func(vec.x);
      func(vec.y);
      func(vec.z);
      func(vec.w);
    }
    else
    {
      static_assert(false, "Unsupported vector length");
    }
  }

  /// @brief Zip two vectors together using the provided function.
  template <VECTOR_TEMPLATE_PARAMS, RegularCallable<T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const VECTOR_TYPE &lhs, const Vector<T, N> &rhs, const F &func) noexcept
  {
    static_assert(N > 0 && N < 5, "Unsupported vector length");

    using U = std::invoke_result_t<F, T, T>;
    using Vec = Vector<U, N>;

    if constexpr (N == 1)
    {
      auto x = func(lhs.x, rhs.x);
      return Vec(x);
    }
    else if constexpr (N == 2)
    {
      auto x = func(lhs.x, rhs.x);
      auto y = func(lhs.y, rhs.y);
      return Vec(x, y);
    }
    else if constexpr (N == 3)
    {
      auto x = func(lhs.x, rhs.x);
      auto y = func(lhs.y, rhs.y);
      auto z = func(lhs.z, rhs.z);
      return Vec(x, y, z);
    }
    else if constexpr (N == 4)
    {
      auto x = func(lhs.x, rhs.x);
      auto y = func(lhs.y, rhs.y);
      auto z = func(lhs.z, rhs.z);
      auto w = func(lhs.w, rhs.w);
      return Vec(x, y, z, w);
    }
    else
    {
      static_assert(false, "Unsupported vector length");
    }
  }

  /// @brief Zip three vectors together using the provided function.
  template <VECTOR_TEMPLATE_PARAMS, RegularCallable<T, T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const VECTOR_TYPE &a, const VECTOR_TYPE &b, const VECTOR_TYPE &c,
                                    const F &func) noexcept
  {
    static_assert(N > 0 && N < 5, "Unsupported vector length");

    using U = std::invoke_result_t<F, T, T, T>;
    using Vec = Vector<U, N>;

    if constexpr (N == 1)
    {
      auto x = func(a.x, b.x, c.x);
      return Vec(x);
    }
    else if constexpr (N == 2)
    {
      auto x = func(a.x, b.x, c.x);
      auto y = func(a.y, b.y, c.y);
      return Vec(x, y);
    }
    else if constexpr (N == 3)
    {
      auto x = func(a.x, b.x, c.x);
      auto y = func(a.y, b.y, c.y);
      auto z = func(a.z, b.z, c.z);
      return Vec(x, y, z);
    }
    else if constexpr (N == 4)
    {
      auto x = func(a.x, b.x, c.x);
      auto y = func(a.y, b.y, c.y);
      auto z = func(a.z, b.z, c.z);
      auto w = func(a.w, b.w, c.w);
      return Vec(x, y, z, w);
    }
    else
    {
      static_assert(false, "Unsupported vector length");
    }
  }

  /// @brief Computes the sum of all components of the vector.
  template <VECTOR_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Sum(const VECTOR_TYPE &v) noexcept
  {
    T sum = T(0);
    ForEach(v, [&sum](auto val) { sum += val; });
    return sum;
  }

  /// @brief Computes the sum of all components of the vector after applying a function to each component.
  template <VECTOR_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto Sum(const VECTOR_TYPE &v, const F &func) noexcept
  {
    using U = std::invoke_result_t<F, T>;

    U sum = U(0);
    ForEach(v, [&sum, &func](auto val) { sum += func(val); });
    return sum;
  }

  /// @brief Checks if any components satisfy the predicate.
  template <VECTOR_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool AnyOf(const VECTOR_TYPE &v, const Func &predicate) noexcept
  {
    if constexpr (N == 1)
      return predicate(v.x);
    else if constexpr (N == 2)
      return predicate(v.x) || predicate(v.y);
    else if constexpr (N == 3)
      return predicate(v.x) || predicate(v.y) || predicate(v.z);
    else if constexpr (N == 4)
      return predicate(v.x) || predicate(v.y) || predicate(v.z) || predicate(v.w);
    else
      static_assert(false, "Unsupported number of components.");
  }

  /// @brief Checks if all of the components in `v` satisfy the predicate.
  template <VECTOR_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool AllOf(const VECTOR_TYPE &v, const Func &predicate) noexcept
  {
    if constexpr (N == 1)
      return predicate(v.x);
    else if constexpr (N == 2)
      return predicate(v.x) && predicate(v.y);
    else if constexpr (N == 3)
      return predicate(v.x) && predicate(v.y) && predicate(v.z);
    else if constexpr (N == 4)
      return predicate(v.x) && predicate(v.y) && predicate(v.z) && predicate(v.w);
    else
      static_assert(false, "Unsupported number of components.");
  }

  /// @brief Checks if none of the components in `v` satisfy the predicate.
  template <VECTOR_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool NoneOf(const VECTOR_TYPE &v, const Func &predicate) noexcept
  {
    if constexpr (N == 1)
      return !predicate(v.x);
    else if constexpr (N == 2)
      return !predicate(v.x) && !predicate(v.y);
    else if constexpr (N == 3)
      return !predicate(v.x) && !predicate(v.y) && !predicate(v.z);
    else if constexpr (N == 4)
      return !predicate(v.x) && !predicate(v.y) && !predicate(v.z) && !predicate(v.w);
    else
      static_assert(false, "Unsupported number of components.");
  }

  VECTOR_BINARY_OPERATOR(+);
  VECTOR_BINARY_OPERATOR(-);
  VECTOR_BINARY_OPERATOR(*);
  VECTOR_BINARY_OPERATOR(/);
  VECTOR_BINARY_OPERATOR(>>);
  VECTOR_BINARY_OPERATOR(<<);
  VECTOR_BINARY_OPERATOR(|);
  VECTOR_BINARY_OPERATOR(&);
  VECTOR_BINARY_OPERATOR(^);

  VECTOR_UNARY_OPERATOR(-);
  VECTOR_UNARY_OPERATOR(+);
  VECTOR_UNARY_OPERATOR(~);

  template <VECTOR_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto operator+(T scalar, const Vector<T, N> &rhs) noexcept
  {
    return rhs + scalar;
  }

  template <VECTOR_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto operator*(T scalar, const Vector<T, N> &rhs) noexcept
  {
    return rhs * scalar;
  }

  /// @brief Computes the dot product of two vectors.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Dot(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Sum(Zip(a, b, [](auto l, auto r) { return l * r; }));
  }

  /// @brief Computes the length (magnitude) of a vector.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Length(const VECTOR_TYPE &x) noexcept
  {
    return static_cast<T>(std::sqrt(Dot(x, x)));
  }

  /// @brief Computes the squared length (magnitude) of a vector.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr T LengthSquared(const VECTOR_TYPE &x) noexcept
  {
    return Dot(x, x);
  }

  /// @brief Normalizes a vector (scales it to length 1).
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Normalize(const VECTOR_TYPE &x) noexcept
  {
    return x * (1.f / std::sqrt(Dot(x, x)));
  }

  /// @brief Computes the distance between two vectors.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr T Distance(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return Length(a - b);
  }

  /// @brief Computes the squared distance between two vectors.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr T DistanceSquared(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return LengthSquared(a - b);
  }

  /// @brief Projects vector `a` onto vector `b`.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Project(const VECTOR_TYPE &a, const VECTOR_TYPE &b) noexcept
  {
    return b * (Dot(a, b) / Dot(b, b));
  }

  /// @brief Reflects vector `v` across the normal `n`.
  /// @param v The input vector to reflect.
  /// @param n The normal vector to reflect across (should be normalized).
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Reflect(const VECTOR_TYPE &v, const VECTOR_TYPE &n) noexcept
  {
    return v - n * Dot(n, v) * T(2);
  }

  /// @brief Computes the refraction of vector `v` given a normal `n` and a refraction index.
  /// @param v The incident vector.
  /// @param n The normal vector (should be normalized).
  /// @param eta The refraction index.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto Refract(const VECTOR_TYPE &v, const VECTOR_TYPE &n, T eta) noexcept
  {
    T dot = Dot(v, n);
    T k = T(1) - eta * eta * (T(1) - dot * dot);
    if (k < T(0))
      return VECTOR_TYPE(0); // Total internal reflection
    return eta * v - (eta * dot + std::sqrt(k)) * n;
  }

  /// @brief Computes the cross product of two 3D vectors.
  template <FloatingPoint T>
  constexpr auto Cross(const Vector3<T> &a, const Vector3<T> &b) noexcept
  {
    return Vector3<T> {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
  }

  /// @return `n` if `Dot(nRef, i)` < `0.0`, otherwise `-n`.
  template <VECTOR_TEMPLATE_PARAMS>
  constexpr auto FaceForward(const VECTOR_TYPE &n, const VECTOR_TYPE &i, const VECTOR_TYPE &nRef) noexcept
  {
    return Dot(nRef, i) < static_cast<T>(0) ? n : -n;
  }

  /// @brief Compute the normal of a triangle.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto TriangleNormal(const Vector3<T> &p1, const Vector3<T> &p2,
                                               const Vector3<T> &p3) noexcept
  {
    return Normalize(Cross(p1 - p2, p1 - p3));
  }

#undef VECTOR_TYPE_ALIASES
#undef VECTOR_INTROSPECTION
#undef VECTOR_BINARY_OPERATOR
#undef VECTOR_UNARY_OPERATOR
#undef VECTOR_OPERATOR_ASSIGNMENT
#undef VECTOR_COMMON_MEMBER_FUNCTIONS
}
