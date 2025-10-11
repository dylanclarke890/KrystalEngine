#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"

namespace Krys::Maths
{
  template <Arithmetic T>
  struct BoundingBox
  {
    using Type = T;

    T Left, Right, Bottom, Top;

    constexpr BoundingBox() noexcept : Left(T(0)), Right(T(0)), Bottom(T(0)), Top(T(0))
    {
    }

    constexpr BoundingBox(T width, T height) noexcept : Left(T(0)), Right(width), Bottom(T(0)), Top(height)
    {
    }

    constexpr BoundingBox(T left, T right, T bottom, T top) noexcept
        : Left(left), Right(right), Bottom(bottom), Top(top)
    {
    }

    NO_DISCARD constexpr T GetWidth() const noexcept
    {
      return Right - Left;
    }

    NO_DISCARD constexpr T GetHeight() const noexcept
    {
      return Top - Bottom;
    }
  };
}