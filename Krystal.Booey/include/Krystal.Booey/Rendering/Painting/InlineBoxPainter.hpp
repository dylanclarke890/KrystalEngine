/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"

namespace krys::boo
{
  class Color;
  class RenderBoxModelObject;
  class RenderStyle;
  struct PaintInfo;
  template <typename TLayer>
  struct FillLayerToPaint;

  namespace inline_display
  {
    struct Box;
  }

  namespace layout_integration
  {
    class InlineContent;
  }

  /// @brief Paints an inline box's decorations: its box shadows, backgrounds and borders, on each line it's
  /// split across.
  class InlineBoxPainter
  {
    KRYS_NON_COPY_MOVABLE(InlineBoxPainter);

  private:
    const inline_iterator::InlineBox _inlineBox;
    PaintInfo &_paintInfo;
    const LayoutPoint _paintOffset;
    const RenderBoxModelObject &_renderer;
    const bool _isFirstLineBox;
    const bool _isRootInlineBox;
    const bool _isHorizontal;

  public:
    InlineBoxPainter(const layout_integration::InlineContent &inlineContent, const inline_display::Box &box,
                     PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;
    ~InlineBoxPainter() noexcept;

    void Paint() noexcept;

  private:
    InlineBoxPainter(const inline_iterator::InlineBox &inlineBox, PaintInfo &paintInfo,
                     const LayoutPoint &paintOffset) noexcept;

    void PaintMask() noexcept;
    void PaintDecorations() noexcept;
    template <typename TLayers>
    void PaintFillLayers(const Color &color, const TLayers &fillLayers, const LayoutRect &paintRect,
                         CompositeOperator op) noexcept;
    template <typename TLayer>
    void PaintFillLayer(const Color &color, const FillLayerToPaint<TLayer> &fillLayer,
                        const LayoutRect &paintRect, CompositeOperator op) noexcept;
    void PaintBoxShadow(style::ShadowStyle shadowStyle, const LayoutRect &paintRect) noexcept;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    // FIXME: Make RenderBoxModelObject functions const.
    KRYS_NODISCARD RenderBoxModelObject &Renderer() const noexcept
    {
      return const_cast<RenderBoxModelObject &>(_renderer);
    }
    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _isHorizontal;
    }
  };
}
