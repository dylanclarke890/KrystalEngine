/*
 * Copyright (C) 2010 Alex Milowski (alex@milowski.com). All rights reserved.
 * Copyright (C) 2016 Igalia S.L.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLRowElement;
  }

  /// @brief Lays its children out in a row, on a common baseline, stretching its vertical operators to the
  /// row's height: the renderer of `<mrow>` and the base of the renderers with more specific layouts.
  class RenderMathMLRow : public RenderMathMLBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLRow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLRow);

  public:
    RenderMathMLRow(Type type, mathml::MathMLRowElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLRow() noexcept override;

    KRYS_NODISCARD mathml::MathMLRowElement &GetElement() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMathMLRow";
    }

  protected:
    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;

    void StretchVerticalOperatorsAndLayoutChildren() noexcept;
    void GetContentBoundingBox(LayoutUnit &width, LayoutUnit &ascent, LayoutUnit &descent) const noexcept;
    void LayoutRowItems(LayoutUnit width, LayoutUnit ascent) noexcept;
    KRYS_NODISCARD LayoutUnit PreferredLogicalWidthOfRowItems() noexcept;
    void ComputePreferredLogicalWidths() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLRow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLRow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
