/*
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2023 Igalia S.L.
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
 */

#pragma once

#include "Krystal.Booey/Rendering/Line/LegacyInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderSVGInlineText;
  class SVGRootInlineBox;

  /// @brief An SVG text renderer's legacy text box, with the text fragments the text layout placed it in.
  class SVGInlineTextBox final : public LegacyInlineTextBox
  {
    KRYS_NON_COPY_MOVABLE(SVGInlineTextBox);

  private:
    float _logicalHeight {0};

    List<SVGTextFragment> _textFragments;

  public:
    explicit SVGInlineTextBox(RenderSVGInlineText &) noexcept;

    KRYS_NODISCARD float VirtualLogicalHeight() const noexcept override
    {
      return _logicalHeight;
    }
    void SetLogicalHeight(float height) noexcept
    {
      _logicalHeight = height;
    }

    bool MapStartEndPositionsIntoFragmentCoordinates(const SVGTextFragment &, uint32 &startPosition,
                                                     uint32 &endPosition) const noexcept;

    KRYS_NODISCARD FloatRect CalculateBoundaries() const noexcept;

    KRYS_NODISCARD const List<SVGTextFragment> &TextFragments() const noexcept
    {
      return _textFragments;
    }
    void SetTextFragments(List<SVGTextFragment> &&) noexcept;

    void DirtyOwnLineBoxes() noexcept override;
    void DirtyLineBoxes() noexcept override;

    KRYS_NODISCARD SVGInlineTextBox *NextTextBox() const noexcept;

  private:
    KRYS_NODISCARD bool IsSVGInlineTextBox() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGInlineTextBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsSVGInlineTextBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
