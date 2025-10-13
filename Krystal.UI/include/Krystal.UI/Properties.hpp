#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Nullable.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Unit.hpp"

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

  struct Margin
  {
    Unit Top {0.f, Unit::Pixels};
    Unit Right {0.f, Unit::Pixels};
    Unit Bottom {0.f, Unit::Pixels};
    Unit Left {0.f, Unit::Pixels};

    constexpr Margin() noexcept = default;

    constexpr Margin(Unit all) noexcept : Top(all), Right(all), Bottom(all), Left(all)
    {
    }

    constexpr Margin(Unit vertical, Unit horizontal) noexcept
        : Top(vertical), Right(horizontal), Bottom(vertical), Left(horizontal)
    {
    }

    constexpr Margin(Unit top, Unit right, Unit bottom, Unit left) noexcept
        : Top(top), Right(right), Bottom(bottom), Left(left)
    {
    }
  };

  struct Padding
  {
    Unit Top {0.f, Unit::Pixels};
    Unit Right {0.f, Unit::Pixels};
    Unit Bottom {0.f, Unit::Pixels};
    Unit Left {0.f, Unit::Pixels};

    constexpr Padding() noexcept = default;

    constexpr Padding(Unit all) noexcept : Top(all), Right(all), Bottom(all), Left(all)
    {
    }
    constexpr Padding(Unit vertical, Unit horizontal) noexcept
        : Top(vertical), Right(horizontal), Bottom(vertical), Left(horizontal)
    {
    }
    constexpr Padding(Unit top, Unit right, Unit bottom, Unit left) noexcept
        : Top(top), Right(right), Bottom(bottom), Left(left)
    {
    }
  };

  struct BorderProperties
  {
    Gfx::Colour Colour {Gfx::Colours::Black};
    Unit Width {0.f, Unit::Pixels};

    constexpr BorderProperties() noexcept = default;

    constexpr BorderProperties(Gfx::Colour colour, Unit width) noexcept : Colour(colour), Width(width)
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

  struct Offsets
  {
    Unit Top {0.f, Unit::Pixels};
    Unit Right {0.f, Unit::Pixels};
    Unit Bottom {0.f, Unit::Pixels};
    Unit Left {0.f, Unit::Pixels};
  };

  struct TextStyles
  {
    Gfx::FontHandle Font {};
    Unit FontSize {16.f, Unit::Points};
    Gfx::Colour Colour {Gfx::Colours::Black};
  };

  struct Properties
  {
    bool Visible {true};
    DisplayType Display {DisplayType::Flex};

    FlexDir FlexDirection {FlexDir::Row};
    Justify JustifyContent {Justify::Start};
    Align AlignItems {Align::Start};
    Align AlignSelf {Align::Start};
    Unit Gap {0.f, Unit::Pixels};

    Unit Width = Auto;
    Unit Height = Auto;

    Margin Margin {};
    Padding Padding {};
    Border Border {};

    Positioning Position {Positioning::Relative};
    Offsets Offsets {};
    int32 ZIndex {0};

    Gfx::Colour BackgroundColour {Gfx::Colours::Transparent};
    TextStyles TextStyles {};

    constexpr Properties() noexcept = default;
  };

  struct ComputedSize
  {
    Unit IntrinsicWidth  = {0.f, Unit::Pixels};
    Unit IntrinsicHeight = {0.f, Unit::Pixels};
    Unit ActualWidth = {0.f, Unit::Pixels};
    Unit ActualHeight = {0.f, Unit::Pixels};

  };

  struct ComputedLayout
  {
    Unit IntrinsicWidth = {0.f, Unit::Pixels};
    Unit IntrinsicHeight = {0.f, Unit::Pixels};
    Unit ActualWidth = {0.f, Unit::Pixels};
    Unit ActualHeight = {0.f, Unit::Pixels};

    Margin Margin {};
    Padding Padding {};
    Border BorderWidth {};
    Offsets Offsets {};
    Unit Gap = {0.f, Unit::Pixels};

    Gfx::Colour BackgroundColour {};
    Unit FontSize = {0.f, Unit::Pixels};

    float X {0.f};
    float Y {0.f};
    int32 ZIndex {0};
  };
}