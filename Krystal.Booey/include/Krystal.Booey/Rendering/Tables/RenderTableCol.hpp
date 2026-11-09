/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
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

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTable;
  class RenderTableCell;
  struct BorderValue;

  /// @brief A table column or column group.
  class RenderTableCol final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCol);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCol);

  private:
    uint32 _span {1};

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final
    {
    }

    RenderTableCol(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableCol(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableCol() noexcept override;

    void ClearNeedsPreferredLogicalWidthsUpdate() noexcept;

    KRYS_NODISCARD uint32 Span() const noexcept
    {
      return _span;
    }
    void SetSpan(uint32 span) noexcept
    {
      _span = span;
    }

    KRYS_NODISCARD bool IsTableColumnGroupWithColumnChildren() const noexcept
    {
      return FirstChild();
    }
    KRYS_NODISCARD bool IsTableColumn() const noexcept;
    KRYS_NODISCARD bool IsTableColumnGroup() const noexcept;

    KRYS_NODISCARD RenderTableCol *EnclosingColumnGroup() const noexcept;
    KRYS_NODISCARD RenderTableCol *EnclosingColumnGroupIfAdjacentBefore() const noexcept
    {
      if (PreviousSibling())
      {
        return nullptr;
      }
      return EnclosingColumnGroup();
    }
    KRYS_NODISCARD RenderTableCol *EnclosingColumnGroupIfAdjacentAfter() const noexcept
    {
      if (NextSibling())
      {
        return nullptr;
      }
      return EnclosingColumnGroup();
    }

    /// @return The next column or column group.
    KRYS_NODISCARD RenderTableCol *NextColumn() const noexcept;

    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellStartBorder() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellEndBorder() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellBefore(const RenderTableCell &cell) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellAfter(const RenderTableCell &cell) const noexcept;

    KRYS_NODISCARD LayoutUnit OffsetLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit OffsetTop() const noexcept override;
    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept override;
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept override;

    /// @brief Updates the span from the element's span attribute.
    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &child,
                                       const RenderStyle &style) const noexcept override;
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTableCol";
    }

    void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept override;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext context) const noexcept override;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept override;

  private:
    void ComputePreferredLogicalWidths() noexcept override
    {
      krys_debug_assert(false);
    }
    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
      krys_debug_assert(false);
    }

    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD bool RequiresLayer() const noexcept override
    {
      return false;
    }

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD RenderTable *Table() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCol)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCol();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
