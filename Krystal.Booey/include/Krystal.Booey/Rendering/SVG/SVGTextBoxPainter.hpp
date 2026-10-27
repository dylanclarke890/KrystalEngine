#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/TextBoxSelectableRange.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include <utility>

namespace krys::boo
{
  class GraphicsContext;
  class Path;
  class RenderBoxModelObject;
  class RenderSVGInlineText;
  class RenderStyle;
  struct PaintInfo;
  struct SVGTextFragment;

  /// @brief Paints an SVG text box's text fragments: decorations, fill and stroke (with the SVG paint
  /// servers), and selection.
  template <typename TTextBoxPath>
  class SVGTextBoxPainter
  {
  private:
    const TTextBoxPath _textBox;
    const RenderSVGInlineText &_renderer;
    PaintInfo &_paintInfo;
    const TextBoxSelectableRange _selectableRange;
    const LayoutPoint _paintOffset;
    const bool _haveSelection;

    EnumSet<RenderSVGResourceMode> _paintingResourceMode;
    LegacyRenderSVGResource *_legacyPaintingResource {nullptr};

  public:
    SVGTextBoxPainter(TTextBoxPath &&, PaintInfo &, const LayoutPoint &paintOffset) noexcept;
    ~SVGTextBoxPainter() noexcept = default;

    void Paint() noexcept;
    void PaintSelectionBackground() noexcept;

  private:
    KRYS_NODISCARD inline_iterator::SVGTextBoxIterator TextBoxIterator() const noexcept;

    KRYS_NODISCARD const RenderSVGInlineText &Renderer() const noexcept
    {
      return _renderer;
    }
    KRYS_NODISCARD const RenderBoxModelObject &ParentRenderer() const noexcept;
    KRYS_NODISCARD EnumSet<RenderSVGResourceMode> PaintingResourceMode() const noexcept
    {
      return _paintingResourceMode;
    }

    void PaintDecoration(style::TextDecorationLine, const SVGTextFragment &) noexcept;
    void PaintDecorationWithStyle(style::TextDecorationLine, const SVGTextFragment &,
                                  const RenderBoxModelObject &) noexcept;
    void PaintTextWithShadows(const RenderStyle &, TextRun &, const SVGTextFragment &, uint32 startPosition,
                              uint32 endPosition) noexcept;
    void PaintText(const RenderStyle &, const RenderStyle &selectionStyle, const SVGTextFragment &,
                   bool hasSelection, bool paintSelectedTextOnly) noexcept;

    bool AcquireLegacyPaintingResource(GraphicsContext *&, float scalingFactor, RenderBoxModelObject &,
                                       const RenderStyle &) noexcept;
    void ReleaseLegacyPaintingResource(GraphicsContext *&, const Path *) noexcept;

    KRYS_NODISCARD std::pair<uint32, uint32> SelectionStartEnd() const noexcept;
  };

  extern template class SVGTextBoxPainter<inline_iterator::BoxLegacyPath>;
  extern template class SVGTextBoxPainter<inline_iterator::BoxModernPath>;

  /// @brief Paints a legacy SVG text box.
  class LegacySVGTextBoxPainter : public SVGTextBoxPainter<inline_iterator::BoxLegacyPath>
  {
  public:
    LegacySVGTextBoxPainter(const SVGInlineTextBox &, PaintInfo &, const LayoutPoint &paintOffset) noexcept;
  };

  /// @brief Paints an SVG text box of the inline content.
  class ModernSVGTextBoxPainter : public SVGTextBoxPainter<inline_iterator::BoxModernPath>
  {
  public:
    ModernSVGTextBoxPainter(const layout_integration::InlineContent &, size_t boxIndex, PaintInfo &,
                            const LayoutPoint &paintOffset) noexcept;
  };

  KRYS_NODISCARD TextRun ConstructTextRun(utf8_stringview, TextDirection, const RenderStyle &,
                                          const SVGTextFragment &) noexcept;
  KRYS_NODISCARD FloatRect SelectionRectForTextFragment(const RenderSVGInlineText &, TextDirection,
                                                        const SVGTextFragment &, uint32 startPosition,
                                                        uint32 endPosition, const RenderStyle &) noexcept;
  bool MapStartEndPositionsIntoFragmentCoordinates(uint32 textBoxStart, const SVGTextFragment &,
                                                   uint32 &startPosition, uint32 &endPosition) noexcept;
}
