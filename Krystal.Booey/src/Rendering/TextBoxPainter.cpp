#include "Krystal.Booey/Rendering/TextBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/StyledMarkedText.hpp"
#include "Krystal.Booey/Rendering/TextPainter.hpp"
#include "Krystal.Booey/Rendering/TextPaintStyle.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>

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
        _isPrinting(_document.Printing())
  {
    krys_debug_assert(paintInfo.Phase == PaintPhase::Foreground || paintInfo.Phase == PaintPhase::Selection
                      || paintInfo.Phase == PaintPhase::TextClip || paintInfo.Phase == PaintPhase::EventRegion
                      || paintInfo.Phase == PaintPhase::Accessibility);
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

    // FIXME: Selection (computeHaveSelection) comes with selection; until then there's nothing to paint in
    // the selection phase.
    if (_paintInfo.Phase == PaintPhase::Selection)
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
      // FIXME: The background fill of selections, document markers, highlights and IME compositions, and
      // platform document markers (spelling and grammar errors) come with editing and selection.

      PaintForegroundAndDecorations();

      // FIXME: Composition underlines, the page's relevant repainted objects and the document's painted
      // text tracking (didPaintText) come with editing and paint milestones.
    }

    if (glyphRotation)
    {
      auto backRotation = *glyphRotation == RotationDirection::Clockwise ? RotationDirection::Counterclockwise
                                                                         : RotationDirection::Clockwise;
      _paintInfo.Context().ConcatCTM(Rotation(_paintRect, backRotation));
    }
  }

  void TextBoxPainter::PaintForegroundAndDecorations() noexcept
  {
    auto hasTextDecoration = !_style.TextDecorationLineInEffect().IsNone();
    // FIXME: Decorations from highlights and spelling and grammar errors come with marked text.
    auto hasDecoration = hasTextDecoration;

    auto contentMayNeedStyledMarkedText = [&]
    {
      if (hasDecoration)
      {
        return true;
      }
      // FIXME: Selections, document markers and highlights come with editing and selection.
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

      // FIXME: Document markers, highlights and dragged and transparent content come with editing.
    }
    // FIXME: The selection's marked text comes with selection.

    auto styledMarkedTexts =
      StyledMarkedText::SubdivideAndResolve(markedTexts, _renderer, _isFirstLine, _paintInfo);

    if (hasDecoration && _paintInfo.Phase != PaintPhase::Selection)
    {
      // FIXME: Clipping decorations out of an excluded selection comes with selection.

      // Coalesce styles of adjacent marked texts to minimize the number of drawing commands.
      auto coalescedStyledMarkedTexts =
        StyledMarkedText::CoalesceAdjacentWithEqualDecorations(styledMarkedTexts);

      for (auto &markedText : coalescedStyledMarkedTexts)
      {
        uint32 startOffset = markedText.StartOffset;
        uint32 endOffset = markedText.EndOffset;
        if (startOffset < endOffset)
        {
          // FIXME: The decorations under and over the text (paintBackgroundDecorations) and through it
          // (paintForegroundDecorations), painted on either side of the foreground, are still to come.
          PaintForeground(markedText);
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
