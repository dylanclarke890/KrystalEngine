#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LineSelection.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  int32 CaretWidth() noexcept
  {
    return 1;
  }

  namespace
  {
    KRYS_NODISCARD LayoutRect ComputeCaretRectForEmptyElement(const RenderBoxModelObject &renderer,
                                                              LayoutUnit logicalWidth,
                                                              LayoutUnit textIndentOffset,
                                                              CaretRectMode caretRectMode) noexcept
    {
      krys_debug_assert(!renderer.FirstChild() || renderer.FirstChild()->IsPseudoElement());

      // FIXME: This does not take into account either :first-line or :first-letter
      // However, as soon as some content is entered, the line boxes will be
      // constructed and this kludge is not called any more. So only the caret size
      // of an empty :first-line'd block is wrong. I think we can live with that.
      const RenderStyle &currentStyle = renderer.FirstLineStyle();
      const WritingMode writingMode = currentStyle.GetWritingMode();

      enum CaretAlignment
      {
        AlignLogicalLeft,
        AlignLogicalRight,
        AlignCenter
      };
      CaretAlignment alignment = AlignLogicalLeft;
      switch (currentStyle.GetTextAlign())
      {
        case style::TextAlign::Left:
        case style::TextAlign::WebKitLeft:
          alignment = writingMode.IsLogicalLeftLineLeft() ? AlignLogicalLeft : AlignLogicalRight;
          break;
        case style::TextAlign::Center:
        case style::TextAlign::WebKitCenter: alignment = AlignCenter; break;
        case style::TextAlign::Right:
        case style::TextAlign::WebKitRight:
          alignment = writingMode.IsLogicalLeftLineLeft() ? AlignLogicalRight : AlignLogicalLeft;
          break;
        case style::TextAlign::Justify:
        case style::TextAlign::Start:
          alignment = writingMode.IsLogicalLeftInlineStart() ? AlignLogicalLeft : AlignLogicalRight;
          break;
        case style::TextAlign::End:
          alignment = writingMode.IsLogicalLeftInlineStart() ? AlignLogicalRight : AlignLogicalLeft;
          break;
      }

      LayoutUnit x = renderer.BorderAndPaddingLogicalLeft();
      LayoutUnit maxX = logicalWidth - renderer.BorderAndPaddingLogicalRight();

      switch (alignment)
      {
        case AlignLogicalLeft:
          if (writingMode.IsLogicalLeftInlineStart())
          {
            x += textIndentOffset;
          }
          break;
        case AlignCenter:
          x = (x + maxX) / 2;
          if (writingMode.IsLogicalLeftInlineStart())
          {
            x += textIndentOffset / 2;
          }
          else
          {
            x -= textIndentOffset / 2;
          }
          break;
        case AlignLogicalRight:
          x = maxX - CaretWidth();
          if (!writingMode.IsLogicalLeftInlineStart())
          {
            x -= textIndentOffset;
          }
          break;
      }
      x = std::min(x, std::max<LayoutUnit>(maxX - CaretWidth(), 0));

      auto lineHeight = LayoutUnit::FromFloatCeil(currentStyle.ComputedLineHeight());
      auto height = std::min(lineHeight, LayoutUnit {currentStyle.MetricsOfPrimaryFont().Height()});
      auto y =
        renderer.BorderAndPaddingBefore() + (lineHeight > height ? (lineHeight - height) / 2 : LayoutUnit {});

      auto rect = LayoutRect(x, y, LayoutUnit(CaretWidth()), height);

      if (caretRectMode == CaretRectMode::ExpandToEndOfLine)
      {
        rect.ShiftMaxXEdgeTo(logicalWidth);
      }

      return writingMode.IsHorizontal() ? rect : rect.TransposedRect();
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForLinePosition(const inline_iterator::LineBoxIterator &lineBox,
                                                              float logicalLeftPosition,
                                                              CaretRectMode caretRectMode) noexcept
    {
      auto &root = lineBox->FormattingContextRoot();
      auto writingMode = root.GetWritingMode();
      auto lineSelectionRect = LineSelection::LogicalRect(*lineBox);

      int32 height = static_cast<int32>(lineSelectionRect.Height());
      int32 top = static_cast<int32>(lineSelectionRect.Y());

      // Distribute the caret's width to either side of the offset.
      float left = logicalLeftPosition;
      int32 caretWidthLeftOfOffset = CaretWidth() / 2;
      left -= static_cast<float>(caretWidthLeftOfOffset);
      int32 caretWidthRightOfOffset = CaretWidth() - caretWidthLeftOfOffset;
      left = std::round(left);

      float lineLeft = lineSelectionRect.X();
      float lineRight = lineSelectionRect.MaxX();

      bool rightAligned = false;
      switch (root.Style().GetTextAlign())
      {
        case style::TextAlign::Right:
        case style::TextAlign::WebKitRight:  rightAligned = writingMode.IsLogicalLeftLineLeft(); break;
        case style::TextAlign::Left:
        case style::TextAlign::WebKitLeft:
        case style::TextAlign::Center:
        case style::TextAlign::WebKitCenter: rightAligned = !writingMode.IsLogicalLeftLineLeft(); break;
        case style::TextAlign::Justify:
        case style::TextAlign::Start:        rightAligned = !writingMode.IsLogicalLeftInlineStart(); break;
        case style::TextAlign::End:          rightAligned = writingMode.IsLogicalLeftInlineStart(); break;
      }

      float leftEdge = std::min<float>(0, lineLeft);
      float rightEdge = std::max<float>(root.LogicalWidth().ToFloat(), lineRight);

      if (rightAligned)
      {
        left = std::max(left, leftEdge);
        left = std::min(left, lineRight - static_cast<float>(CaretWidth()));
      }
      else
      {
        left = std::min(left, rightEdge - static_cast<float>(caretWidthRightOfOffset));
        left = std::max(left, lineLeft);
      }

      auto rect = IntRect(static_cast<int32>(left), top, CaretWidth(), height);

      if (caretRectMode == CaretRectMode::ExpandToEndOfLine)
      {
        rect.ShiftMaxXEdgeTo(static_cast<int32>(lineRight));
      }

      return LayoutRect(writingMode.IsHorizontal() ? rect : rect.TransposedRect());
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForSVGInlineText(const InlineBoxAndOffset &boxAndOffset,
                                                               CaretRectMode) noexcept
    {
      auto &box = boxAndOffset.Box;
      auto caretOffset = boxAndOffset.Offset;
      if (!box || !box->IsSVGText())
      {
        return {};
      }

      inline_iterator::SVGTextBoxIterator textBox(*box);
      if (!textBox)
      {
        return {};
      }

      if (caretOffset < textBox->Start() || caretOffset > textBox->Start() + textBox->Length())
      {
        return {};
      }

      const auto &text = textBox->Renderer().Text();

      // Use the edge of the selection rect to determine the caret rect.
      if (caretOffset < textBox->Start() + textBox->Length())
      {
        auto next =
          caretOffset
          + krys::text::DecodeUTF8(Span<const char8>(text.data() + caretOffset, text.size() - caretOffset))
              .Length;
        LayoutRect rect = textBox->LocalSelectionRect(caretOffset, next);
        LayoutUnit x = !textBox->IsInlineFlipped() ? rect.X() : rect.MaxX();
        return LayoutRect(x, rect.Y(), CaretWidth(), rect.Height());
      }

      auto previous = static_cast<uint32>(krys::text::PreviousCodePointStart(text, caretOffset));
      LayoutRect rect = textBox->LocalSelectionRect(previous, caretOffset);
      LayoutUnit x = !textBox->IsInlineFlipped() ? rect.MaxX() : rect.X();
      return {x, rect.Y(), CaretWidth(), rect.Height()};
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForText(const InlineBoxAndOffset &boxAndOffset,
                                                      CaretRectMode caretRectMode) noexcept
    {
      if (!boxAndOffset.Box)
      {
        return {};
      }

      inline_iterator::TextBoxIterator textBox(*boxAndOffset.Box);

      auto positionForOffset = [&](uint32 offset) -> float
      {
        krys_debug_assert(offset >= textBox->Start());
        krys_debug_assert(offset <= textBox->End());

        if (textBox->IsLineBreak())
        {
          return 0;
        }

        auto selectableRange = textBox->SelectableRange();
        auto [startOffset, endOffset] = [&]
        {
          if (textBox->Direction() == TextDirection::RTL)
          {
            return std::pair {selectableRange.Clamp(offset), textBox->Length()};
          }
          return std::pair {0u, selectableRange.Clamp(offset)};
        }();

        LayoutRect selectionRect;
        // Get logical x coordinate relative to text run.
        auto textRun = textBox->GetTextRun(inline_iterator::TextRunMode::Editing);
        textBox->GetFontCascade().AdjustSelectionRectForText(
          textBox->Renderer().CanUseSimplifiedTextMeasuring().value_or(false), textRun, selectionRect,
          startOffset, endOffset);
        selectionRect.ShiftXEdgeTo(selectionRect.MaxX());

        // Convert to box coordinates.
        if (!textBox->GetWritingMode().IsLogicalLeftLineLeft())
        {
          selectionRect.SetX(LayoutUnit(textBox->LogicalWidth()) - selectionRect.X());
        }
        selectionRect.Move(LayoutUnit(textBox->LogicalLeftIgnoringInlineDirection()), 0_lu);

        // Finally, snap.
        return SnapRectToDevicePixelsWithWritingDirection(
                 selectionRect, textBox->Renderer().GetDocument().DeviceScaleFactor(), textRun.Ltr())
          .X();
      };

      return ComputeCaretRectForLinePosition(textBox->GetLineBox(), positionForOffset(boxAndOffset.Offset),
                                             caretRectMode);
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForLineBreak(const InlineBoxAndOffset &boxAndOffset,
                                                           CaretRectMode caretRectMode) noexcept
    {
      krys_debug_assert(!boxAndOffset.Offset);

      if (!boxAndOffset.Box)
      {
        return {};
      }

      auto lineBox = boxAndOffset.Box->GetLineBox();
      auto position = boxAndOffset.Box->GetWritingMode().IsLogicalLeftLineLeft()
                        ? lineBox->ContentLogicalLeft()
                        : lineBox->ContentLogicalRight();
      return ComputeCaretRectForLinePosition(lineBox, position, caretRectMode);
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForBox(const RenderBox &renderer,
                                                     const InlineBoxAndOffset &boxAndOffset,
                                                     CaretRectMode caretRectMode) noexcept
    {
      // VisiblePositions at offsets inside containers either a) refer to the positions before/after
      // those containers (tables and select elements) or b) refer to the position inside an empty block.
      // They never refer to children.
      // FIXME: Paint the carets inside empty blocks differently than the carets before/after elements.

      LayoutRect rect(renderer.Location(), LayoutSize(LayoutUnit(CaretWidth()), renderer.Height()));
      auto writingMode = boxAndOffset.Box ? boxAndOffset.Box->GetWritingMode() : renderer.GetWritingMode();

      if ((!boxAndOffset.Offset) == writingMode.IsInlineFlipped())
      {
        rect.Move(LayoutSize(renderer.Width() - CaretWidth(), 0_lu));
      }

      if (boxAndOffset.Box)
      {
        auto lineBox = boxAndOffset.Box->GetLineBox();
        auto top = lineBox->ContentLogicalTop();
        rect.SetY(LayoutUnit(top));
        rect.SetHeight(LayoutUnit(lineBox->ContentLogicalBottom() - top));
      }

      // If height of box is smaller than font height, use the latter one,
      // otherwise the caret might become invisible.
      //
      // Also, if the box is not a replaced element, always use the font height.
      // This prevents the "big caret" bug described in:
      // <rdar://problem/3777804> Deleting all content in a document can result in giant tall-as-window
      // insertion point
      //
      // FIXME: ignoring :first-line, missing good reason to take care of
      auto fontHeight = LayoutUnit(renderer.Style().MetricsOfPrimaryFont().Height());
      if (fontHeight > rect.Height()
          || (!renderer.IsBlockLevelReplacedOrAtomicInline() && !renderer.IsRenderTable()))
      {
        rect.SetHeight(fontHeight);
      }

      // Move to local coords
      rect.MoveBy(-renderer.Location());

      // FIXME: Border/padding should be added for all elements but this workaround
      // is needed because we use offsets inside an "atomic" element to represent
      // positions before and after the element in deprecated editing offsets.
      if (auto *element = renderer.GetElement();
          element && !(EditingIgnoresContent(*element) || IsRenderedTable(element)))
      {
        rect.SetX(rect.X() + renderer.BorderLeft() + renderer.PaddingLeft());
        rect.SetY(rect.Y() + renderer.PaddingTop() + renderer.BorderTop());
      }

      if (caretRectMode == CaretRectMode::ExpandToEndOfLine)
      {
        rect.ShiftMaxXEdgeTo(renderer.X() + renderer.Width());
      }

      return writingMode.IsHorizontal() ? rect : rect.TransposedRect();
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForBlock(const RenderBlock &renderer,
                                                       const InlineBoxAndOffset &boxAndOffset,
                                                       CaretRectMode caretRectMode) noexcept
    {
      // Do the normal calculation in most cases.
      if (renderer.FirstChild() && !renderer.FirstChild()->IsPseudoElement())
      {
        return ComputeCaretRectForBox(renderer, boxAndOffset, caretRectMode);
      }

      return ComputeCaretRectForEmptyElement(renderer, renderer.LogicalWidth(), renderer.TextIndentOffset(),
                                             caretRectMode);
    }

    KRYS_NODISCARD LayoutRect ComputeCaretRectForInline(const RenderInline &renderer) noexcept
    {
      if (renderer.FirstChild())
      {
        // This condition is possible if the RenderInline is at an editing boundary,
        // i.e. the VisiblePosition is:
        //   <RenderInline editingBoundary=true>|<RenderText> </RenderText></RenderInline>
        // FIXME: need to figure out how to make this return a valid rect, note that
        // there are no line boxes created in the above case.
        return {};
      }

      LayoutRect caretRect = ComputeCaretRectForEmptyElement(
        renderer, renderer.BorderAndPaddingLogicalWidth(), 0_lu, CaretRectMode::Normal);

      if (auto firstInlineBox = inline_iterator::LineLeftmostInlineBoxFor(renderer))
      {
        caretRect.MoveBy(LayoutPoint {firstInlineBox->VisualRectIgnoringBlockDirection().Location()});
      }

      return caretRect;
    }
  }

  LayoutRect ComputeLocalCaretRect(const RenderObject &renderer, const InlineBoxAndOffset &boxAndOffset,
                                   CaretRectMode caretRectMode) noexcept
  {
    if (Is<RenderSVGInlineText>(renderer))
    {
      return ComputeCaretRectForSVGInlineText(boxAndOffset, caretRectMode);
    }

    if (Is<RenderText>(renderer))
    {
      return ComputeCaretRectForText(boxAndOffset, caretRectMode);
    }

    if (Is<RenderLineBreak>(renderer))
    {
      return ComputeCaretRectForLineBreak(boxAndOffset, caretRectMode);
    }

    if (auto *block = DynamicDowncast<RenderBlock>(renderer))
    {
      return ComputeCaretRectForBlock(*block, boxAndOffset, caretRectMode);
    }

    if (auto *box = DynamicDowncast<RenderBox>(renderer))
    {
      return ComputeCaretRectForBox(*box, boxAndOffset, caretRectMode);
    }

    if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
    {
      return ComputeCaretRectForInline(*renderInline);
    }

    return {};
  }
}
