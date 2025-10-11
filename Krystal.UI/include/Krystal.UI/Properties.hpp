#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  struct BoundingBox
  {
    float X;
    float Y;
    float Width;
    float Height;

    constexpr BoundingBox() noexcept : X(0.f), Y(0.f), Width(0.f), Height(0.f)
    {
    }

    constexpr BoundingBox(float x, float y, float width, float height) noexcept
        : X(x), Y(y), Width(width), Height(height)
    {
    }

    NO_DISCARD constexpr float GetWidth() const noexcept
    {
      return Width;
    }

    NO_DISCARD constexpr float GetHeight() const noexcept
    {
      return Height;
    }

    NO_DISCARD constexpr float GetRight() const noexcept
    {
      return X + Width;
    }

    NO_DISCARD constexpr float GetBottom() const noexcept
    {
      return Y + Height;
    }
  };

  struct EdgeValues
  {
    float Left {0.f};
    float Right {0.f};
    float Top {0.f};
    float Bottom {0.f};

    constexpr float Horizontal() const noexcept
    {
      return Left + Right;
    }

    constexpr float Vertical() const noexcept
    {
      return Top + Bottom;
    }
  };

  enum class SizeUnitType : uint8
  {
    Points,     // 1/72 of an inch
    Pixels,     // Device pixels
    Percentage, // Relative to a parent dimension
  };

  struct SizeUnit
  {
    float Value {0.f};
    SizeUnitType Type {SizeUnitType::Pixels};

    constexpr SizeUnit(float value, SizeUnitType type) noexcept : Value(value), Type(type)
    {
    }
  };

  struct Margin
  {
    SizeUnit Top {0.f, SizeUnitType::Pixels};
    SizeUnit Right {0.f, SizeUnitType::Pixels};
    SizeUnit Bottom {0.f, SizeUnitType::Pixels};
    SizeUnit Left {0.f, SizeUnitType::Pixels};

    constexpr Margin() noexcept = default;

    constexpr Margin(SizeUnit all) noexcept : Top(all), Right(all), Bottom(all), Left(all)
    {
    }

    constexpr Margin(SizeUnit vertical, SizeUnit horizontal) noexcept
        : Top(vertical), Right(horizontal), Bottom(vertical), Left(horizontal)
    {
    }

    constexpr Margin(SizeUnit top, SizeUnit right, SizeUnit bottom, SizeUnit left) noexcept
        : Top(top), Right(right), Bottom(bottom), Left(left)
    {
    }
  };

  struct Padding
  {
    SizeUnit Top {0.f, SizeUnitType::Pixels};
    SizeUnit Right {0.f, SizeUnitType::Pixels};
    SizeUnit Bottom {0.f, SizeUnitType::Pixels};
    SizeUnit Left {0.f, SizeUnitType::Pixels};

    constexpr Padding() noexcept = default;

    constexpr Padding(SizeUnit all) noexcept : Top(all), Right(all), Bottom(all), Left(all)
    {
    }
    constexpr Padding(SizeUnit vertical, SizeUnit horizontal) noexcept
        : Top(vertical), Right(horizontal), Bottom(vertical), Left(horizontal)
    {
    }
    constexpr Padding(SizeUnit top, SizeUnit right, SizeUnit bottom, SizeUnit left) noexcept
        : Top(top), Right(right), Bottom(bottom), Left(left)
    {
    }
  };

  struct BorderProperties
  {
    Gfx::Colour Colour {Gfx::Colours::Black};
    SizeUnit Width {0.f, SizeUnitType::Pixels};

    constexpr BorderProperties() noexcept = default;

    constexpr BorderProperties(Gfx::Colour colour, SizeUnit width) noexcept : Colour(colour), Width(width)
    {
    }
  };

  struct Border
  {
    BorderProperties Top {};
    BorderProperties Right {};
    BorderProperties Bottom {};
    BorderProperties Left {};

    constexpr Border() noexcept = default;
  };

  enum class Anchor
  {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
  };

  struct TextStyle
  {
    Gfx::FontHandle Font {};
    SizeUnit FontSize {16.f, SizeUnitType::Points};
    Gfx::Colour Colour {Gfx::Colours::Black};
    bool Bold {false};
    bool Italic {false};
    bool Underline {false};
  };

  struct Properties
  {
    bool Visible {true};
    Gfx::Colour BackgroundColour {Gfx::Colours::Transparent};

    Margin Margin {};
    Padding Padding {};
    Border Border {};

    TextStyle Text {};

    SizeUnit Width {0.f, SizeUnitType::Pixels};
    SizeUnit Height {0.f, SizeUnitType::Pixels};

    Anchor AnchorPoint {Anchor::TopLeft};
    SizeUnit RelativeX {0.f, SizeUnitType::Pixels};
    SizeUnit RelativeY {0.f, SizeUnitType::Pixels};

    constexpr Properties() noexcept = default;
  };

  struct ComputedLayout
  {
    EdgeValues Margin {};
    EdgeValues Padding {};
    EdgeValues BorderWidth {};
    float FontSizePx {0.f};

    // size/position of content, minus padding, border and margin
    BoundingBox Content {};

    // size/position including padding, border and margin
    BoundingBox Bounds {};
  };

  constexpr SizeUnit operator"" _pt(long double value) noexcept
  {
    return SizeUnit(static_cast<float>(value), SizeUnitType::Points);
  }

  constexpr SizeUnit operator"" _px(long double value) noexcept
  {
    return SizeUnit(static_cast<float>(value), SizeUnitType::Pixels);
  }

  constexpr SizeUnit operator"" _pct(long double value) noexcept
  {
    return SizeUnit(static_cast<float>(value), SizeUnitType::Percentage);
  }
}