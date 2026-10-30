#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"

namespace krys::boo::style
{
  // MARK: Integer Primitive

  template <css::Range = css::All, typename = int>
  struct Integer;

  // MARK: Number Primitive

  template <css::Range = css::All, typename = double>
  struct Number;

  // MARK: Percentage Primitive

  template <css::Range = css::All, typename = double>
  struct Percentage;

  // MARK: Dimension Primitives

  template <css::Range = css::All, typename = double>
  struct Angle;

  template <css::Range = css::All, typename = float>
  struct Length;

  template <css::Range = css::All, typename = double>
  struct Time;

  template <css::Range = css::All, typename = double>
  struct Frequency;

  template <css::Range = css::NonNegative, typename = double>
  struct Resolution;

  template <css::Range = css::All, typename = double>
  struct Flex;

  // MARK: Dimension + Percentage Primitives

  template <css::Range = css::All, typename = float>
  struct AnglePercentage;

  template <css::Range = css::All, typename = float>
  struct LengthPercentage;
}
