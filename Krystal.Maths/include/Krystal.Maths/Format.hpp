#pragma once

#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Quaternion.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <format>
#include <iostream>

namespace std
{
#pragma region Vector

  /// @brief Provides a custom formatter for vectors.
  template <VECTOR_TEMPLATE_PARAMS>
  struct formatter<VECTOR_TYPE>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const VECTOR_TYPE &v, FormatContext &ctx) const
    {
      if constexpr (N == 1)
        return format_to(ctx.out(), "({})", v.x);
      else if constexpr (N == 2)
        return format_to(ctx.out(), "({}, {})", v.x, v.y);
      else if constexpr (N == 3)
        return format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
      else if constexpr (N == 4)
        return format_to(ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      else
        static_assert(N > 0 && N < 5, "Only vectors of length 1, 2, 3, or 4 are supported");
    }
  };

  template <VECTOR_TEMPLATE_PARAMS>
  ostream &operator<<(std::ostream &os, const VECTOR_TYPE &v)
  {
    os << std::format("{}", v);
    return os;
  }

#pragma endregion

#pragma region Matrix

  /// @brief Provides a custom formatter for matrices.
  template <MATRIX_TEMPLATE_PARAMS>
  struct formatter<MATRIX_TYPE>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(const MATRIX_TYPE &m, FormatContext &ctx) const
    {
      if constexpr (R == 2 && C == 2)
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0], m[1][1]);
      else if constexpr (R == 2 && C == 3)
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1],
                         m[1][2]);
      else if constexpr (R == 2 && C == 4)
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]", m[0][0], m[0][1], m[0][2], m[0][3],
                         m[1][0], m[1][1], m[1][2], m[1][3]);
      else if constexpr (R == 3 && C == 2)
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0], m[1][1],
                         m[2][0], m[2][1]);
      else if constexpr (R == 3 && C == 3)
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]", m[0][0], m[0][1], m[0][2],
                         m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
      else if constexpr (R == 3 && C == 4)
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]", m[0][0], m[0][1],
                         m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2],
                         m[2][3]);
      else if constexpr (R == 4 && C == 2)
        return format_to(ctx.out(), "[{}, {}]\n[{}, {}]\n[{}, {}]\n[{}, {}]", m[0][0], m[0][1], m[1][0],
                         m[1][1], m[2][0], m[2][1], m[3][0], m[3][1]);
      else if constexpr (R == 4 && C == 3)
        return format_to(ctx.out(), "[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]", m[0][0],
                         m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0],
                         m[3][1], m[3][2]);
      else if constexpr (R == 4 && C == 4)
        return format_to(ctx.out(), "[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]",
                         m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0],
                         m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
      else
        static_assert(R > 1 && R < 5 && C > 1 && C < 5,
                      "Only matrices ranging from 2x2 to 4x4 are supported.");
    }
  };

  template <MATRIX_TEMPLATE_PARAMS>
  ostream &operator<<(std::ostream &os, const MATRIX_TYPE &m)
  {
    os << std::format("{}", m);
    return os;
  }

#pragma endregion

#pragma region Quaternion

  template <QUATERNION_TEMPLATE_PARAMS>
  struct formatter<QUATERNION_TYPE>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const QUATERNION_TYPE &q, FormatContext &ctx)
    {
      return format_to(ctx.out(), "Q(w: {}, x: {}, y: {}, z: {})", q.w, q.x, q.y, q.z);
    }
  };

  template <QUATERNION_TEMPLATE_PARAMS>
  ostream &operator<<(std::ostream &os, const QUATERNION_TYPE &q)
  {
    os << std::format("{}", q);
    return os;
  }

#pragma endregion
}