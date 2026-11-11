/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBlockFlow;
  class RenderBox;
  class RenderInline;
  struct PaintInfo;

  namespace inline_display
  {
    struct Box;
  }

  namespace layout
  {
    class ElementBox;
  }
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief Paints a block's inline content for one paint phase: its display boxes in order (text, inline
  /// boxes, atomic inlines and blocks in inline), culled to the paint rect.
  class InlineContentPainter
  {
    KRYS_NON_COPY_MOVABLE(InlineContentPainter);

  private:
    PaintInfo &_paintInfo;
    const LayoutPoint _paintOffset;
    LayoutRect _damageRect;
    const RenderInline *_inlineBoxWithLayer {nullptr};
    const InlineContent &_inlineContent;
    const RenderBlockFlow &_root;
    ListHashSet<const RenderInline *> _outlineObjects;

  public:
    /// @param inlineBoxWithLayer The inline box whose layer is being painted, if any: only its content is
    /// painted.
    InlineContentPainter(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                         const RenderInline *inlineBoxWithLayer, const InlineContent &inlineContent,
                         const RenderBlockFlow &root) noexcept;

    void Paint() noexcept;

  private:
    void PaintDisplayBox(const inline_display::Box &box) noexcept;
    void PaintEllipsis(size_t lineIndex) noexcept;
    KRYS_NODISCARD LayoutPoint FlippedContentOffsetIfNeeded(const RenderBox &childRenderer) const noexcept;
    KRYS_NODISCARD const RenderBlock &Root() const noexcept;
  };

  /// @brief Which display boxes a layer's painting includes: everything outside inline boxes with
  /// self-painting layers (which paint themselves), or only the content of the inline box whose layer is
  /// being painted.
  class LayerPaintScope
  {
  private:
    const layout::ElementBox *const _inlineBoxWithLayer;
    const layout::ElementBox *_currentExcludedInlineBox {nullptr};

  public:
    explicit LayerPaintScope(const RenderInline *inlineBoxWithLayer) noexcept;

    /// @brief Whether @p box is painted, noting an inline box with a self-painting layer as excluded.
    KRYS_NODISCARD bool TestIsIncludesAndUpdate(const inline_display::Box &box) noexcept;
  };
}
