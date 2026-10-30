#include "Krystal.Booey/Rendering/TextBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/CompositionHighlight.hpp"
#include "Krystal.Booey/Editing/CompositionUnderline.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LineSelection.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/StyledMarkedText.hpp"
#include "Krystal.Booey/Rendering/TextPainter.hpp"
#include "Krystal.Booey/Rendering/TextPaintStyle.hpp"
#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>
#include <ranges>

namespace krys::boo
{
  // This is temporary and will be removed when subpixel inline layout is enabled.
  KRYS_NODISCARD static float Snap(float value) noexcept
  {
    return std::round(value);
  }

  KRYS_NODISCARD static Maybe<bool> EmphasisMarkExistsAndIsAbove(const RenderText &renderer,
                                                                 const RenderStyle &style) noexcept
  {
    // This function returns true if there are text emphasis marks and they are suppressed by ruby text.
    if (style.GetTextEmphasisStyle().IsNone())
    {
      return {};
    }

    auto emphasisPosition = style.GetTextEmphasisPosition();
    bool isAbove = !emphasisPosition.Contains(style::TextEmphasisPositionValue::Under);
    if (style.GetWritingMode().IsVerticalTypographic())
    {
      isAbove = !emphasisPosition.Contains(style::TextEmphasisPositionValue::Left);
    }

    auto findRubyAnnotation = [&]() -> const RenderBlockFlow *
    {
      for (auto *baseCandidate = renderer.Parent(); baseCandidate; baseCandidate = baseCandidate->Parent())
      {
        if (!baseCandidate->IsInline())
        {
          return {};
        }
        if (baseCandidate->Style().Display() == DisplayType::RubyBase)
        {
          if (auto *annotationCandidate = DynamicDowncast<RenderBlockFlow>(baseCandidate->NextSibling());
              annotationCandidate && annotationCandidate->Style().Display() == DisplayType::RubyAnnotation)
          {
            return annotationCandidate;
          }
          return {};
        }
      }
      return {};
    };

    if (auto *annotation = findRubyAnnotation())
    {
      // The emphasis marks are suppressed only if there is a ruby annotation box on the same side and it is
      // not empty.
      if (annotation->HasContentfulInlineLine()
          && isAbove == (annotation->Style().GetRubyPosition() == RubyPosition::Over))
      {
        return {};
      }
    }

    return isAbove;
  }

  namespace
  {
    struct ShapedContent
    {
      StringBuilder Text;
      float VisualLeft {0.f};        // visual left of the shaped content.
      size_t TextBoxStartOffset {0}; // text box's position relative to the shaped content.
      float TextBoxVisualLeft {0.f}; // text box's left relative to the visual left of the shaped content.
    };
  }

  static void BuildTextForShaping(ShapedContent &shapedContent, inline_iterator::BoxModernPath textBox,
                                  bool needsTextBoxVisualLeft = false) noexcept
  {
    krys_debug_assert(textBox.Direction() == TextDirection::RTL);

    auto shapingBoundaryIterator = textBox;
    // 1. Find shaping boundary start when we are at the end or inside a shape range (note that we deal with
    // rtl content hence the opposite direction walk)
    // 2. Walk from the start to the end and build the text content.

    auto moveToShapingBoundaryStart = [&]
    {
      if (shapingBoundaryIterator.GetBox().GetText().IsAtShapingBoundaryStart())
      {
        return;
      }

      shapingBoundaryIterator.TraverseNextLeafOnLine();
      for (; !shapingBoundaryIterator.AtEnd(); shapingBoundaryIterator.TraverseNextLeafOnLine())
      {
        auto &displayBox = shapingBoundaryIterator.GetBox();
        if (displayBox.IsText())
        {
          shapedContent.TextBoxStartOffset += displayBox.GetText().Length();
          if (displayBox.GetText().IsAtShapingBoundaryStart())
          {
            break;
          }
        }
      }
    };
    moveToShapingBoundaryStart();

    if (shapingBoundaryIterator.AtEnd() || !shapingBoundaryIterator.IsText())
    {
      krys_debug_assert(false);
      return;
    }

    auto buildTextContent = [&]
    {
      for (; !shapingBoundaryIterator.AtEnd(); shapingBoundaryIterator.TraversePreviousLeafOnLine())
      {
        auto &displayBox = shapingBoundaryIterator.GetBox();
        if (!displayBox.IsText())
        {
          continue;
        }
        auto &text = displayBox.GetText();
        if (shapingBoundaryIterator.Direction() == TextDirection::LTR)
        {
          shapedContent.Text.Clear();
          return;
        }

        shapedContent.Text.Append(text.RenderedContent());
        if (text.IsAtShapingBoundaryEnd())
        {
          shapedContent.VisualLeft = displayBox.VisualRectIgnoringBlockDirection().X();
          return;
        }
      }
      // We should always find the boundary end.
      krys_debug_assert(false);
      shapedContent.Text.Clear();
    };
    buildTextContent();

    if (shapedContent.Text.IsEmpty())
    {
      krys_debug_assert(false);
      return;
    }

    auto computeVisualLeftForTextBox = [&]
    {
      if (!needsTextBoxVisualLeft)
      {
        return;
      }
      // Starting from visual left, walk all the way to the current text box.
      for (; !shapingBoundaryIterator.AtEnd(); shapingBoundaryIterator.TraverseNextLeafOnLine())
      {
        if (shapingBoundaryIterator == textBox)
        {
          return;
        }
        auto &displayBox = shapingBoundaryIterator.GetBox();
        if (displayBox.IsText())
        {
          shapedContent.TextBoxVisualLeft += displayBox.VisualRectIgnoringBlockDirection().Width();
        }
      }
    };
    computeVisualLeftForTextBox();
  }

