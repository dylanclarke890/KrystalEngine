/*
 * Copyright (C) 2012 Google, Inc.
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<circle>` and `<ellipse>`: they're filled, stroked and hit tested without a
  /// path.
  class LegacyRenderSVGEllipse final : public LegacyRenderSVGShape
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGEllipse);

  private:
    FloatPoint _center;
    FloatSize _radii;

  public:
    LegacyRenderSVGEllipse(svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGEllipse() noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGEllipse";
    }

    void UpdateShapeFromElement() noexcept override;
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return HasPath() ? LegacyRenderSVGShape::IsEmpty() : _fillBoundingBox.IsEmpty();
    }
    KRYS_NODISCARD bool IsRenderingDisabled() const noexcept override;
    void FillShape(GraphicsContext &) const noexcept override;
    void StrokeShape(GraphicsContext &) const noexcept override;
    KRYS_NODISCARD bool
      ShapeDependentStrokeContains(const FloatPoint &,
                                   PointCoordinateSpace = GlobalCoordinateSpace) noexcept override;
    KRYS_NODISCARD bool ShapeDependentFillContains(const FloatPoint &,
                                                   const WindRule) const noexcept override;
    void CalculateRadiiAndCenter() noexcept;

    KRYS_NODISCARD bool CanUseStrokeHitTestFastPath() const noexcept;
  };
}
