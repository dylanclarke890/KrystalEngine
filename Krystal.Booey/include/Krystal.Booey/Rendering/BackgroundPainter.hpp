#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundLayers.hpp"

namespace krys::boo
{
  class FloatRoundedRect;
  class GraphicsContext;
  class RenderView;
  struct PaintInfo;

  enum BaseBackgroundColorUsage
  {
    BaseBackgroundColorUse,
    BaseBackgroundColorOnly,
    BaseBackgroundColorSkip
  };

  /// @brief Where a background image is drawn: the rect it covers, its tiles' size, the phase of the first
  /// tile and the space between tiles.
  struct BackgroundImageGeometry
  {
    LayoutRect DestinationRect;
    LayoutPoint DestinationOrigin;
    LayoutSize TileSizeWithoutPixelSnapping;
    LayoutSize TileSize;
    LayoutSize Phase;
    LayoutSize SpaceSize;
    bool HasNonLocalGeometry; // Has background-attachment: fixed. Implies that we can't always cheaply
                              // compute destRect.

    BackgroundImageGeometry(const LayoutRect &destinationRect, const LayoutSize &tileSizeWithoutPixelSnapping,
                            const LayoutSize &tileSize, const LayoutSize &phase, const LayoutSize &spaceSize,
                            bool fixedAttachment) noexcept;

    KRYS_NODISCARD LayoutSize RelativePhase() const noexcept
    {
      LayoutSize relativePhase = Phase;
      relativePhase += DestinationRect.Location() - DestinationOrigin;
      return relativePhase;
    }

    void Clip(const LayoutRect &clipRect) noexcept
    {
      DestinationRect.Intersect(clipRect);
    }
  };

  template <typename TLayer>
  struct FillLayerToPaint
  {
    const TLayer &Layer;
    bool IsLast;
  };

  /// @brief Paints a box's backgrounds: its colour and images, layer by layer, clipped and positioned as
  /// their properties say, and the root's background over the canvas.
  /// FIXME: Mask layers (`mask-image`) come with images; box shadows (`paintBoxShadow`) come with borders;
  /// `background-clip: text` and `border-area` need text painting and borders.
  class BackgroundPainter
  {
  private:
    RenderBoxModelObject &_renderer;
    const PaintInfo &_paintInfo;
    Maybe<FillBox> _overrideClip;
    Maybe<FillBox> _overrideOrigin;

  public:
    BackgroundPainter(RenderBoxModelObject &, const PaintInfo &) noexcept;

    void SetOverrideClip(FillBox overrideClip) noexcept
    {
      _overrideClip = overrideClip;
    }
    void SetOverrideOrigin(FillBox overrideOrigin) noexcept
    {
      _overrideOrigin = overrideOrigin;
    }

    void PaintBackground(const LayoutRect &, BleedAvoidance) const noexcept;

    void PaintFillLayers(const Color &, const style::BackgroundLayers &, const LayoutRect &, BleedAvoidance,
                         CompositeOperator, RenderElement *backgroundObject = nullptr) const noexcept;
    void PaintFillLayer(const Color &, const FillLayerToPaint<style::BackgroundLayer> &, const LayoutRect &,
                        BleedAvoidance, const inline_iterator::InlineBoxIterator &,
                        const LayoutRect &backgroundImageStrip = {},
                        CompositeOperator = CompositeOperator::SourceOver,
                        RenderElement *backgroundObject = nullptr,
                        BaseBackgroundColorUsage = BaseBackgroundColorUse) const noexcept;

    KRYS_NODISCARD static bool PaintsOwnBackground(const RenderBoxModelObject &) noexcept;

    KRYS_NODISCARD static BackgroundImageGeometry CalculateFillLayerImageGeometry(
      const RenderBoxModelObject &, const RenderLayerModelObject *paintContainer,
      const style::BackgroundLayer &, const LayoutPoint &paintOffset, const LayoutRect &borderBoxRect,
      Maybe<FillBox> overrideOrigin = std::nullopt) noexcept;

    static void ClipRoundedInnerRect(GraphicsContext &, const FloatRoundedRect &clipRect) noexcept;

    KRYS_NODISCARD static bool
      BoxShadowShouldBeAppliedToBackground(const RenderBoxModelObject &, const LayoutPoint &paintOffset,
                                           BleedAvoidance,
                                           const inline_iterator::InlineBoxIterator &) noexcept;

  private:
    void PaintRootBoxFillLayers() const noexcept;

    template <typename TLayers>
    void PaintFillLayersImpl(const Color &, const TLayers &, const LayoutRect &, BleedAvoidance,
                             CompositeOperator, RenderElement *backgroundObject = nullptr) const noexcept;
    template <typename TLayer>
    void PaintFillLayerImpl(const Color &, const FillLayerToPaint<TLayer> &, const LayoutRect &,
                            BleedAvoidance, const inline_iterator::InlineBoxIterator &,
                            const LayoutRect &backgroundImageStrip = {},
                            CompositeOperator = CompositeOperator::SourceOver,
                            RenderElement *backgroundObject = nullptr,
                            BaseBackgroundColorUsage = BaseBackgroundColorUse) const noexcept;

    template <typename TLayer>
    KRYS_NODISCARD static BackgroundImageGeometry
      CalculateFillLayerImageGeometryImpl(const RenderBoxModelObject &,
                                          const RenderLayerModelObject *paintContainer, const TLayer &,
                                          const LayoutPoint &paintOffset, const LayoutRect &borderBoxRect,
                                          Maybe<FillBox> overrideOrigin = std::nullopt) noexcept;
    template <typename TLayer>
    KRYS_NODISCARD static LayoutSize CalculateFillTileSize(const RenderBoxModelObject &, const TLayer &,
                                                           const LayoutSize &positioningAreaSize) noexcept;

    KRYS_NODISCARD const dom::Document &Document() const noexcept;
    KRYS_NODISCARD const RenderView &View() const noexcept;
  };

  /// @brief @p rect inset by a device pixel on each side.
  KRYS_NODISCARD LayoutRect ShrinkRectByOneDevicePixel(const GraphicsContext &, const LayoutRect &,
                                                       float devicePixelRatio) noexcept;
}