  TextBoxPainter::TextBoxPainter(const layout_integration::InlineContent &inlineContent,
                                 const inline_display::Box &box, const RenderStyle &style,
                                 PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
      : _textBox(inline_iterator::BoxModernPath {inlineContent, inlineContent.IndexForBox(box)}),
        _renderer(Downcast<RenderText>(_textBox.Renderer())), _document(_renderer.GetDocument()),
        _style(style),
        _logicalRect(_textBox.IsHorizontal() ? _textBox.VisualRectIgnoringBlockDirection()
                                             : _textBox.VisualRectIgnoringBlockDirection().TransposedRect()),
        _paintTextRun(_textBox.GetTextRun()), _paintInfo(paintInfo),
        _selectableRange(_textBox.SelectableRange()), _paintOffset(paintOffset),
        _paintRect(ComputePaintRect(paintOffset)), _isFirstLine(_textBox.IsFirstFormattedLine()),
        _isPrinting(_document.Printing()), _haveSelection(ComputeHaveSelection())
  {
    krys_debug_assert(paintInfo.Phase == PaintPhase::Foreground || paintInfo.Phase == PaintPhase::Selection
                      || paintInfo.Phase == PaintPhase::TextClip || paintInfo.Phase == PaintPhase::EventRegion
                      || paintInfo.Phase == PaintPhase::Accessibility);

    auto &editor = _renderer.GetDocument().GetEditor();
    _containsComposition = _renderer.TextNode() && editor.CompositionNode() == _renderer.TextNode();
    _compositionWithCustomUnderlines = _containsComposition && editor.CompositionUsesCustomUnderlines();
  }

  TextBoxPainter::~TextBoxPainter() noexcept = default;

  inline_iterator::TextBoxIterator TextBoxPainter::MakeIterator() const noexcept
  {
    auto pathCopy = _textBox;
    return inline_iterator::TextBoxIterator {krys::move(pathCopy)};
  }

  void TextBoxPainter::Paint() noexcept
  {
    if (_paintInfo.PaintBehavior.Contains(PaintBehavior::ExcludeText))
    {
      return;
    }

    if (_paintInfo.Phase == PaintPhase::Selection && !_haveSelection)
    {
      return;
    }

    if (_paintInfo.Phase == PaintPhase::EventRegion)
    {
      // FIXME: Event regions come with event handling.
      return;
    }

    Maybe<RotationDirection> glyphRotation;
    if (!TextBox().IsHorizontal())
    {
      glyphRotation = TextBox().GetWritingMode().IsLineOverLeft() ? RotationDirection::Counterclockwise
                                                                  : RotationDirection::Clockwise;
      _paintInfo.Context().ConcatCTM(Rotation(_paintRect, *glyphRotation));
    }

    if (_paintInfo.Phase == PaintPhase::Accessibility)
    {
      // FIXME: Accessibility bounds come with accessibility.
    }
    else
    {
      if (_paintInfo.Phase == PaintPhase::Foreground)
      {
        // FIXME: The background fill of document markers, and platform document markers (spelling and grammar
        // errors), come with document markers.
        auto shouldPaintBackgroundFill = [&]
        {
          if (_isPrinting)
          {
            return false;
          }
          if (_haveSelection && !_compositionWithCustomUnderlines)
          {
            return true;
          }
          if (_containsComposition && !_compositionWithCustomUnderlines)
          {
            return true;
          }
          if (_document.HasHighlight())
          {
            return true;
          }
          return false;
        };
        if (shouldPaintBackgroundFill())
        {
          PaintBackgroundFill();
        }
      }

      PaintForegroundAndDecorations();

      if (_paintInfo.Phase == PaintPhase::Foreground && _compositionWithCustomUnderlines)
      {
        PaintCompositionUnderlines();
      }
      // FIXME: The page's relevant repainted objects and the document's painted text tracking (didPaintText)
      // come with paint milestones.
    }

    if (glyphRotation)
    {
      auto backRotation = *glyphRotation == RotationDirection::Clockwise ? RotationDirection::Counterclockwise
                                                                         : RotationDirection::Clockwise;
      _paintInfo.Context().ConcatCTM(Rotation(_paintRect, backRotation));
    }
  }

  std::pair<uint32, uint32> TextBoxPainter::SelectionStartEnd() const noexcept
  {
    return _renderer.View().Selection().RangeForTextBox(_renderer, _selectableRange);
  }

  MarkedText TextBoxPainter::CreateMarkedTextFromSelectionInBox() const noexcept
  {
    auto [selectionStart, selectionEnd] = SelectionStartEnd();
    if (selectionStart < selectionEnd)
    {
      return {selectionStart, selectionEnd, MarkedText::Type::Selection};
    }
    return {};
  }

  // FIXME: Document markers, and the composition's custom highlights, with those.
  void TextBoxPainter::PaintBackgroundFill() noexcept
  {
    if (_containsComposition && !_compositionWithCustomUnderlines)
    {
      auto &editor = _renderer.GetDocument().GetEditor();
      auto [clampedStart, clampedEnd] =
        _selectableRange.Clamp(editor.CompositionStart(), editor.CompositionEnd());
      PaintBackgroundFillForRange(clampedStart, clampedEnd,
                                  CompositionHighlight::DefaultCompositionFillColor);
    }

    auto markedTexts =
      MarkedText::CollectForHighlights(_renderer, _selectableRange, MarkedText::PaintPhase::Background);

    auto hasSelectionWithNonCustomUnderline = _haveSelection && !_compositionWithCustomUnderlines;
    if (hasSelectionWithNonCustomUnderline && !_paintInfo.Context().PaintingDisabled())
    {
      auto selectionMarkedText = CreateMarkedTextFromSelectionInBox();
      if (!selectionMarkedText.IsEmpty())
      {
        markedTexts.push_back(krys::move(selectionMarkedText));
      }
    }
    auto styledMarkedTexts =
      StyledMarkedText::SubdivideAndResolve(markedTexts, _renderer, _isFirstLine, _paintInfo);

    // Coalesce styles of adjacent marked texts to minimize the number of drawing commands.
    auto coalescedStyledMarkedTexts =
      StyledMarkedText::CoalesceAdjacentWithEqualBackground(styledMarkedTexts);
    for (auto &markedText : coalescedStyledMarkedTexts)
    {
      PaintBackgroundFillForRange(markedText.StartOffset, markedText.EndOffset,
                                  markedText.MarkedStyle.BackgroundColor);
    }
  }

  // FIXME: Shaped content (isInsideShapedContent), once there's shaping across boxes.
  LayoutRect TextBoxPainter::SelectionRectForRange(uint32 startOffset, uint32 endOffset) const noexcept
  {
    // Note that if the text is truncated, we let the thing being painted in the truncation
    // draw its own highlight.
    auto lineBox = MakeIterator()->GetLineBox();
    auto selectionBottom = LineSelection::LogicalBottom(*lineBox);
    auto selectionTop = LineSelection::LogicalTopAdjustedForPrecedingBlock(*lineBox);
    // Use same y positioning and height as for selection, so that when the selection and this subrange are on
    // the same word there are no pieces sticking out.
    auto deltaY = LayoutUnit {GetWritingMode().IsLineInverted() ? selectionBottom - _logicalRect.MaxY()
                                                                : _logicalRect.Y() - selectionTop};
    auto selectionHeight = LayoutUnit {std::max(0.f, selectionBottom - selectionTop)};
    auto selectionRect = LayoutRect {LayoutUnit(_paintRect.X()), LayoutUnit(_paintRect.Y()) - deltaY,
                                     LayoutUnit(_logicalRect.Width()), selectionHeight};

    GetFontCascade().AdjustSelectionRectForText(_renderer.CanUseSimplifiedTextMeasuring().value_or(false),
                                                _paintTextRun, selectionRect, startOffset, endOffset);
    return selectionRect;
  }

  void TextBoxPainter::PaintBackgroundFillForRange(uint32 startOffset, uint32 endOffset,
                                                   const Color &color) noexcept
  {
    if (startOffset >= endOffset)
    {
      return;
    }

    GraphicsContext &context = _paintInfo.Context();
    GraphicsContextStateSaver stateSaver {context};
    UpdateGraphicsContext(context, TextPaintStyle {color}); // Don't draw text at all!

    auto selectionRect = SelectionRectForRange(startOffset, endOffset);

    if (_paintTextRun.Length() == endOffset - startOffset)
    {
      // FIXME: We should reconsider re-measuring the content when non-whitespace runs are joined together.
      auto unAdjustedSelectionRectMaxX = LayoutUnit {_paintRect.X() + _logicalRect.Width()};
      auto visualRight = std::max(selectionRect.MaxX(), unAdjustedSelectionRectMaxX);
      selectionRect.ShiftMaxXEdgeTo(visualRight);
    }

    // FIXME: Support painting combined text.
    auto backgroundRect = SnapRectToDevicePixels(selectionRect, _document.DeviceScaleFactor());
    context.FillRect(backgroundRect, color);
  }

  bool TextBoxPainter::ComputeHaveSelection() const noexcept
  {
    if (_isPrinting || _paintInfo.Phase == PaintPhase::TextClip)
    {
      return false;
    }

    return _renderer.View().Selection().HighlightStateForTextBox(_renderer, _selectableRange)
           != RenderObject::HighlightState::None;
  }

  void TextBoxPainter::PaintForegroundAndDecorations() noexcept
  {
    auto shouldPaintSelectionForeground = _haveSelection && !_compositionWithCustomUnderlines;
    auto hasTextDecoration = !_style.TextDecorationLineInEffect().IsNone();
    auto hasHighlightDecoration =
      _document.HasHighlight()
      && !MarkedText::CollectForHighlights(_renderer, _selectableRange, MarkedText::PaintPhase::Decoration)
            .empty();
    // FIXME: Decorations from spelling and grammar errors, once there are document markers.
    auto hasDecoration = hasTextDecoration || hasHighlightDecoration;

    auto contentMayNeedStyledMarkedText = [&]
    {
      if (hasDecoration)
      {
        return true;
      }
      if (shouldPaintSelectionForeground)
      {
        return true;
      }
      // FIXME: Document markers, once there are any.
      if (_document.HasHighlight())
      {
        return true;
      }
      return false;
    };
    auto startPosition = _selectableRange.Clamp(TextBox().Start());
    auto endPosition = _selectableRange.Clamp(TextBox().End());

    if (!contentMayNeedStyledMarkedText())
    {
      auto markedText = MarkedText {startPosition, endPosition, MarkedText::Type::Unmarked};
      auto styledMarkedText = StyledMarkedText {
        markedText,
        StyledMarkedText::ComputeStyleForUnmarkedMarkedText(_renderer, _style, _isFirstLine, _paintInfo)};
      PaintForeground(styledMarkedText);
      return;
    }

    List<MarkedText> markedTexts;
    if (_paintInfo.Phase != PaintPhase::Selection)
    {
      // The marked texts for the gaps between document markers and selection are implicitly created by
      // subdividing the entire line.
      markedTexts.push_back({startPosition, endPosition, MarkedText::Type::Unmarked});

      if (!_isPrinting)
      {
        // FIXME: Document markers and dragged and transparent content, once there are those.
        auto highlights =
          MarkedText::CollectForHighlights(_renderer, _selectableRange, MarkedText::PaintPhase::Foreground);
        markedTexts.insert(markedTexts.end(), highlights.begin(), highlights.end());
      }
    }
    // The selection marked text acts as a placeholder when computing the marked texts for the gaps...
    if (shouldPaintSelectionForeground)
    {
      krys_debug_assert(!_isPrinting);
      auto selectionMarkedText = CreateMarkedTextFromSelectionInBox();
      if (!selectionMarkedText.IsEmpty())
      {
        markedTexts.push_back(krys::move(selectionMarkedText));
      }
    }

    auto styledMarkedTexts =
      StyledMarkedText::SubdivideAndResolve(markedTexts, _renderer, _isFirstLine, _paintInfo);

    // ... now remove the selection marked text if we are excluding selection.
    if (!_isPrinting && _paintInfo.PaintBehavior.Contains(PaintBehavior::ExcludeSelection))
    {
      std::erase_if(styledMarkedTexts, [](const StyledMarkedText &markedText)
                    { return markedText.MarkedType == MarkedText::Type::Selection; });
    }

    if (hasDecoration && _paintInfo.Phase != PaintPhase::Selection)
    {
      uint32 length = _selectableRange.Truncation ? *_selectableRange.Truncation : _paintTextRun.Length();
      uint32 selectionStart = 0;
      uint32 selectionEnd = 0;
      if (_haveSelection)
      {
        std::tie(selectionStart, selectionEnd) = SelectionStartEnd();
      }

      FloatRect textDecorationSelectionClipOutRect;
      if ((_paintInfo.PaintBehavior.Contains(PaintBehavior::ExcludeSelection))
          && selectionStart < selectionEnd && selectionEnd <= length)
      {
        textDecorationSelectionClipOutRect = _paintRect;
        float logicalWidthBeforeRange;
        float logicalWidthAfterRange;
        float logicalSelectionWidth = GetFontCascade().WidthOfTextRange(
          _paintTextRun, selectionStart, selectionEnd, logicalWidthBeforeRange, logicalWidthAfterRange);
        // FIXME: Do we need to handle vertical bottom to top text?
        if (!TextBox().IsHorizontal())
        {
          textDecorationSelectionClipOutRect.Move(0, logicalWidthBeforeRange);
          textDecorationSelectionClipOutRect.SetHeight(logicalSelectionWidth);
        }
        else if (TextBox().Direction() == TextDirection::RTL)
        {
          textDecorationSelectionClipOutRect.Move(logicalWidthAfterRange, 0);
          textDecorationSelectionClipOutRect.SetWidth(logicalSelectionWidth);
        }
        else
        {
          textDecorationSelectionClipOutRect.Move(logicalWidthBeforeRange, 0);
          textDecorationSelectionClipOutRect.SetWidth(logicalSelectionWidth);
        }
      }

      // Coalesce styles of adjacent marked texts to minimize the number of drawing commands.
      auto coalescedStyledMarkedTexts =
        StyledMarkedText::CoalesceAdjacentWithEqualDecorations(styledMarkedTexts);

      for (auto &markedText : coalescedStyledMarkedTexts)
      {
        uint32 startOffset = markedText.StartOffset;
        uint32 endOffset = markedText.EndOffset;
        if (startOffset < endOffset)
        {
          // Avoid measuring the text when the entire line box is selected as an optimization.
          auto snappedPaintRect = SnapRectToDevicePixelsWithWritingDirection(
            LayoutRect {_paintRect}, _document.DeviceScaleFactor(), _paintTextRun.Ltr());
          if (startOffset || endOffset != _paintTextRun.Length())
          {
            LayoutRect selectionRect = {LayoutUnit(_paintRect.X()), LayoutUnit(_paintRect.Y()),
                                        LayoutUnit(_paintRect.Width()), LayoutUnit(_paintRect.Height())};
            GetFontCascade().AdjustSelectionRectForText(
              _renderer.CanUseSimplifiedTextMeasuring().value_or(false), _paintTextRun, selectionRect,
              startOffset, endOffset);
            snappedPaintRect = SnapRectToDevicePixelsWithWritingDirection(
              selectionRect, _document.DeviceScaleFactor(), _paintTextRun.Ltr());
          }
          auto decorationPainter = CreateDecorationPainter(markedText, textDecorationSelectionClipOutRect);
          PaintBackgroundDecorations(decorationPainter, markedText, snappedPaintRect);
          PaintForeground(markedText);
          PaintForegroundDecorations(decorationPainter, markedText, snappedPaintRect);
        }
      }
    }
    else
    {
      // Coalesce styles of adjacent marked texts to minimize the number of drawing commands.
      auto coalescedStyledMarkedTexts =
        StyledMarkedText::CoalesceAdjacentWithEqualForeground(styledMarkedTexts);

      if (coalescedStyledMarkedTexts.empty())
      {
        return;
      }

      for (auto &markedText : coalescedStyledMarkedTexts)
      {
        PaintForeground(markedText);
      }
    }
  }

  // FIXME: The redesigned text cursor's rounded underlines, which are Cocoa's.
  void TextBoxPainter::FillCompositionUnderline(float start, float width,
                                                const CompositionUnderline &underline) const noexcept
  {
    // Thick marked text underlines are 2px thick as long as there is room for the 2px line under the
    // baseline. All other marked text underlines are 1px thick. If there's not enough space the underline
    // will touch or overlap characters.
    int32 lineThickness = 1;
    auto baseline = Snap(_style.MetricsOfPrimaryFont().Ascent());
    if (underline.Thick && _logicalRect.Height() - baseline >= 2)
    {
      lineThickness = 2;
    }

    // We need to have some space between underlines of subsequent clauses, because some input methods do not
    // use different underline styles for those. We make each line shorter, which has a harmless side effect
    // of shortening the first and last clauses, too.
    start += 1;
    width -= 2;

    auto underlineColor =
      underline.UnderlineColor == CompositionUnderlineColor::TextColor
        ? _style.VisitedDependentColorWithColorFilter(css::PropertyId::WebkitTextFillColor)
        : _style.ColorByApplyingColorFilter(underline.Color);

    auto &context = _paintInfo.Context();
    context.SetStrokeColor(underlineColor);
    context.SetStrokeThickness(static_cast<float>(lineThickness));
    context.DrawLineForText(
      FloatRect(_paintRect.X() + start,
                _paintRect.Y() + _logicalRect.Height() - static_cast<float>(lineThickness), width,
                static_cast<float>(lineThickness)),
      _isPrinting);
  }

  void TextBoxPainter::PaintCompositionUnderlines() noexcept
  {
    const auto &underlines = _renderer.GetDocument().GetEditor().CustomCompositionUnderlines();
    for (const auto &underline : underlines)
    {
      if (underline.EndOffset <= TextBox().Start())
      {
        // Underline is completely before this run. This might be an underline that sits
        // before the first run we draw, or underlines that were within runs we skipped
        // due to truncation.
        continue;
      }

      if (underline.StartOffset >= TextBox().End())
      {
        break; // Underline is completely after this run, bail. A later run will paint it.
      }

      // Underline intersects this run. Paint it.
      PaintCompositionUnderline(underline);

      if (underline.EndOffset > TextBox().End())
      {
        break; // Underline also runs into the next run. Bail now, no more marker advancement.
      }
    }
  }

  namespace
  {
    void MirrorRTLSegment(float logicalWidth, TextDirection direction, float &start, float width) noexcept
    {
      if (direction == TextDirection::LTR)
      {
        return;
      }
      start = logicalWidth - width - start;
    }
  }

  float TextBoxPainter::TextPosition() const noexcept
  {
    // When computing the width of a text run, RenderBlock::computeInlineDirectionPositionsForLine() doesn't
    // include the actual offset from the containing block edge in its measurement. TextPosition() should be
    // consistent so the text are rendered in the same width.
    if (!_logicalRect.X())
    {
      return 0;
    }
    return _logicalRect.X() - MakeIterator()->GetLineBox()->ContentLogicalLeft();
  }

  void TextBoxPainter::PaintCompositionUnderline(const CompositionUnderline &underline) noexcept
  {
    float start = 0;                    // start of line to draw, relative to tx
    float width = _logicalRect.Width(); // how much line to draw
    bool useWholeWidth = true;
    uint32 paintStart = TextBox().Start();
    uint32 paintEnd = TextBox().End();
    if (paintStart <= underline.StartOffset)
    {
      paintStart = underline.StartOffset;
      useWholeWidth = false;
      start =
        _renderer.Width(TextBox().Start(), paintStart - TextBox().Start(), TextPosition(), _isFirstLine);
    }
    if (paintEnd != underline.EndOffset)
    {
      paintEnd = std::min(paintEnd, underline.EndOffset);
      useWholeWidth = false;
    }
    if (_selectableRange.Truncation)
    {
      paintEnd = std::min(paintEnd, TextBox().Start() + *_selectableRange.Truncation);
      useWholeWidth = false;
    }
    if (!useWholeWidth)
    {
      width = _renderer.Width(paintStart, paintEnd - paintStart, TextPosition() + start, _isFirstLine);
      MirrorRTLSegment(_logicalRect.Width(), TextBox().Direction(), start, width);
    }

    FillCompositionUnderline(start, width, underline);
  }

  void TextBoxPainter::PaintForeground(const StyledMarkedText &markedText) noexcept
  {
    if (markedText.StartOffset >= markedText.EndOffset)
    {
      return;
    }

    auto &context = _paintInfo.Context();
    const FontCascade &font = GetFontCascade();

    float emphasisMarkOffset = 0;
    auto emphasisExistsAndIsAbove = EmphasisMarkExistsAndIsAbove(_renderer, _style);
    StringAtom emphasisMark =
      emphasisExistsAndIsAbove ? StringAtom(_style.GetTextEmphasisStyle().MarkString()) : StringAtom::Null();
    if (!emphasisMark.View().empty())
    {
      emphasisMarkOffset =
        *emphasisExistsAndIsAbove
          ? -Snap(font.MetricsOfPrimaryFont().Ascent()) - font.EmphasisMarkDescent(emphasisMark)
          : Snap(font.MetricsOfPrimaryFont().Descent()) + font.EmphasisMarkAscent(emphasisMark);
    }

    TextPainter textPainter {context,
                             font,
                             _style,
                             markedText.MarkedStyle.TextStyles,
                             markedText.MarkedStyle.TextShadow,
                             emphasisMark,
                             emphasisMarkOffset};

    bool isTransparentMarkedText = markedText.MarkedType == MarkedText::Type::DraggedContent
                                   || markedText.MarkedType == MarkedText::Type::TransparentContent;
    GraphicsContextStateSaver stateSaver(context, markedText.MarkedStyle.TextStyles.StrokeWidth > 0
                                                    || isTransparentMarkedText);
    if (isTransparentMarkedText)
    {
      context.SetAlpha(markedText.MarkedStyle.Alpha);
    }
    UpdateGraphicsContext(context, markedText.MarkedStyle.TextStyles);

    if (IsInsideShapedContent() && PaintForegroundForShapeRange(textPainter))
    {
      return;
    }
    // TextPainter wants the box rectangle and text origin of the entire line box.
    textPainter.PaintRange(_paintTextRun, _paintRect, TextOriginFromPaintRect(_paintRect),
                           markedText.StartOffset, markedText.EndOffset);
  }

  bool TextBoxPainter::PaintForegroundForShapeRange(TextPainter &textPainter) noexcept
  {
    krys_debug_assert(_textBox.Direction() == TextDirection::RTL);

    auto &context = _paintInfo.Context();

    auto clipRect = [&]
    {
      // We could also just use ink overflow here but since non-range painting
      // sets up no clipping, we should not do that either here.
      auto rect = FloatRect::InfiniteRect();
      rect.SetX(_paintRect.X());
      // Note that this is RTL direction.
      auto &textContent = _textBox.GetBox().GetText();
      if (!textContent.IsAtShapingBoundaryStart())
      {
        rect.SetWidth(_paintRect.Width());
      }
      // FIXME: Setup a semi-inifite rect for the (visually) first box where x is -inifite with fixed maxX.
      return rect;
    };
    context.Save();
    context.Clip(clipRect());
    auto shapedContent = ShapedContent {};
    BuildTextForShaping(shapedContent, _textBox);

    if (shapedContent.Text.IsEmpty())
    {
      context.Restore();
      return false;
    }

    auto paintRect = _paintRect;
    paintRect.ShiftXEdgeTo(_paintOffset.X() + shapedContent.VisualLeft);

    auto characterScanForCodePath = true;
    auto expansion = _textBox.GetBox().GetExpansion();
    auto text = shapedContent.Text.ToString();
    auto run = TextRun {text,
                        paintRect.X(),
                        expansion.HorizontalExpansion,
                        expansion.Behavior,
                        _textBox.Direction(),
                        _style.RtlOrdering() == Order::Visual,
                        characterScanForCodePath};
    run.DisableSpacing();

    textPainter.PaintRange(run, paintRect, TextOriginFromPaintRect(paintRect), 0,
                           static_cast<uint32>(text.size()));
    context.Restore();
    return true;
  }

  TextDecorationPainter TextBoxPainter::CreateDecorationPainter(const StyledMarkedText &markedText,
                                                                const FloatRect &clipOutRect) noexcept
  {
    auto &context = _paintInfo.Context();

    UpdateGraphicsContext(context, markedText.MarkedStyle.TextStyles);

    // Note that if the text is truncated, we let the thing being painted in the truncation
    // draw its own decoration.
    GraphicsContextStateSaver stateSaver {context, false};
    bool isTransparentContent = markedText.MarkedType == MarkedText::Type::DraggedContent
                                || markedText.MarkedType == MarkedText::Type::TransparentContent;
    if (isTransparentContent || !clipOutRect.IsEmpty())
    {
      stateSaver.Save();
      if (isTransparentContent)
      {
        context.SetAlpha(markedText.MarkedStyle.Alpha);
      }
      if (!clipOutRect.IsEmpty())
      {
        context.ClipOut(clipOutRect);
      }
    }

    // Create painter
    return {context, GetFontCascade(), markedText.MarkedStyle.TextShadow, _document.Printing(),
            GetWritingMode()};
  }

  KRYS_NODISCARD static inline float ComputedTextDecorationThickness(const RenderStyle &styleToUse,
                                                                     float deviceScaleFactor) noexcept
  {
    return CeilToDevicePixel(styleToUse.GetTextDecorationThickness().Resolve(styleToUse), deviceScaleFactor);
  }

  KRYS_NODISCARD static inline float ComputedAutoTextDecorationThickness(const RenderStyle &styleToUse,
                                                                         float deviceScaleFactor) noexcept
  {
    return CeilToDevicePixel(style::TextDecorationThickness {css::keywords::Auto {}}.Resolve(styleToUse),
                             deviceScaleFactor);
  }

  KRYS_NODISCARD static inline float ComputedLinethroughCenter(const RenderStyle &styleToUse,
                                                               float textDecorationThickness,
                                                               float autoTextDecorationThickness) noexcept
  {
    auto center = 2 * styleToUse.MetricsOfPrimaryFont().Ascent() / 3 + autoTextDecorationThickness / 2;
    return center - textDecorationThickness / 2;
  }

  KRYS_NODISCARD static inline style::TextDecorationLine
    ComputedTextDecorationType(const RenderStyle &styleToUse,
                               const TextDecorationPainter::Styles &textDecorationStyles) noexcept
  {
    auto textDecorations = styleToUse.TextDecorationLineInEffect();
    (void)textDecorations.AddOrReplaceIfNotNone(
      TextDecorationPainter::TextDecorationsInEffectForStyle(textDecorationStyles));
    return textDecorations;
  }

  KRYS_NODISCARD static inline const RenderStyle &
    DecoratingBoxStyleForInlineBox(const inline_iterator::InlineBox &inlineBox, bool isFirstLine) noexcept
  {
    if (!inlineBox.IsRootInlineBox())
    {
      return inlineBox.Style();
    }
    // "When specified on or propagated to a block container that establishes an inline formatting context,
    // the decorations are propagated to an anonymous inline box that wraps all the in-flow inline-level
    // children of the block container" https://drafts.csswg.org/css-text-decor-4/#line-decoration Sadly we
    // don't have the concept of anonymous inline box for all inline-level chidren when content forces us to
    // generate anonymous block containers.
    for (const RenderElement *ancestor = &inlineBox.Renderer(); ancestor; ancestor = ancestor->Parent())
    {
      if (!ancestor->IsAnonymous())
      {
        return isFirstLine ? ancestor->FirstLineStyle() : ancestor->Style();
      }
    }
    krys_debug_assert(false);
    return inlineBox.Style();
  }

  KRYS_NODISCARD static inline bool IsDecoratingBoxForBackground(const inline_iterator::InlineBox &inlineBox,
                                                                 const RenderStyle &styleToUse) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;
    auto *element = DynamicDowncast<html::HTMLElement>(inlineBox.Renderer().GetElement());
    if (element
        && (Is<html::HTMLAnchorElement>(*element)
            || element->TagQualifiedName().TagName() == html::TagName::font))
    {
      // <font> and <a> are always considered decorating boxes.
      return true;
    }
    return styleToUse.GetTextDecorationLine().ContainsAny({Flag::Underline, Flag::Overline})
           || (inlineBox.IsRootInlineBox()
               && styleToUse.TextDecorationLineInEffect().ContainsAny({Flag::Underline, Flag::Overline}));
  }

