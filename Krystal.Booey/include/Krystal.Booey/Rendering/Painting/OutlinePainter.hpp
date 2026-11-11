/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBlockFlow;
  class RenderBox;
  class RenderElement;
  class RenderInline;
  class RenderLayerModelObject;
  struct PaintInfo;

  /// @brief Paints a box's outline (`outline`), or its focus ring for `outline-style: auto`.
  // FIXME: List box focus rings come with controls.
  class OutlinePainter
  {
  private:
    const PaintInfo &_paintInfo;

  public:
    explicit OutlinePainter(const PaintInfo &) noexcept;

    void PaintOutline(const RenderElement &, const LayoutRect &paintRect) const noexcept;
    /// @brief Paints an inline's outline around its line boxes, shrink-wrapped into one shape where they
    /// touch.
    void PaintOutline(const RenderInline &, const LayoutPoint &paintOffset) const noexcept;

    KRYS_NODISCARD static List<LayoutRect>
      CollectFocusRingRects(const RenderElement &, const LayoutPoint &additionalOffset,
                            const RenderLayerModelObject *paintContainer) noexcept;

    /// @brief One path outlining @p rects (an inline's line boxes) with the box's corner radii on its first
    /// and last lines, or an empty path when they don't join into one shape.
    KRYS_NODISCARD static Path PathWithShrinkWrappedRects(const List<FloatRect> &rects,
                                                          const style::BorderRadius &radii,
                                                          float outlineOffset, WritingMode writingMode,
                                                          float deviceScaleFactor) noexcept;

  private:
    void PaintOutlineWithLineRects(const RenderInline &, const LayoutPoint &paintOffset,
                                   const List<LayoutRect> &lineRects) const noexcept;
    void PaintFocusRing(const RenderElement &, const List<LayoutRect> &) const noexcept;

    static void CollectFocusRingRects(const RenderElement &, List<LayoutRect> &,
                                      const LayoutPoint &additionalOffset,
                                      const RenderLayerModelObject *paintContainer) noexcept;
    static void CollectFocusRingRectsForInline(const RenderInline &, List<LayoutRect> &, const LayoutPoint &,
                                               const RenderLayerModelObject *) noexcept;
    static void CollectFocusRingRectsForInlineChildren(const RenderBlockFlow &, List<LayoutRect> &,
                                                       const LayoutPoint &,
                                                       const RenderLayerModelObject *) noexcept;
    static bool CollectFocusRingRectsForBlock(const RenderBlock &, List<LayoutRect> &, const LayoutPoint &,
                                              const RenderLayerModelObject *) noexcept;
    static void CollectFocusRingRectsForChildBox(const RenderBox &, List<LayoutRect> &, const LayoutPoint &,
                                                 const RenderLayerModelObject *) noexcept;
  };
}
