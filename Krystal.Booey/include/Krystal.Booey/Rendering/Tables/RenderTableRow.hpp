/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2016 Google Inc. All rights reserved.
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
  class RenderTableSection;
  struct BorderValue;

  /// @brief A table row.
  class RenderTableRow final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableRow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableRow);

  public:
    /// @brief Paints a row with a self-painting layer: its outline, its background behind its cells, and the
    /// cells; rows without layers are painted by their section.
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;
    void PaintOutlineForRowIfNeeded(PaintInfo &, const LayoutPoint &) noexcept;
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction action) noexcept final;

    constexpr static uint32 UnsetRowIndex = 0x7F'FF'FF'FF;
    constexpr static uint32 MaxRowIndex = 0x7F'FF'FF'FE; // 2,147,483,646

  private:
    uint32 _rowIndex : 31 {UnsetRowIndex};

  public:
    RenderTableRow(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableRow(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableRow() noexcept override;

    KRYS_NODISCARD RenderTableRow *NextRow() const noexcept;
    KRYS_NODISCARD RenderTableRow *PreviousRow() const noexcept;

    KRYS_NODISCARD RenderTableCell *FirstCell() const noexcept;
    KRYS_NODISCARD RenderTableCell *LastCell() const noexcept;

    KRYS_NODISCARD RenderTable *Table() const noexcept;
    KRYS_NODISCARD RenderTableSection *Section() const noexcept;

    void SetRowIndex(uint32 rowIndex) noexcept;
    KRYS_NODISCARD bool RowIndexWasSet() const noexcept
    {
      return _rowIndex != UnsetRowIndex;
    }
    KRYS_NODISCARD uint32 RowIndex() const noexcept
    {
      krys_debug_assert(RowIndexWasSet());
      return _rowIndex;
    }

    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableStart() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableEnd() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningStartCell(const RenderTableCell &cell) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningEndCell(const RenderTableCell &cell) const noexcept;

    void DidInsertTableCell(RenderTableCell &child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableRow (anonymous)" : u8"RenderTableRow";
    }

    void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept override;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext context) const noexcept override;
    KRYS_NODISCARD RepaintRects
      RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                    RepaintOutlineBounds repaintOutlineBounds) const noexcept override;

  private:
    void WillBeRemovedFromTree() noexcept override;
    void Layout() noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
    }

    KRYS_NODISCARD bool RequiresLayer() const noexcept final
    {
      return HasNonVisibleOverflow() || HasTransformRelatedProperty() || HasHiddenBackface() || HasClipPath()
             || CreatesGroup() || IsStickilyPositioned();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableRow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableRow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