  void TextBoxPainter::CollectDecoratingBoxesForBackgroundPainting(
    DecoratingBoxList &decoratingBoxList, const inline_iterator::TextBoxIterator &textBox,
    FloatPoint textBoxLocation, const TextDecorationPainter::Styles &overrideDecorationStyle) noexcept
  {
    auto ancestorInlineBox = textBox->ParentInlineBox();
    if (!ancestorInlineBox)
    {
      krys_debug_assert(false);
      return;
    }

    if (ancestorInlineBox->IsRootInlineBox())
    {
      decoratingBoxList.push_back({ancestorInlineBox,
                                   DecoratingBoxStyleForInlineBox(*ancestorInlineBox, _isFirstLine),
                                   overrideDecorationStyle, textBoxLocation});
      return;
    }

    if (GetWritingMode().IsLineInverted())
    {
      // FIXME: underlineOffsetForTextBoxPainting returns incorrect value for vertical-lr.
      decoratingBoxList.push_back({ancestorInlineBox, _style, overrideDecorationStyle, textBoxLocation});
      return;
    }

    enum UseOverriderDecorationStyle : bool
    {
      No,
      Yes
    };
    auto appendIfIsDecoratingBoxForBackground = [&](const inline_iterator::InlineBoxIterator &inlineBox,
                                                    UseOverriderDecorationStyle useOverriderDecorationStyle)
    {
      auto &boxStyle = DecoratingBoxStyleForInlineBox(*inlineBox, _isFirstLine);

      auto computedDecorationStyle = [&]
      {
        return TextDecorationPainter::StylesForRenderer(inlineBox->Renderer(),
                                                        boxStyle.TextDecorationLineInEffect(), _isFirstLine);
      };
      if (!IsDecoratingBoxForBackground(*inlineBox, boxStyle))
      {
        // Some cases even non-decoration boxes may have some decoration pieces coming from the marked text
        // (e.g. highlight).
        if (useOverriderDecorationStyle == UseOverriderDecorationStyle::No
            || overrideDecorationStyle == computedDecorationStyle())
        {
          return;
        }
      }

      auto decoratingBoxLocation = textBoxLocation;
      auto parentInlineBox = textBox->ParentInlineBox();
      // Normally text box top is aligned with the parent inline box top (i.e. textBox->logicalTop() ==
      // parentInlineBox->logicalTop()) but when inline box sides are trimmed (see: text-box property) inline
      // box gets an offset while text box does not.
      if (&inlineBox->Renderer() != &parentInlineBox->Renderer())
      {
        auto decoratingBoxContentBoxTop =
          inlineBox->LogicalTop()
          + (!inlineBox->IsRootInlineBox()
               ? static_cast<float>(inlineBox->Renderer().BorderAndPaddingBefore())
               : 0.f);
        auto parentInlineBoxContentBoxTop =
          parentInlineBox->LogicalTop()
          + (!parentInlineBox->IsRootInlineBox()
               ? static_cast<float>(parentInlineBox->Renderer().BorderAndPaddingBefore())
               : 0.f);
        decoratingBoxLocation.MoveBy(
          FloatPoint {0.f, decoratingBoxContentBoxTop - parentInlineBoxContentBoxTop
                             + TextBoxEdgeAdjustmentForUnderline(parentInlineBox->Style())});
      }
      else
      {
        decoratingBoxLocation.MoveBy(FloatPoint {0.f, TextBoxEdgeAdjustmentForUnderline(inlineBox->Style())});
      }
      auto decorationStyleToUse = useOverriderDecorationStyle == UseOverriderDecorationStyle::Yes
                                    ? overrideDecorationStyle
                                    : computedDecorationStyle();
      decoratingBoxList.push_back({inlineBox, boxStyle, decorationStyleToUse, decoratingBoxLocation});
    };

    // FIXME: Figure out if the decoration styles coming from the styled marked text should be used only on
    // the closest inline box (direct parent).
    appendIfIsDecoratingBoxForBackground(ancestorInlineBox, UseOverriderDecorationStyle::Yes);
    while (!ancestorInlineBox->IsRootInlineBox())
    {
      ancestorInlineBox = ancestorInlineBox->ParentInlineBox();
      if (!ancestorInlineBox)
      {
        krys_debug_assert(false);
        break;
      }
      appendIfIsDecoratingBoxForBackground(ancestorInlineBox, UseOverriderDecorationStyle::No);
    }
  }

