#pragma once

namespace krys::boo::css::calc
{
  // Don't change these values; parsing uses them.
  enum class Operator : uint8
  {
    Sum = '+',
    Negate = '-',
    Product = '*',
    Invert = '/',
    Min = 0,
    Max,
    Clamp,
    Pow,
    Sqrt,
    Hypot,
    Sin,
    Cos,
    Tan,
    Exp,
    Log,
    Asin,
    Acos,
    Atan,
    Atan2,
    Abs,
    Sign,
    Mod,
    Rem,
    RoundNearest,
    RoundUp,
    RoundDown,
    RoundToZero,
    Progress,
    Random,
    Blend,
  };
}
