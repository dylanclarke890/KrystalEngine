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

#include "Krystal.Booey/Rendering/MathML/RenderMathMLScripts.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLUnderOverElement;
  }

  /// @brief The renderer of `<munder>`, `<mover>` and `<munderover>`: a base with scripts centred below and
  /// above it, which move to its side as sub- and superscripts when its operator's limits move.
  /// @see https://w3c.github.io/mathml-core/#underscripts-and-overscripts-munder-mover-munderover
  class RenderMathMLUnderOver final : public RenderMathMLScripts
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLUnderOver);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLUnderOver);

  public:
    RenderMathMLUnderOver(mathml::MathMLUnderOverElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLUnderOver() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLUnderOver";
    }

  private:
    struct VerticalParameters
    {
      bool UseUnderOverBarFallBack {false};
      LayoutUnit UnderGapMin;
      LayoutUnit OverGapMin;
      LayoutUnit UnderShiftMin;
      LayoutUnit OverShiftMin;
      LayoutUnit UnderExtraDescender;
      LayoutUnit OverExtraAscender;
      LayoutUnit AccentBaseHeight;
    };

    KRYS_NODISCARD bool IsMathContentCentered() const noexcept final
    {
      return !ShouldMoveLimits();
    }
    KRYS_NODISCARD mathml::MathMLUnderOverElement &GetElement() const noexcept;

    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;

    void StretchHorizontalOperatorsAndLayoutChildren() noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD bool ShouldMoveLimits() const noexcept;
    KRYS_NODISCARD RenderBox &Base() const noexcept;
    KRYS_NODISCARD RenderBox &Under() const noexcept;
    KRYS_NODISCARD RenderBox &Over() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalOffset(const RenderBox &child) const noexcept;
    KRYS_NODISCARD bool HasAccent(bool accentUnder = false) const noexcept;
    KRYS_NODISCARD bool HasAccentUnder() const noexcept
    {
      return HasAccent(true);
    }
    KRYS_NODISCARD VerticalParameters GetVerticalParameters() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLUnderOver)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLUnderOver();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