  void TextBoxPainter::PaintBackgroundDecorations(TextDecorationPainter &decorationPainter,
                                                  const StyledMarkedText &markedText,
                                                  const FloatRect &textBoxPaintRect) noexcept
  {
    auto textRun =
      _paintTextRun.SubRun(markedText.StartOffset, markedText.EndOffset - markedText.StartOffset);

    auto textBox = MakeIterator();
    auto decoratingBoxList = DecoratingBoxList {};
    CollectDecoratingBoxesForBackgroundPainting(decoratingBoxList, textBox, textBoxPaintRect.Location(),
                                                markedText.MarkedStyle.TextDecorationStyles);

    for (auto &decoratingBox : decoratingBoxList | std::views::reverse)
    {
      auto computedTextDecorationType =
        ComputedTextDecorationType(decoratingBox.Style, decoratingBox.TextDecorationStyles);
      auto computedBackgroundDecorationGeometry = [&]
      {
        auto textDecorationThickness =
          ComputedTextDecorationThickness(decoratingBox.Style, _document.DeviceScaleFactor());
        auto underlineOffset = [&]
        {
          if (!computedTextDecorationType.HasUnderline())
          {
            return 0.f;
          }
          auto baseOffset = UnderlineOffsetForTextBoxPainting(*decoratingBox.InlineBox, decoratingBox.Style);
          auto wavyOffset =
            decoratingBox.TextDecorationStyles.Underline.DecorationStyle == TextDecorationStyle::Wavy
              ? WavyOffsetFromDecoration()
              : 0.f;
          return baseOffset + wavyOffset;
        };
        auto autoTextDecorationThickness =
          ComputedAutoTextDecorationThickness(decoratingBox.Style, _document.DeviceScaleFactor());
        auto overlineOffset = [&]
        {
          if (!computedTextDecorationType.HasOverline())
          {
            return 0.f;
          }
          auto baseOffset = OverlineOffsetForTextBoxPainting(*decoratingBox.InlineBox, decoratingBox.Style);
          baseOffset += (autoTextDecorationThickness - textDecorationThickness);
          auto wavyOffset =
            decoratingBox.TextDecorationStyles.Overline.DecorationStyle == TextDecorationStyle::Wavy
              ? WavyOffsetFromDecoration()
              : 0.f;
          return baseOffset - wavyOffset;
        };

        return TextDecorationPainter::BackgroundDecorationGeometry {
          TextOriginFromPaintRect(textBoxPaintRect),
          RoundPointToDevicePixels(LayoutPoint {decoratingBox.Location}, _document.DeviceScaleFactor(),
                                   _paintTextRun.Ltr()),
          textBoxPaintRect.Width(),
          textDecorationThickness,
          underlineOffset(),
          overlineOffset(),
          ComputedLinethroughCenter(decoratingBox.Style, textDecorationThickness,
                                    autoTextDecorationThickness),
          Snap(decoratingBox.Style.MetricsOfPrimaryFont().Ascent()) + 2.f,
          GetWavyStrokeParameters(decoratingBox.Style.ComputedFontSize())};
      };

      decorationPainter.PaintBackgroundDecorations(_style, textRun, computedBackgroundDecorationGeometry(),
                                                   computedTextDecorationType,
                                                   decoratingBox.TextDecorationStyles);
    }
  }

