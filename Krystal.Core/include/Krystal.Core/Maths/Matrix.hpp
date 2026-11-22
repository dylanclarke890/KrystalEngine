#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Traits.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <compare>
#include <functional>
#include <utility>

namespace krys
{
  /// @brief A matrix of @p C columns and @p R rows (each 2 to 4) of type @p T, stored column by column:
  /// `m[column][row]`.
  template <Number T, int C, int R>
  struct Matrix
  {
    static_assert(C >= 2 && C <= 4 && R >= 2 && R <= 4, "Matrices have 2 to 4 columns and 2 to 4 rows.");

    constexpr static int Columns = C;
    constexpr static int Rows = R;
    using Type = Matrix<T, C, R>;
    using ComponentType = T;
    using RowType = Vector<T, C>;
    using ColumnType = Vector<T, R>;
    using TransposeType = Matrix<T, R, C>;

  private:
    ColumnType _data[C] {};

  public:
    /// @brief The zero matrix.
    constexpr Matrix() noexcept = default;

    /// @brief A matrix with every component set to @p value.
    constexpr explicit Matrix(T value) noexcept
    {
      for (auto &column : _data)
      {
        column = ColumnType(value);
      }
    }

    constexpr Matrix(const ColumnType &column0, const ColumnType &column1) noexcept
    requires(C == 2)
        : _data {column0, column1}
    {
    }

    constexpr Matrix(const ColumnType &column0, const ColumnType &column1, const ColumnType &column2) noexcept
    requires(C == 3)
        : _data {column0, column1, column2}
    {
    }

    constexpr Matrix(const ColumnType &column0, const ColumnType &column1, const ColumnType &column2,
                     const ColumnType &column3) noexcept
    requires(C == 4)
        : _data {column0, column1, column2, column3}
    {
    }

    /// @brief A matrix of @p values, column by column.
    template <ConvertibleTo<T>... TValues>
    requires(sizeof...(TValues) == C * R)
    constexpr Matrix(TValues... values) noexcept
    {
      const T flat[] {static_cast<T>(values)...};
      for (int column = 0; column < C; ++column)
      {
        for (int row = 0; row < R; ++row)
        {
          _data[column][row] = flat[column * R + row];
        }
      }
    }

    /// @brief The upper-left 3x3 of @p other.
    // A template, so that in Matrix<T, 4, 4> it isn't a (constrained-away) copy constructor.
    template <SameType<Matrix<T, 4, 4>> TOther>
    requires(C == 3 && R == 3)
    constexpr Matrix(const TOther &other) noexcept
        : _data {ColumnType(other[0]), ColumnType(other[1]), ColumnType(other[2])}
    {
    }

    KRYS_NODISCARD constexpr ColumnType &operator[](int column) noexcept
    {
      krys_debug_assert(column >= 0 && column < C);
      return _data[column];
    }

    KRYS_NODISCARD constexpr const ColumnType &operator[](int column) const noexcept
    {
      krys_debug_assert(column >= 0 && column < C);
      return _data[column];
    }

    KRYS_NODISCARD constexpr auto operator<=>(const Matrix &) const noexcept = default;
  };

  template <Number T, int C, int R>
  struct Traits<Matrix<T, C, R>>
  {
    using ComponentType = T;
    constexpr static bool IsVector = false;
    constexpr static bool IsMatrix = true;

    constexpr static int TotalComponents = C * R;
    constexpr static int Columns = C;
    constexpr static int Rows = R;
  };

  template <typename T>
  concept MatrixType = Traits<T>::IsMatrix;

#pragma region Aliases

  template <Number T>
  using Matrix2x2 = Matrix<T, 2, 2>;
  using Mat2x2 = Matrix<float, 2, 2>;
  using Mat2x2b = Matrix<bool, 2, 2>;
  using Mat2x2f = Matrix<float, 2, 2>;
  using Mat2x2d = Matrix<double, 2, 2>;
  using Mat2x2i = Matrix<int, 2, 2>;
  using Mat2x2u = Matrix<unsigned int, 2, 2>;

  template <Number T>
  using Matrix2x3 = Matrix<T, 2, 3>;
  using Mat2x3 = Matrix<float, 2, 3>;
  using Mat2x3b = Matrix<bool, 2, 3>;
  using Mat2x3f = Matrix<float, 2, 3>;
  using Mat2x3d = Matrix<double, 2, 3>;
  using Mat2x3i = Matrix<int, 2, 3>;
  using Mat2x3u = Matrix<unsigned int, 2, 3>;

