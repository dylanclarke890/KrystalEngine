#pragma once

namespace Krys::UI
{
  struct Unit
  {
    enum UnitType : uint8
    {
      Auto,       // Calculated by the layout engine
      Points,     // 1/72 of an inch
      Pixels,     // Device pixels
      Percentage, // Relative to a parent dimension
    };

    float Value {0.f};
    UnitType Type {Pixels};

    constexpr Unit(float value, UnitType type) noexcept : Value(value), Type(type)
    {
    }

    constexpr bool IsPixels() const noexcept
    {
      return Type == Pixels;
    }

    constexpr bool IsPoints() const noexcept
    {
      return Type == Points;
    }

    constexpr bool IsPercentage() const noexcept
    {
      return Type == Percentage;
    }

    constexpr bool IsAuto() const noexcept
    {
      return Type == Auto;
    }

    constexpr bool IsFixed() const noexcept
    {
      return Type == Points || Type == Pixels;
    }
  };

  constexpr Unit Auto {0.f, Unit::Auto};

  namespace Literals
  {
    constexpr Unit operator"" _pt(long double value) noexcept
    {
      return Unit(static_cast<float>(value), Unit::Points);
    }

    constexpr Unit operator"" _px(long double value) noexcept
    {
      return Unit(static_cast<float>(value), Unit::Pixels);
    }

    constexpr Unit operator"" _pct(long double value) noexcept
    {
      return Unit(static_cast<float>(value), Unit::Percentage);
    }
  }
}