  KRYS_NODISCARD static const RenderStyle &
    DecoratingBoxStyle(const inline_iterator::TextBoxIterator &textBox) noexcept
  {
    if (auto parentInlineBox = textBox->ParentInlineBox())
    {
      return parentInlineBox->Style();
    }
    krys_debug_assert(false);
    return textBox->Style();
  }

  void TextBoxPainter::PaintForegroundDecorations(TextDecorationPainter &decorationPainter,
                                                  const StyledMarkedText &markedText,
                                                  const FloatRect &textBoxPaintRect) noexcept
  {
    auto textBox = MakeIterator();
    auto &styleForDecoration = DecoratingBoxStyle(textBox);
    auto computedTextDecorationType = [&]
    {
      auto textDecorations = styleForDecoration.TextDecorationLineInEffect();
      (void)textDecorations.AddOrReplaceIfNotNone(
        TextDecorationPainter::TextDecorationsInEffectForStyle(markedText.MarkedStyle.TextDecorationStyles));
      return textDecorations;
    }();

    if (!computedTextDecorationType.HasLineThrough())
    {
      return;
    }

    auto deviceScaleFactor = _document.DeviceScaleFactor();
    auto textDecorationThickness = ComputedTextDecorationThickness(styleForDecoration, deviceScaleFactor);
    auto linethroughCenter =
      ComputedLinethroughCenter(styleForDecoration, textDecorationThickness,
                                ComputedAutoTextDecorationThickness(styleForDecoration, deviceScaleFactor));
    decorationPainter.PaintForegroundDecorations(
      {textBoxPaintRect.Location(), textBoxPaintRect.Width(), textDecorationThickness, linethroughCenter,
       GetWavyStrokeParameters(styleForDecoration.ComputedFontSize())},
      markedText.MarkedStyle.TextDecorationStyles);
  }

