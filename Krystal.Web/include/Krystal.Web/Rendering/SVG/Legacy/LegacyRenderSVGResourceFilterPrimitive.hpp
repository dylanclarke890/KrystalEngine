/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
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

#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"

namespace krys::web
{
  class FilterEffect;
  class SVGFilterPrimitiveStandardAttributes;

  /// @brief The renderer of a filter primitive: it holds the primitive's style, and tells its filter when the
  /// primitive changes.
  class LegacyRenderSVGResourceFilterPrimitive final : public LegacyRenderSVGHiddenContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceFilterPrimitive);

  public:
    LegacyRenderSVGResourceFilterPrimitive(SVGFilterPrimitiveStandardAttributes &, RenderStyle &&) noexcept;

    KRYS_NODISCARD SVGFilterPrimitiveStandardAttributes &FilterPrimitiveElement() const noexcept;

    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceFilterPrimitive";
    }

    void MarkFilterEffectForRepaint(FilterEffect *) noexcept;

    void MarkFilterEffectForRebuild() noexcept;

    void GetElement() const = delete;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::LegacyRenderSVGResourceFilterPrimitive)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceFilterPrimitive();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