  template <Number T>
  using Matrix2x4 = Matrix<T, 2, 4>;
  using Mat2x4 = Matrix<float, 2, 4>;
  using Mat2x4b = Matrix<bool, 2, 4>;
  using Mat2x4f = Matrix<float, 2, 4>;
  using Mat2x4d = Matrix<double, 2, 4>;
  using Mat2x4i = Matrix<int, 2, 4>;
  using Mat2x4u = Matrix<unsigned int, 2, 4>;

  template <Number T>
  using Matrix3x2 = Matrix<T, 3, 2>;
  using Mat3x2 = Matrix<float, 3, 2>;
  using Mat3x2b = Matrix<bool, 3, 2>;
  using Mat3x2f = Matrix<float, 3, 2>;
  using Mat3x2d = Matrix<double, 3, 2>;
  using Mat3x2i = Matrix<int, 3, 2>;
  using Mat3x2u = Matrix<unsigned int, 3, 2>;

  template <Number T>
  using Matrix3x3 = Matrix<T, 3, 3>;
  using Mat3x3 = Matrix<float, 3, 3>;
  using Mat3x3b = Matrix<bool, 3, 3>;
  using Mat3x3f = Matrix<float, 3, 3>;
  using Mat3x3d = Matrix<double, 3, 3>;
  using Mat3x3i = Matrix<int, 3, 3>;
  using Mat3x3u = Matrix<unsigned int, 3, 3>;

  template <Number T>
  using Matrix3x4 = Matrix<T, 3, 4>;
  using Mat3x4 = Matrix<float, 3, 4>;
  using Mat3x4b = Matrix<bool, 3, 4>;
  using Mat3x4f = Matrix<float, 3, 4>;
  using Mat3x4d = Matrix<double, 3, 4>;
  using Mat3x4i = Matrix<int, 3, 4>;
  using Mat3x4u = Matrix<unsigned int, 3, 4>;

  template <Number T>
  using Matrix4x2 = Matrix<T, 4, 2>;
  using Mat4x2 = Matrix<float, 4, 2>;
  using Mat4x2b = Matrix<bool, 4, 2>;
  using Mat4x2f = Matrix<float, 4, 2>;
  using Mat4x2d = Matrix<double, 4, 2>;
  using Mat4x2i = Matrix<int, 4, 2>;
  using Mat4x2u = Matrix<unsigned int, 4, 2>;

  template <Number T>
  using Matrix4x3 = Matrix<T, 4, 3>;
  using Mat4x3 = Matrix<float, 4, 3>;
  using Mat4x3b = Matrix<bool, 4, 3>;
  using Mat4x3f = Matrix<float, 4, 3>;
  using Mat4x3d = Matrix<double, 4, 3>;
  using Mat4x3i = Matrix<int, 4, 3>;
  using Mat4x3u = Matrix<unsigned int, 4, 3>;

  template <Number T>
  using Matrix4x4 = Matrix<T, 4, 4>;
  using Mat4x4 = Matrix<float, 4, 4>;
  using Mat4x4b = Matrix<bool, 4, 4>;
  using Mat4x4f = Matrix<float, 4, 4>;
  using Mat4x4d = Matrix<double, 4, 4>;
  using Mat4x4i = Matrix<int, 4, 4>;
  using Mat4x4u = Matrix<unsigned int, 4, 4>;

  using Mat2 = Matrix<float, 2, 2>;
  using Mat3 = Matrix<float, 3, 3>;
  using Mat4 = Matrix<float, 4, 4>;

#pragma endregion

  /// @brief The identity matrix of type @p TMatrix, which must be square.
  template <MatrixType TMatrix>
  KRYS_NODISCARD constexpr TMatrix Identity() noexcept
  {
    static_assert(TMatrix::Columns == TMatrix::Rows, "Only square matrices have an identity.");

    TMatrix identity;
    for (int i = 0; i < TMatrix::Columns; ++i)
    {
      identity[i][i] = typename TMatrix::ComponentType(1);
    }
    return identity;
  }

#pragma region Component-wise algorithms

  /// @brief A matrix of @p func applied to each component of @p m.
  template <Number T, int C, int R, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto MapEach(const Matrix<T, C, R> &m, const F &func) noexcept
  {
    using U = invoke_result_t<F, T>;
    return [&]<int... I>(std::integer_sequence<int, I...>)
    {
      return Matrix<U, C, R>(MapEach(m[I], func)...);
    }(std::make_integer_sequence<int, C> {});
  }