  FloatRect TextBoxPainter::ComputePaintRect(const LayoutPoint &paintOffset) const noexcept
  {
    FloatPoint localPaintOffset(paintOffset);
    if (GetWritingMode().IsVertical())
    {
      localPaintOffset.Move(0, -_logicalRect.Height());
      if (GetWritingMode().IsLineOverLeft())
      {
        localPaintOffset.Move(_logicalRect.Height(), _logicalRect.Width());
      }
    }

    auto visualRect = TextBox().VisualRectIgnoringBlockDirection();
    TextBox().FormattingContextRoot().FlipForWritingMode(visualRect);

    auto boxOrigin = visualRect.Location();

    boxOrigin.MoveBy(localPaintOffset);
    if (GetWritingMode().IsVertical())
    {
      // This is required by the CTM rotation we do for vertical content.
      boxOrigin.SetX(RoundToDevicePixel(LayoutUnit {boxOrigin.X()}, _document.DeviceScaleFactor()));
    }
    return {boxOrigin, FloatSize(_logicalRect.Width(), _logicalRect.Height())};
  }

  const FontCascade &TextBoxPainter::GetFontCascade() const noexcept
  {
    return _style.GetFontCascade();
  }

  WritingMode TextBoxPainter::GetWritingMode() const noexcept
  {
    return _style.GetWritingMode();
  }

