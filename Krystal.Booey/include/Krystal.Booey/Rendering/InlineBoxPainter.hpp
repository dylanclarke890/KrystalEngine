#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"

namespace krys::boo
{
  class Color;
  class RenderBoxModelObject;
  class RenderStyle;
  struct PaintInfo;
  template <typename TLayer>
  struct FillLayerToPaint;

  namespace inline_display
  {
    struct Box;
  }

  namespace layout_integration
  {
    class InlineContent;
  }

  /// @brief Paints an inline box's decorations: its box shadows, backgrounds and borders, on each line it's
  /// split across.
  /// FIXME: Masks (`paintMask`: `mask-image` and `mask-border`) come with masks; accessibility bounds come
  /// with accessibility.
  class InlineBoxPainter
  {
    KRYS_NON_COPY_MOVABLE(InlineBoxPainter);

  private:
    const inline_iterator::InlineBox _inlineBox;
    PaintInfo &_paintInfo;
    const LayoutPoint _paintOffset;
    const RenderBoxModelObject &_renderer;
    const bool _isFirstLineBox;
    const bool _isRootInlineBox;
    const bool _isHorizontal;

  public:
    InlineBoxPainter(const layout_integration::InlineContent &inlineContent, const inline_display::Box &box,
                     PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;
    ~InlineBoxPainter() noexcept;

    void Paint() noexcept;

  private:
    InlineBoxPainter(const inline_iterator::InlineBox &inlineBox, PaintInfo &paintInfo,
                     const LayoutPoint &paintOffset) noexcept;

    void PaintDecorations() noexcept;
    template <typename TLayers>
    void PaintFillLayers(const Color &color, const TLayers &fillLayers, const LayoutRect &paintRect,
                         CompositeOperator op) noexcept;
    template <typename TLayer>
    void PaintFillLayer(const Color &color, const FillLayerToPaint<TLayer> &fillLayer,
                        const LayoutRect &paintRect, CompositeOperator op) noexcept;
    void PaintBoxShadow(style::ShadowStyle shadowStyle, const LayoutRect &paintRect) noexcept;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    // FIXME: Make RenderBoxModelObject functions const.
    KRYS_NODISCARD RenderBoxModelObject &Renderer() const noexcept
    {
      return const_cast<RenderBoxModelObject &>(_renderer);
    }
    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _isHorizontal;
    }
  };
}
