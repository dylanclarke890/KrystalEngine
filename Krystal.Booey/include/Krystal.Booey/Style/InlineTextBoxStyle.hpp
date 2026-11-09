/*
 * Copyright (C) 2014-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderStyle;
  class RenderText;

  namespace inline_iterator
  {
    class InlineBox;
    class LineBoxIterator;
  }

  KRYS_NODISCARD inline float WavyOffsetFromDecoration() noexcept
  {
    return 1.f;
  }

  struct WavyStrokeParameters
  {
    // Distance between decoration's axis and Bezier curve's control points.
    // The height of the curve is based on this distance. Increases the curve's height
    // as fontSize increases to make the curve look better.
    float ControlPointDistance {0.f};

    // Increment used to form the diamond shape between start point (p1), control
    // points and end point (p2) along the axis of the decoration. The curve gets
    // wider as font size increases.
    float Step {0.f};
  };

  KRYS_NODISCARD WavyStrokeParameters GetWavyStrokeParameters(float fontSize) noexcept;

  struct TextUnderlinePositionUnder
  {
    float TextRunLogicalHeight {0.f};
    // This offset value is the distance between the current text run's logical bottom and the lowest position
    // of all the text runs on line that belong to the same decorating box.
    float TextRunOffsetFromBottomMost {0.f};
  };

  /// @brief The ink overflow of the text decorations of text with @p style.
  KRYS_NODISCARD GlyphOverflow InkOverflowForDecorations(const RenderStyle &style) noexcept;
  KRYS_NODISCARD GlyphOverflow InkOverflowForDecorations(
    const RenderStyle &style, TextUnderlinePositionUnder textUnderlinePositionUnder) noexcept;
  /// @brief The ink overflow of the text decorations of @p renderer's text box from @p textBoxLogicalTop to
  /// @p textBoxLogicalBottom on @p lineBox.
  KRYS_NODISCARD GlyphOverflow InkOverflowForDecorations(const inline_iterator::LineBoxIterator &lineBox,
                                                         const RenderText &renderer, float textBoxLogicalTop,
                                                         float textBoxLogicalBottom) noexcept;

  /// @brief Whether underlines are aligned as for `text-underline-position: under` in a decorating box with
  /// @p decoratingBoxStyle.
  KRYS_NODISCARD bool IsAlignedForUnder(const RenderStyle &decoratingBoxStyle) noexcept;

  /// @brief How far below the top of @p inlineBox's content an underline of its decorations (with @p style)
  /// is drawn.
  KRYS_NODISCARD float UnderlineOffsetForTextBoxPainting(const inline_iterator::InlineBox &inlineBox,
                                                         const RenderStyle &style) noexcept;
  /// @brief How far below the top of @p inlineBox's content an overline of its decorations (with @p style) is
  /// drawn: only moved in vertical typographic modes.
  KRYS_NODISCARD float OverlineOffsetForTextBoxPainting(const inline_iterator::InlineBox &inlineBox,
                                                        const RenderStyle &style) noexcept;
  KRYS_NODISCARD float TextBoxEdgeAdjustmentForUnderline(const RenderStyle &style) noexcept;
}
