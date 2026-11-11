/*
 * Copyright (C) 2021-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/Painting/MarkedText.hpp"
#include "Krystal.Booey/Rendering/Painting/TextDecorationPainter.hpp"
#include "Krystal.Booey/Rendering/Selection/TextBoxSelectableRange.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderStyle;
  struct CompositionUnderline;
  class RenderText;
  class TextPainter;
  struct PaintInfo;
  struct StyledMarkedText;

  namespace dom
  {
    class Document;
  }

  namespace inline_display
  {
    struct Box;
  }

  namespace layout_integration
  {
    class InlineContent;
  }

  /// @brief Paints a text box: its text, with shadows, emphasis marks, decorations, highlights, the selection
  /// and an input method's composition (its underlines, or else its fill).
  /// FIXME: Event regions and contentful paint tracking come with event regions and paint timing. There are
  /// no document markers (see DIFFERENCES.md).
  class TextBoxPainter
  {
    KRYS_NON_COPY_MOVABLE(TextBoxPainter);

  protected:
    // FIXME: We could just talk to the display box directly.
    const inline_iterator::BoxModernPath _textBox;
    const RenderText &_renderer;
    const dom::Document &_document;
    const RenderStyle &_style;
    const FloatRect _logicalRect;
    const TextRun _paintTextRun;
    PaintInfo &_paintInfo;
    const TextBoxSelectableRange _selectableRange;
    const LayoutPoint _paintOffset;
    const FloatRect _paintRect;
    const bool _isCombinedText;
    const bool _isFirstLine;
    const bool _isPrinting;
    const bool _haveSelection;
    bool _containsComposition {false};
    bool _compositionWithCustomUnderlines {false};

  public:
    TextBoxPainter(const layout_integration::InlineContent &inlineContent, const inline_display::Box &box,
                   const RenderStyle &style, PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;
    ~TextBoxPainter() noexcept;

    void Paint() noexcept;

    /// @brief A shadow's offset turned into the line's coordinates under @p writingMode.
    KRYS_NODISCARD static FloatSize
      RotateShadowOffset(const css::SpaceSeparatedPoint<style::Length<css::AllUnzoomed>> &offset,
                         WritingMode writingMode, const style::ZoomFactor &zoomFactor) noexcept;

  protected:
    KRYS_NODISCARD const inline_iterator::BoxModernPath &TextBox() const noexcept
    {
      return _textBox;
    }
    KRYS_NODISCARD inline_iterator::TextBoxIterator MakeIterator() const noexcept;

    void PaintBackgroundFill() noexcept;
    void PaintForegroundAndDecorations() noexcept;
    void PaintCompositionUnderlines() noexcept;
    void PaintCompositionUnderline(const CompositionUnderline &underline) noexcept;
    void FillCompositionUnderline(float start, float width,
                                  const CompositionUnderline &underline) const noexcept;
    /// @brief Where the box starts from its line's content start, as text widths are measured.
    KRYS_NODISCARD float TextPosition() const noexcept;

    /// @brief The range of the box's text the selection covers.
    KRYS_NODISCARD std::pair<uint32, uint32> SelectionStartEnd() const noexcept;
    KRYS_NODISCARD MarkedText CreateMarkedTextFromSelectionInBox() const noexcept;
    /// @brief The rect of the box's text from @p startOffset to @p endOffset, as tall as the line's
    /// selection.
    KRYS_NODISCARD LayoutRect SelectionRectForRange(uint32 startOffset, uint32 endOffset) const noexcept;
    void PaintBackgroundFillForRange(uint32 startOffset, uint32 endOffset, const Color &color) noexcept;
    KRYS_NODISCARD bool ComputeHaveSelection() const noexcept;
    void PaintForeground(const StyledMarkedText &markedText) noexcept;
    KRYS_NODISCARD bool PaintForegroundForShapeRange(TextPainter &textPainter) noexcept;
    KRYS_NODISCARD TextDecorationPainter CreateDecorationPainter(const StyledMarkedText &markedText,
                                                                 const FloatRect &clipOutRect) noexcept;
    void PaintBackgroundDecorations(TextDecorationPainter &decorationPainter,
                                    const StyledMarkedText &markedText,
                                    const FloatRect &textBoxPaintRect) noexcept;
    void PaintForegroundDecorations(TextDecorationPainter &decorationPainter,
                                    const StyledMarkedText &markedText,
                                    const FloatRect &textBoxPaintRect) noexcept;

    KRYS_NODISCARD FloatRect ComputePaintRect(const LayoutPoint &paintOffset) const noexcept;
    KRYS_NODISCARD const FontCascade &GetFontCascade() const noexcept;
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;
    KRYS_NODISCARD FloatPoint TextOriginFromPaintRect(const FloatRect &paintRect) const noexcept;
    KRYS_NODISCARD bool IsInsideShapedContent() const noexcept;

    struct DecoratingBox
    {
      inline_iterator::InlineBoxIterator InlineBox;
      const RenderStyle &Style;
      TextDecorationPainter::Styles TextDecorationStyles;
      FloatPoint Location;
    };
    using DecoratingBoxList = List<DecoratingBox>;
    /// @brief The inline boxes whose underlines and overlines @p textBox paints: its parent and the ancestors
    /// that decorate it, innermost first.
    void CollectDecoratingBoxesForBackgroundPainting(
      DecoratingBoxList &decoratingBoxList, const inline_iterator::TextBoxIterator &textBox,
      FloatPoint textBoxLocation, const TextDecorationPainter::Styles &overrideDecorationStyle) noexcept;
  };
}
