#pragma once

#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Quaternion.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include <format>
#include <iostream>

namespace std
{
#pragma region Vector

  /// @brief Provides a custom formatter for vectors.
  template <krys::Number T, int N>
  struct formatter<krys::Vector<T, N>>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const krys::Vector<T, N> &v, FormatContext &ctx) const
    {
      if constexpr (N == 1)
      {
        return format_to(ctx.out(), "({})", v.x);
      }
      else if constexpr (N == 2)
      {
        return format_to(ctx.out(), "({}, {})", v.x, v.y);
      }
      else if constexpr (N == 3)
      {
        return format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
      }
      else if constexpr (N == 4)
      {
        return format_to(ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      }
      else
      {
        static_assert(N > 0 && N < 5, "Only vectors of length 1, 2, 3, or 4 are supported");
      }
    }
  };

  template <krys::Number T, int N>
  ostream &operator<<(std::ostream &os, const krys::Vector<T, N> &v)
  {
    os << std::format("{}", v);
    return os;
  }

#pragma endregion

#pragma region Matrix

  /// @brief Provides a custom formatter for matrices.
  template <krys::Number T, int C, int R>
  struct formatter<krys::Matrix<T, C, R>>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const krys::Matrix<T, C, R> &m, FormatContext &ctx) const
    {
      if constexpr (R == 2 && C == 2)
      {
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0], m[1][1]);
      }
      else if constexpr (R == 2 && C == 3)
      {
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1],
                         m[1][2]);
      }
      else if constexpr (R == 2 && C == 4)
      {
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]", m[0][0], m[0][1], m[0][2], m[0][3],
                         m[1][0], m[1][1], m[1][2], m[1][3]);
      }
      else if constexpr (R == 3 && C == 2)
      {
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0], m[1][1],
                         m[2][0], m[2][1]);
      }
      else if constexpr (R == 3 && C == 3)
      {
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]", m[0][0], m[0][1], m[0][2],
                         m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
      }
      else if constexpr (R == 3 && C == 4)
      {
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]", m[0][0], m[0][1],
                         m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2],
                         m[2][3]);
      }
      else if constexpr (R == 4 && C == 2)
      {
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]\n[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0],
                         m[1][1], m[2][0], m[2][1], m[3][0], m[3][1]);
      }
      else if constexpr (R == 4 && C == 3)
      {
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]", m[0][0],
                         m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0],
                         m[3][1], m[3][2]);
      }
      else if constexpr (R == 4 && C == 4)
      {
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]",
                         m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0],
                         m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
      }
      else
      {
        static_assert(R > 1 && R < 5 && C > 1 && C < 5,
                      "Only matrices ranging from 2x2 to 4x4 are supported.");
      }
    }
  };

  template <krys::Number T, int C, int R>
  ostream &operator<<(std::ostream &os, const krys::Matrix<T, C, R> &m)
  {
    os << std::format("{}", m);
    return os;
  }

#pragma endregion

#pragma region Quaternion

  template <krys::Number T>
  struct formatter<krys::Quaternion<T>>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const krys::Quaternion<T> &q, FormatContext &ctx)
    {
      return format_to(ctx.out(), "Q(w: {}, x: {}, y: {}, z: {})", q.w, q.x, q.y, q.z);
    }
  };

  template <krys::Number T>
  ostream &operator<<(std::ostream &os, const krys::Quaternion<T> &q)
  {
    os << std::format("{}", q);
    return os;
  }

#pragma endregion
}