  FloatPoint TextBoxPainter::TextOriginFromPaintRect(const FloatRect &paintRect) const noexcept
  {
    auto ascent = Snap(_style.MetricsOfPrimaryFont().Ascent());
    if (GetWritingMode().IsVertical())
    {
      // FIXME: This is required by the CTM rotation logic. We should eventually (re)move it though.
      ascent = RoundToDevicePixel(LayoutUnit {ascent}, _document.DeviceScaleFactor());
    }

    auto textOrigin = FloatPoint {paintRect.X(), paintRect.Y() + ascent};

    if (GetWritingMode().IsHorizontal())
    {
      textOrigin.SetY(RoundToDevicePixel(LayoutUnit {textOrigin.Y()}, _document.DeviceScaleFactor()));
    }
    else
    {
      textOrigin.SetX(RoundToDevicePixel(LayoutUnit {textOrigin.X()}, _document.DeviceScaleFactor()));
    }

    return textOrigin;
  }

  bool TextBoxPainter::IsInsideShapedContent() const noexcept
  {
    auto &textContent = TextBox().GetBox().GetText();
    return textContent.IsAtShapingBoundaryStart() || textContent.IsAtShapingBoundaryEnd()
           || textContent.IsInsideShapingBoundary();
  }

  FloatSize TextBoxPainter::RotateShadowOffset(
    const css::SpaceSeparatedPoint<style::Length<css::AllUnzoomed>> &offset, WritingMode writingMode,
    const style::ZoomFactor &zoomFactor) noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return {
        offset.x().ResolveZoom(zoomFactor),
        offset.y().ResolveZoom(zoomFactor),
      };
    }

    if (writingMode.IsLineOverLeft())
    { // sideways-lr
      return {
        -offset.y().ResolveZoom(zoomFactor),
        offset.x().ResolveZoom(zoomFactor),
      };
    }

    return {
      offset.y().ResolveZoom(zoomFactor),
      -offset.x().ResolveZoom(zoomFactor),
    };
  }
}