  /// @brief Calls @p func with each component of @p m, column by column.
  template <Number T, int C, int R, RegularCallable<T> F>
  constexpr void ForEach(const Matrix<T, C, R> &m, const F &func) noexcept
  {
    for (int column = 0; column < C; ++column)
    {
      ForEach(m[column], func);
    }
  }

  /// @brief A matrix of @p func applied to each pair of components of @p lhs and @p rhs.
  template <Number T, int C, int R, RegularCallable<T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs,
                                    const F &func) noexcept
  {
    using U = invoke_result_t<F, T, T>;
    return [&]<int... I>(std::integer_sequence<int, I...>)
    {
      return Matrix<U, C, R>(Zip(lhs[I], rhs[I], func)...);
    }(std::make_integer_sequence<int, C> {});
  }

  /// @brief A matrix of @p func applied to each triple of components of @p a, @p b and @p c.
  template <Number T, int C, int R, RegularCallable<T, T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const Matrix<T, C, R> &a, const Matrix<T, C, R> &b,
                                    const Matrix<T, C, R> &c, const F &func) noexcept
  {
    using U = invoke_result_t<F, T, T, T>;
    return [&]<int... I>(std::integer_sequence<int, I...>)
    {
      return Matrix<U, C, R>(Zip(a[I], b[I], c[I], func)...);
    }(std::make_integer_sequence<int, C> {});
  }

  /// @brief The sum of the components of @p m.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr T Sum(const Matrix<T, C, R> &m) noexcept
  {
    T sum = T(0);
    ForEach(m, [&sum](T value) { sum += value; });
    return sum;
  }

  /// @brief The sum of @p func applied to each component of @p m.
  template <Number T, int C, int R, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto Sum(const Matrix<T, C, R> &m, const F &func) noexcept
  {
    using U = invoke_result_t<F, T>;

    U sum = U(0);
    ForEach(m, [&sum, &func](T value) { sum += func(value); });
    return sum;
  }

  /// @brief Whether any component of @p m satisfies @p predicate.
  template <Number T, int C, int R, Predicate<T> F>
  KRYS_NODISCARD constexpr bool AnyOf(const Matrix<T, C, R> &m, const F &predicate) noexcept
  {
    for (int column = 0; column < C; ++column)
    {
      if (AnyOf(m[column], predicate))
      {
        return true;
      }
    }
    return false;
  }

  /// @brief Whether every component of @p m satisfies @p predicate.
  template <Number T, int C, int R, Predicate<T> F>
  KRYS_NODISCARD constexpr bool AllOf(const Matrix<T, C, R> &m, const F &predicate) noexcept
  {
    for (int column = 0; column < C; ++column)
    {
      if (!AllOf(m[column], predicate))
      {
        return false;
      }
    }
    return true;
  }

  /// @brief Whether no component of @p m satisfies @p predicate.
  template <Number T, int C, int R, Predicate<T> F>
  KRYS_NODISCARD constexpr bool NoneOf(const Matrix<T, C, R> &m, const F &predicate) noexcept
  {
    return !AnyOf(m, predicate);
  }

#pragma endregion

