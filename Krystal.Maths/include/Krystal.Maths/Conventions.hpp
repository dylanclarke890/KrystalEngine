#pragma once

namespace Krys::Maths::Conventions
{
  enum class Handedness
  {
    Left,
    Right
  };

  enum class DepthRange
  {
    ZeroToOne,
    NegativeOneToOne
  };

  constexpr Handedness DefaultHandedness = Handedness::Right;
  constexpr DepthRange DefaultDepthRange = DepthRange::NegativeOneToOne;
}