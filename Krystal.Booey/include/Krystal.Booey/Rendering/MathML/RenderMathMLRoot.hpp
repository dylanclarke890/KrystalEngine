/*
 * Copyright (C) 2009 Alex Milowski (alex@milowski.com). All rights reserved.
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

#include "Krystal.Booey/Rendering/MathML/MathOperator.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLRootElement;
    enum class RootType : uint8;
  }

  /// @brief The renderer of `<msqrt>` and `<mroot>`: base^(1/index), or sqrt(base), in radical notation.
  /// @see https://w3c.github.io/mathml-core/#radicals-msqrt-mroot
  class RenderMathMLRoot final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLRoot);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLRoot);

  private:
    struct HorizontalParameters
    {
      LayoutUnit KernBeforeDegree;
      LayoutUnit KernAfterDegree;
    };

    struct VerticalParameters
    {
      LayoutUnit VerticalGap;
      LayoutUnit RuleThickness;
      LayoutUnit ExtraAscender;
      float DegreeBottomRaisePercent {0};
    };

    MathOperator _radicalOperator;
    LayoutUnit _radicalOperatorTop;
    LayoutUnit _baseWidth;

  public:
    RenderMathMLRoot(mathml::MathMLRootElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLRoot() noexcept override;

    void ResetRadicalOperator() noexcept
    {
      _radicalOperator.Reset(Style());
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLRoot";
    }

  private:
    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD RenderBox &GetBase() const noexcept;
    KRYS_NODISCARD RenderBox &GetIndex() const noexcept;
    KRYS_NODISCARD mathml::MathMLRootElement &GetElement() const noexcept;
    KRYS_NODISCARD mathml::RootType GetRootType() const noexcept;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;

    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    void Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept final;

    KRYS_NODISCARD HorizontalParameters GetHorizontalParameters(LayoutUnit indexWidth) const noexcept;
    KRYS_NODISCARD VerticalParameters GetVerticalParameters() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLRoot)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLRoot();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