#pragma region Operators

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator+(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::plus<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator-(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::minus<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator/(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::divides<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator>>(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, [](T l, T r) { return l >> r; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator<<(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, [](T l, T r) { return l << r; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator|(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_or<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator&(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_and<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator^(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::bit_xor<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator+(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v + scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator-(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v - scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator*(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v * scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator/(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v / scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator>>(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v >> scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator<<(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v << scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator|(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v | scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator&(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v & scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator^(const Matrix<T, C, R> &lhs, T scalar) noexcept
  {
    return MapEach(lhs, [scalar](T v) { return v ^ scalar; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator+(T scalar, const Matrix<T, C, R> &rhs) noexcept
  {
    return rhs + scalar;
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator*(T scalar, const Matrix<T, C, R> &rhs) noexcept
  {
    return rhs * scalar;
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator-(const Matrix<T, C, R> &m) noexcept
  {
    return MapEach(m, std::negate<> {});
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator+(const Matrix<T, C, R> &m) noexcept
  {
    return MapEach(m, [](T value) { return +value; });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator~(const Matrix<T, C, R> &m) noexcept
  {
    return MapEach(m, std::bit_not<> {});
  }

  /// @brief The matrix product of @p lhs and @p rhs. For the component-wise product, use `Hadamard`.
  template <Number T, int C, int R, int P>
  KRYS_NODISCARD constexpr auto operator*(const Matrix<T, C, R> &lhs, const Matrix<T, P, C> &rhs) noexcept
  {
    Matrix<T, P, R> product;
    for (int column = 0; column < P; ++column)
    {
      for (int row = 0; row < R; ++row)
      {
        for (int k = 0; k < C; ++k)
        {
          product[column][row] += lhs[k][row] * rhs[column][k];
        }
      }
    }
    return product;
  }

  /// @brief @p v transformed by @p m.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto operator*(const Matrix<T, C, R> &m, const Vector<T, C> &v) noexcept
  {
    Vector<T, R> result;
    for (int row = 0; row < R; ++row)
    {
      for (int k = 0; k < C; ++k)
      {
        result[row] += m[k][row] * v[k];
      }
    }
    return result;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator+=(Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return lhs = lhs + rhs;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator-=(Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return lhs = lhs - rhs;
  }

  /// @brief Multiplies @p lhs by @p rhs (the matrix product), which must be square.
  template <Number T, int N>
  constexpr Matrix<T, N, N> &operator*=(Matrix<T, N, N> &lhs, const Matrix<T, N, N> &rhs) noexcept
  {
    return lhs = lhs * rhs;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator/=(Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return lhs = lhs / rhs;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator+=(Matrix<T, C, R> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs + scalar;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator-=(Matrix<T, C, R> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs - scalar;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator*=(Matrix<T, C, R> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs * scalar;
  }

  template <Number T, int C, int R>
  constexpr Matrix<T, C, R> &operator/=(Matrix<T, C, R> &lhs, identity_t<T> scalar) noexcept
  {
    return lhs = lhs / scalar;
  }

#pragma endregion

#pragma region Linear algebra

  /// @brief The component-wise (Hadamard) product of @p lhs and @p rhs.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Hadamard(const Matrix<T, C, R> &lhs, const Matrix<T, C, R> &rhs) noexcept
  {
    return Zip(lhs, rhs, std::multiplies<> {});
  }

  /// @brief The transpose of @p m.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr Matrix<T, R, C> Transpose(const Matrix<T, C, R> &m) noexcept
  {
    Matrix<T, R, C> transpose;
    for (int column = 0; column < C; ++column)
    {
      for (int row = 0; row < R; ++row)
      {
        transpose[row][column] = m[column][row];
      }
    }
    return transpose;
  }

  /// @brief The determinant of the square matrix @p m.
  template <Number T, int N>
  KRYS_NODISCARD constexpr T Determinant(const Matrix<T, N, N> &m) noexcept
  {
    if constexpr (N == 2)
    {
      return m[0].x * m[1].y - m[0].y * m[1].x;
    }
    else if constexpr (N == 3)
    {
      return m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) - m[1].x * (m[0].y * m[2].z - m[2].y * m[0].z)
             + m[2].x * (m[0].y * m[1].z - m[0].z * m[1].y);
    }
    else
    {
      // Expansion along the first row, by the minors of the remaining three rows.
      auto minor = [&m](int a, int b, int c)
      {
        return Determinant(
          Matrix<T, 3, 3> {{m[a].y, m[a].z, m[a].w}, {m[b].y, m[b].z, m[b].w}, {m[c].y, m[c].z, m[c].w}});
      };
      return m[0].x * minor(1, 2, 3) - m[1].x * minor(0, 2, 3) + m[2].x * minor(0, 1, 3)
             - m[3].x * minor(0, 1, 2);
    }
  }

  /// @brief The inverse of the square matrix @p m, or its adjugate if it's singular (determinant 0).
  template <Number T, int N>
  KRYS_NODISCARD constexpr Matrix<T, N, N> Inverse(const Matrix<T, N, N> &m) noexcept
  {
    if constexpr (N == 2)
    {
      T determinant = Determinant(m);
      T inverseDeterminant = T(1) / (determinant == T(0) ? T(1) : determinant);
      return {m[1].y * inverseDeterminant, -m[0].y * inverseDeterminant, -m[1].x * inverseDeterminant,
              m[0].x * inverseDeterminant};
    }
    else if constexpr (N == 3)
    {
      T determinant = Determinant(m);
      T inverseDeterminant = T(1) / (determinant == T(0) ? T(1) : determinant);
      return {(m[1].y * m[2].z - m[2].y * m[1].z) * inverseDeterminant,
              -(m[0].y * m[2].z - m[2].y * m[0].z) * inverseDeterminant,
              (m[0].y * m[1].z - m[1].y * m[0].z) * inverseDeterminant,

              -(m[1].x * m[2].z - m[2].x * m[1].z) * inverseDeterminant,
              (m[0].x * m[2].z - m[2].x * m[0].z) * inverseDeterminant,
              -(m[0].x * m[1].z - m[1].x * m[0].z) * inverseDeterminant,

              (m[1].x * m[2].y - m[2].x * m[1].y) * inverseDeterminant,
              -(m[0].x * m[2].y - m[2].x * m[0].y) * inverseDeterminant,
              (m[0].x * m[1].y - m[1].x * m[0].y) * inverseDeterminant};
    }
    else
    {
      // 2x2 minors of the lower two columns (A_rrcc: rows, then columns).
      T a2323 = m[2].z * m[3].w - m[2].w * m[3].z;
      T a1323 = m[2].y * m[3].w - m[2].w * m[3].y;
      T a1223 = m[2].y * m[3].z - m[2].z * m[3].y;
      T a0323 = m[2].x * m[3].w - m[2].w * m[3].x;
      T a0223 = m[2].x * m[3].z - m[2].z * m[3].x;
      T a0123 = m[2].x * m[3].y - m[2].y * m[3].x;
      T a2313 = m[1].z * m[3].w - m[1].w * m[3].z;
      T a1313 = m[1].y * m[3].w - m[1].w * m[3].y;
      T a1213 = m[1].y * m[3].z - m[1].z * m[3].y;
      T a2312 = m[1].z * m[2].w - m[1].w * m[2].z;
      T a1312 = m[1].y * m[2].w - m[1].w * m[2].y;
      T a1212 = m[1].y * m[2].z - m[1].z * m[2].y;
      T a0313 = m[1].x * m[3].w - m[1].w * m[3].x;
      T a0213 = m[1].x * m[3].z - m[1].z * m[3].x;
      T a0312 = m[1].x * m[2].w - m[1].w * m[2].x;
      T a0212 = m[1].x * m[2].z - m[1].z * m[2].x;
      T a0113 = m[1].x * m[3].y - m[1].y * m[3].x;
      T a0112 = m[1].x * m[2].y - m[1].y * m[2].x;

      T determinant = m[0].x * (m[1].y * a2323 - m[1].z * a1323 + m[1].w * a1223)
                      - m[0].y * (m[1].x * a2323 - m[1].z * a0323 + m[1].w * a0223)
                      + m[0].z * (m[1].x * a1323 - m[1].y * a0323 + m[1].w * a0123)
                      - m[0].w * (m[1].x * a1223 - m[1].y * a0223 + m[1].z * a0123);
      T d = T(1) / (determinant == T(0) ? T(1) : determinant);

      return {{d * (m[1].y * a2323 - m[1].z * a1323 + m[1].w * a1223),
               d * -(m[0].y * a2323 - m[0].z * a1323 + m[0].w * a1223),
               d * (m[0].y * a2313 - m[0].z * a1313 + m[0].w * a1213),
               d * -(m[0].y * a2312 - m[0].z * a1312 + m[0].w * a1212)},
              {d * -(m[1].x * a2323 - m[1].z * a0323 + m[1].w * a0223),
               d * (m[0].x * a2323 - m[0].z * a0323 + m[0].w * a0223),
               d * -(m[0].x * a2313 - m[0].z * a0313 + m[0].w * a0213),
               d * (m[0].x * a2312 - m[0].z * a0312 + m[0].w * a0212)},
              {d * (m[1].x * a1323 - m[1].y * a0323 + m[1].w * a0123),
               d * -(m[0].x * a1323 - m[0].y * a0323 + m[0].w * a0123),
               d * (m[0].x * a1313 - m[0].y * a0313 + m[0].w * a0113),
               d * -(m[0].x * a1312 - m[0].y * a0312 + m[0].w * a0112)},
              {d * -(m[1].x * a1223 - m[1].y * a0223 + m[1].z * a0123),
               d * (m[0].x * a1223 - m[0].y * a0223 + m[0].z * a0123),
               d * -(m[0].x * a1213 - m[0].y * a0213 + m[0].z * a0113),
               d * (m[0].x * a1212 - m[0].y * a0212 + m[0].z * a0112)}};
    }
  }

  /// @brief The outer product of @p a and @p b: the matrix whose column `c` is `a * b[c]`.
  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr Matrix<T, C, R> OuterProduct(const Vector<T, R> &a, const Vector<T, C> &b) noexcept
  {
    Matrix<T, C, R> product;
    for (int column = 0; column < C; ++column)
    {
      for (int row = 0; row < R; ++row)
      {
        product[column][row] = a[row] * b[column];
      }
    }
    return product;
  }

#pragma endregion
}
