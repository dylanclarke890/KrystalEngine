#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Rendering/Style/BorderValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{
  /// @brief A resolved border in the collapsing border model, with the precedence it won by.
  class CollapsedBorderValue
  {
  private:
    LayoutUnit _width;
    Color _color;
    uint8 _style : 4;      // BorderStyle
    uint8 _precedence : 3; // BorderPrecedence
    uint8 _transparent : 1;

  public:
    CollapsedBorderValue() noexcept
        : _style(static_cast<uint8>(BorderStyle::None)),
          _precedence(static_cast<uint8>(BorderPrecedence::Off)), _transparent(false)
    {
    }

    CollapsedBorderValue(const BorderValue &border, const Color &color, BorderPrecedence precedence,
                         const style::ZoomFactor) noexcept
        : _width(border.NonZero() ? style::Evaluate<LayoutUnit>(border.Width, style::ZoomNeeded {}) : 0_lu),
          _color(color), _style(border.Style), _precedence(static_cast<uint8>(precedence)),
          _transparent(border.IsTransparent())
    {
    }

    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      return Style() > BorderStyle::Hidden ? _width : 0_lu;
    }
    KRYS_NODISCARD BorderStyle Style() const noexcept
    {
      return static_cast<BorderStyle>(_style);
    }
    KRYS_NODISCARD bool Exists() const noexcept
    {
      return Precedence() != BorderPrecedence::Off;
    }
    KRYS_NODISCARD const Color &GetColor() const noexcept
    {
      return _color;
    }
    KRYS_NODISCARD bool IsTransparent() const noexcept
    {
      return _transparent;
    }
    KRYS_NODISCARD BorderPrecedence Precedence() const noexcept
    {
      return static_cast<BorderPrecedence>(_precedence);
    }

    KRYS_NODISCARD bool IsSameIgnoringColor(const CollapsedBorderValue &o) const noexcept
    {
      return Width() == o.Width() && Style() == o.Style() && Precedence() == o.Precedence();
    }

    KRYS_NODISCARD static LayoutUnit AdjustedCollapsedBorderWidth(float borderWidth, float deviceScaleFactor,
                                                                  bool roundUp) noexcept;
  };

  inline LayoutUnit CollapsedBorderValue::AdjustedCollapsedBorderWidth(float borderWidth,
                                                                       float deviceScaleFactor,
                                                                       bool roundUp) noexcept
  {
    float halfCollapsedBorderWidth = (borderWidth + (roundUp ? (1 / deviceScaleFactor) : 0)) / 2;
    return LayoutUnit(FloorToDevicePixel(halfCollapsedBorderWidth, deviceScaleFactor));
  }
}
