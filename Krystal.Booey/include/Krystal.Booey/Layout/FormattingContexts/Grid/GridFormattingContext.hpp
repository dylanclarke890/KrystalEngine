#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/PlacedGridItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class LayoutState;

  /// @brief Lays out the in-flow children of a grid container.
  class GridFormattingContext
  {
    KRYS_NON_COPY_MOVABLE(GridFormattingContext);

  private:
    const ElementBox &_gridBox;
    LayoutState &_globalLayoutState;
    const IntegrationUtils _integrationUtils;

  public:
    GridFormattingContext(const ElementBox &gridBox, LayoutState &layoutState) noexcept;

    void Layout(GridLayoutConstraints layoutConstraints) noexcept;

    KRYS_NODISCARD PlacedGridItems ConstructPlacedGridItems(const GridAreas &gridAreas) const noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _gridBox;
    }
    KRYS_NODISCARD const IntegrationUtils &GetIntegrationUtils() const noexcept
    {
      return _integrationUtils;
    }
    KRYS_NODISCARD const BoxGeometry &GeometryForGridItem(const ElementBox &layoutBox) const noexcept;

  private:
    KRYS_NODISCARD UnplacedGridItems ConstructUnplacedGridItems() const noexcept;
    KRYS_NODISCARD const LayoutState &GetLayoutState() const noexcept
    {
      return _globalLayoutState;
    }
    KRYS_NODISCARD BoxGeometry &GeometryForGridItem(const ElementBox &layoutBox) noexcept;
    void SetGridItemGeometries(const GridItemRects &gridItemRects) noexcept;
  };
}
