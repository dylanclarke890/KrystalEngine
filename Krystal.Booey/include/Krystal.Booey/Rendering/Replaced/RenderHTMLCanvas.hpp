/*
 * Copyright (C) 2004, 2006, 2007, 2009 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/Replaced/RenderReplaced.hpp"

namespace krys::boo::html
{
  class HTMLCanvasElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<canvas>`: replaced content at the canvas's size, which its context draws.
  class RenderHTMLCanvas final : public RenderReplaced
  {
    KRYS_NON_COPY_MOVABLE(RenderHTMLCanvas);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderHTMLCanvas);

  public:
    RenderHTMLCanvas(html::HTMLCanvasElement &element, RenderStyle &&style) noexcept;
    ~RenderHTMLCanvas() noexcept override;

    KRYS_NODISCARD html::HTMLCanvasElement &CanvasElement() const noexcept;

    /// @brief Follows the canvas's size, zoomed, as the intrinsic size.
    void CanvasSizeChanged() noexcept;

  private:
    KRYS_NODISCARD bool RequiresLayer() const noexcept override;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderHTMLCanvas";
    }
    void PaintReplaced(PaintInfo &, const LayoutPoint &) noexcept override;
    void IntrinsicSizeChanged() noexcept override
    {
      CanvasSizeChanged();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderHTMLCanvas)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderHTMLCanvas();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
