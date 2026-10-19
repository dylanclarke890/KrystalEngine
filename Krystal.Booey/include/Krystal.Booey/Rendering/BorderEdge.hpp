#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  class RenderStyle;

  /// @brief One side of a border (or outline) as painting sees it: its width, colour and style, and whether
  /// it's there at all.
  class BorderEdge
  {
  private:
    boo::Color _color;
    LayoutUnit _width;
    float _flooredToDevicePixelWidth {0};
    float _devicePixelRatio {1};
    BorderStyle _style {BorderStyle::Hidden};
    bool _isTransparent {false};
    bool _isPresent {false};

  public:
    BorderEdge() noexcept = default;
    BorderEdge(float edgeWidth, boo::Color edgeColor, BorderStyle edgeStyle, bool edgeIsTransparent,
               bool edgeIsPresent, float devicePixelRatio) noexcept;

    KRYS_NODISCARD BorderStyle Style() const noexcept
    {
      return _style;
    }
    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      return _width;
    }
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }
    KRYS_NODISCARD bool IsTransparent() const noexcept
    {
      return _isTransparent;
    }
    KRYS_NODISCARD bool IsPresent() const noexcept
    {
      return _isPresent;
    }

    KRYS_NODISCARD bool HasVisibleColorAndStyle() const noexcept
    {
      return _style > BorderStyle::Hidden && !_isTransparent;
    }
    KRYS_NODISCARD bool ShouldRender() const noexcept
    {
      return _isPresent && WidthForPainting() && HasVisibleColorAndStyle();
    }
    KRYS_NODISCARD bool PresentButInvisible() const noexcept
    {
      return WidthForPainting() && !HasVisibleColorAndStyle();
    }
    KRYS_NODISCARD float WidthForPainting() const noexcept
    {
      return _isPresent ? _flooredToDevicePixelWidth : 0;
    }

    void GetDoubleBorderStripeWidths(LayoutUnit &outerWidth, LayoutUnit &innerWidth) const noexcept;
    KRYS_NODISCARD bool ObscuresBackgroundEdge(float scale) const noexcept;
    KRYS_NODISCARD bool ObscuresBackground() const noexcept;

  private:
    KRYS_NODISCARD float BorderWidthInDevicePixel(int32 logicalPixels) const noexcept
    {
      return LayoutUnit(static_cast<float>(logicalPixels) / _devicePixelRatio).ToFloat();
    }
  };

  using BorderEdges = RectEdges<BorderEdge>;

  // inflation is only added to edges with non-zero widths.
  KRYS_NODISCARD BorderEdges GetBorderEdges(const RenderStyle &, float deviceScaleFactor,
                                            RectEdges<bool> closedEdges = RectEdges<bool> {true},
                                            LayoutSize inflation = {},
                                            bool setColorsToBlack = false) noexcept;
  KRYS_NODISCARD BorderEdges GetBorderEdgesForOutline(const RenderStyle &, BorderStyle,
                                                      float deviceScaleFactor) noexcept;

  KRYS_NODISCARD inline bool EdgesShareColor(const BorderEdge &firstEdge,
                                             const BorderEdge &secondEdge) noexcept
  {
    return EqualIgnoringSemanticColor(firstEdge.GetColor(), secondEdge.GetColor());
  }

  KRYS_NODISCARD inline bool IncludesAdjacentEdges(EnumSet<BoxSide> sides) noexcept
  {
    // The set includes adjacent edges if and only if it contains at least one horizontal and one vertical
    // edge.
    return sides.ContainsAny({BoxSide::Top, BoxSide::Bottom})
           && sides.ContainsAny({BoxSide::Left, BoxSide::Right});
  }
}
