#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/BorderEdge.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/BorderData.hpp"

namespace krys::boo
{
  class BorderShape;
  class Path;
  struct PaintInfo;

  namespace style
  {
    struct BorderImage;
  }

  /// @brief Paints a box's border: its image, or each side in its style, joined at the corners and following
  /// the border radii.
  /// FIXME: `mask-border` (the mask border overload of `paintNinePieceImage`) comes with masks.
  class BorderPainter
  {
    friend class OutlinePainter;

  private:
    struct Sides
    {
      Maybe<BorderData::RadiiType> Radii {}; // FIXME: Do we need this separately from the shape?
      const BorderEdges &Edges;
      bool HaveAllSolidEdges {true};
      bool OuterEdgeIsRectangular {true};
      bool InnerEdgeIsRectangular {true};
      boo::BleedAvoidance BleedAvoidance {boo::BleedAvoidance::None};
      RectEdges<bool> ClosedEdges {true};
      bool AppliedClipAlready {false};
    };

    const RenderElement &_renderer;
    const PaintInfo &_paintInfo;

  public:
    BorderPainter(const RenderElement &, const PaintInfo &) noexcept;

    void PaintBorder(const LayoutRect &, const RenderStyle &, BleedAvoidance = BleedAvoidance::None,
                     RectEdges<bool> closedEdges = {true}) const noexcept;

    bool PaintNinePieceImage(const LayoutRect &, const RenderStyle &, const style::BorderImage &,
                             CompositeOperator = CompositeOperator::SourceOver) const noexcept;

    static void DrawLineForBoxSide(GraphicsContext &, const dom::Document &, const FloatRect &, BoxSide,
                                   Color, BorderStyle, float adjacentWidth1, float adjacentWidth2,
                                   bool antialias = false) noexcept;

    KRYS_NODISCARD static Maybe<Path> PathForBorderArea(const LayoutRect &, const RenderStyle &,
                                                        float deviceScaleFactor,
                                                        RectEdges<bool> closedEdges = {true}) noexcept;

    KRYS_NODISCARD static bool ShouldAntialiasLines(GraphicsContext &) noexcept;
    KRYS_NODISCARD static bool DecorationHasAllSolidEdges(const RectEdges<BorderEdge> &) noexcept;

  private:
    void PaintSides(const BorderShape &, const Sides &) const noexcept;

    template <typename T>
    bool PaintNinePieceImageImpl(const LayoutRect &, const RenderStyle &, const T &,
                                 CompositeOperator = CompositeOperator::SourceOver) const noexcept;

    void PaintTranslucentBorderSides(const BorderShape &, const Sides &, EnumSet<BoxSide> edgesToDraw,
                                     bool antialias) const noexcept;
    void PaintBorderSides(const BorderShape &, const Sides &, EnumSet<BoxSide> edgesToDraw, bool antialias,
                          const Color *overrideColor = nullptr) const noexcept;

    void PaintOneBorderSide(const BorderShape &, const Sides &, const LayoutRect &sideRect, BoxSide,
                            BoxSide adjacentSide1, BoxSide adjacentSide2, const Path *, bool antialias,
                            const Color *overrideColor) const noexcept;

    void DrawBoxSideFromPath(const BorderShape &, const Path &borderPath, const BorderEdges &,
                             float thickness, float drawThickness, BoxSide, Color, BorderStyle,
                             BleedAvoidance) const noexcept;
    void ClipBorderSidePolygon(const BorderShape &, BoxSide, bool firstEdgeMatches,
                               bool secondEdgeMatches) const noexcept;

    KRYS_NODISCARD LayoutRect BorderRectAdjustedForBleedAvoidance(const LayoutRect &,
                                                                  BleedAvoidance) const noexcept;

    KRYS_NODISCARD static Color CalculateBorderStyleColor(const BorderStyle &, const BoxSide &,
                                                          const Color &) noexcept;

    KRYS_NODISCARD const dom::Document &Document() const noexcept;
  };

  /// @brief @p rect inset by a device pixel on each side.
  KRYS_NODISCARD LayoutRect ShrinkRectByOneDevicePixel(const GraphicsContext &, const LayoutRect &,
                                                       float devicePixelRatio) noexcept;
}
