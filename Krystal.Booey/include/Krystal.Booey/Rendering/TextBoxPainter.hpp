#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/MarkedText.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

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
  /// FIXME: Document markers (their background fills and marked text, and platform document markers), the
  /// composition's custom highlights, event regions, accessibility bounds and contentful paint tracking come
  /// with document markers, event regions and accessibility.
  /// FIXME: Combined text (`text-combine-upright`, `RenderCombineText`) is still to come.
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
    // FIXME: Rounded backgrounds (BackgroundStyle::Rounded), with IME composition highlights.
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
