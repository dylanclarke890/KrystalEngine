/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
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
