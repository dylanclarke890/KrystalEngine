#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderStyle;
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

  /// @brief Paints a text box: its text, with shadows, emphasis marks and decorations.
  /// FIXME: Selection, document markers, highlights and IME composition (the background fill, marked text
  /// other than unmarked, composition underlines and platform document markers), event regions,
  /// accessibility bounds and contentful paint tracking come with editing, selection, event regions and
  /// accessibility.
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

    void PaintForegroundAndDecorations() noexcept;
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
