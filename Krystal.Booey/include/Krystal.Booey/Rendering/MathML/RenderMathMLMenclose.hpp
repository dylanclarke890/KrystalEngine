/*
 * Copyright (C) 2014 Gurpreet Kaur (k.gurpreet@samsung.com). All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLMencloseElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<menclose>`: a row with space around it for the notations it draws.
  class RenderMathMLMenclose final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLMenclose);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLMenclose);

  private:
    struct SpaceAroundContent
    {
      LayoutUnit Left;
      LayoutUnit Right;
      LayoutUnit Top;
      LayoutUnit Bottom;
    };

    LayoutRect _contentRect;

  public:
    RenderMathMLMenclose(mathml::MathMLMencloseElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLMenclose() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLMenclose";
    }

  private:
    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;

    KRYS_NODISCARD LayoutUnit RuleThickness() const noexcept;
    KRYS_NODISCARD bool
      HasNotation(mathml::MathMLMencloseElement::MencloseNotationFlag notationFlag) const noexcept
    {
      return static_cast<mathml::MathMLMencloseElement &>(NodeForNonAnonymous()).HasNotation(notationFlag);
    }

    KRYS_NODISCARD SpaceAroundContent GetSpaceAroundContent(LayoutUnit contentWidth,
                                                            LayoutUnit contentHeight) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLMenclose)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLMenclose();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
