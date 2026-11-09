/*
 * Copyright (C) 2009 Alex Milowski (alex@milowski.com). All rights reserved.
 * Copyright (C) 2010 François Sausset (sausset@gmail.com). All rights reserved.
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

#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLFractionElement;
  }

  /// @brief The renderer of `<mfrac>`: its numerator centred over its denominator, with a fraction bar
  /// between them on the math axis.
  /// @see https://w3c.github.io/mathml-core/#fractions-mfrac
  class RenderMathMLFraction final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLFraction);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLFraction);

  public:
    RenderMathMLFraction(mathml::MathMLFractionElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLFraction() noexcept override;

    KRYS_NODISCARD LayoutUnit DefaultLineThickness() const noexcept;
    KRYS_NODISCARD LayoutUnit LineThickness() const noexcept;
    KRYS_NODISCARD float RelativeLineThickness() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLFraction";
    }

  private:
    struct FractionParameters
    {
      LayoutUnit NumeratorShiftUp;
      LayoutUnit DenominatorShiftDown;
    };

    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;
    KRYS_NODISCARD RenderMathMLOperator *UnembellishedOperator() const noexcept final;
    KRYS_NODISCARD bool IsMathContentCentered() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD mathml::MathMLFractionElement &GetElement() const noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD RenderBox &Numerator() const noexcept;
    KRYS_NODISCARD RenderBox &Denominator() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalOffset(const RenderBox &child) const noexcept;
    KRYS_NODISCARD FractionParameters GetFractionParameters() const noexcept;
    KRYS_NODISCARD FractionParameters GetStackParameters() const noexcept;
    KRYS_NODISCARD LayoutUnit FractionAscent() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLFraction)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLFraction();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
