#include "Krystal.Booey/Rendering/RenderLayerFilters.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/CSSFilterRenderer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"

namespace krys::boo
{
  Ref<RenderLayerFilters> RenderLayerFilters::Create(RenderLayer &layer, FloatSize scale) noexcept
  {
    return AdoptRef(*new RenderLayerFilters(layer, scale));
  }

  RenderLayerFilters::RenderLayerFilters(RenderLayer &layer, FloatSize scale) noexcept
      : _layer(CreateWeakPtr(&layer)), _filterScale(scale)
  {
  }

  RenderLayerFilters::~RenderLayerFilters() noexcept
  {
    RemoveReferenceFilterClients();
  }

  void RenderLayerFilters::ClearFilter() noexcept
  {
    _filter = nullptr;
  }

  bool RenderLayerFilters::HasFilterThatMovesPixels() const noexcept
  {
    return _filter && _filter->HasFilterThatMovesPixels();
  }

  bool RenderLayerFilters::HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept
  {
    return _filter && _filter->HasFilterThatShouldBeRestrictedBySecurityOrigin();
  }

  bool RenderLayerFilters::HasSourceImage() const noexcept
  {
    return _targetSwitcher && _targetSwitcher->HasSourceImage();
  }

  void RenderLayerFilters::UpdateReferenceFilterClients(const style::Filter &) noexcept
  {
    RemoveReferenceFilterClients();

    // FIXME: Watching the SVG filter documents and elements a reference filter names, so the layer repaints
    // when they change, once there's SVG.
  }

  void RenderLayerFilters::RemoveReferenceFilterClients() noexcept
  {
    // FIXME: The SVG filter documents and elements, once there's SVG.
  }

  bool RenderLayerFilters::IsIdentity(RenderElement &renderer) noexcept
  {
    const auto &filter = renderer.Style().GetFilter();
    return CSSFilterRenderer::IsIdentity(renderer, filter);
  }

  IntOutsets RenderLayerFilters::CalculateOutsets(RenderElement &renderer,
                                                  const FloatRect &targetBoundingBox) noexcept
  {
    const auto &filter = renderer.Style().GetFilter();

    if (!filter.HasFilterThatMovesPixels())
    {
      return {};
    }

    return CSSFilterRenderer::CalculateOutsets(renderer, filter, targetBoundingBox);
  }

  GraphicsContext *RenderLayerFilters::BeginFilterEffect(RenderElement &renderer, GraphicsContext &context,
                                                         const LayoutRect &filterBoxRect,
                                                         const LayoutRect &dirtyRect,
                                                         const LayoutRect &layerRepaintRect,
                                                         const LayoutRect &clipRect) noexcept
  {
    auto *page = renderer.GetDocument().GetPage();
    if (!page)
    {
      return nullptr;
    }
    auto preferredFilterRenderingModes = page->PreferredFilterRenderingModes(context);
    auto outsets = CalculateOutsets(renderer, FloatRect(filterBoxRect));

    auto dirtyFilterRegion = dirtyRect;
    auto filterRegion = dirtyRect;

    // FIXME: SVG shapes' filter regions (their object bounding boxes), once there's SVG.
    if (!outsets.IsZero())
    {
      // FIXME: This flipping was added for drop-shadow, but it's not obvious that it's correct.
      LayoutBoxExtent flippedOutsets {LayoutUnit {outsets.Bottom()}, LayoutUnit {outsets.Left()},
                                      LayoutUnit {outsets.Top()}, LayoutUnit {outsets.Right()}};
      dirtyFilterRegion.Expand(flippedOutsets);
    }

    dirtyFilterRegion = Intersection(filterBoxRect, dirtyFilterRegion);
    filterRegion = dirtyFilterRegion;

    if (!outsets.IsZero())
    {
      filterRegion.Expand(ToLayoutBoxExtent(outsets));
    }

    if (filterRegion.IsEmpty())
    {
      return nullptr;
    }

    auto geometryReferenceGeometryChanged =
      [](const FilterGeometry &existingGeometry, const FilterGeometry &newGeometry)
    {
      return existingGeometry.ReferenceBox != newGeometry.ReferenceBox
             || existingGeometry.Scale != newGeometry.Scale;
    };

    auto geometry = FilterGeometry {
      .ReferenceBox = FloatRect(filterBoxRect),
      .FilterRegion = FloatRect(filterRegion),
      .Scale = _filterScale,
    };

    bool hasUpdatedBackingStore = false;
    if (!_filter || geometryReferenceGeometryChanged(_filter->Geometry(), geometry)
        || _preferredFilterRenderingModes != preferredFilterRenderingModes)
    {
      // FIXME: This rebuilds the entire effects chain even if the filter style didn't change.
      _filter = CSSFilterRenderer::Create(renderer, renderer.Style().GetFilter(), geometry,
                                          preferredFilterRenderingModes, context);
      hasUpdatedBackingStore = true;
    }
    else if (FloatRect(filterRegion) != _filter->FilterRegion())
    {
      _filter->SetFilterRegion(FloatRect(filterRegion));
      hasUpdatedBackingStore = true;
    }

    _preferredFilterRenderingModes = preferredFilterRenderingModes;

    if (!_filter)
    {
      return nullptr;
    }

    Ref<CSSFilterRenderer> filter = _filter;
    if (!filter->HasFilterThatMovesPixels())
    {
      _repaintRect = dirtyRect;
    }
    else if (hasUpdatedBackingStore || !HasSourceImage())
    {
      _repaintRect = filterRegion;
    }
    else
    {
      _repaintRect = dirtyRect;
      _repaintRect.Unite(layerRepaintRect);
      _repaintRect.Intersect(filterRegion);
    }

    ResetDirtySourceRect();

    if (!_targetSwitcher || hasUpdatedBackingStore)
    {
      // FIXME: SVG shapes' source image rects (their object bounding boxes), once there's SVG.
      auto sourceImageRect = FloatRect(dirtyFilterRegion);
      _targetSwitcher = GraphicsContextSwitcher::Create(context, sourceImageRect, filter);
    }

    if (!_targetSwitcher)
    {
      return nullptr;
    }

    _targetSwitcher->BeginClipAndDrawSourceImage(context, FloatRect(_repaintRect), FloatRect(clipRect));

    return _targetSwitcher->DrawingContext(context);
  }

  void RenderLayerFilters::ApplyFilterEffect(GraphicsContext &destinationContext) noexcept
  {
    krys_debug_assert(_targetSwitcher);
    _targetSwitcher->EndClipAndDrawSourceImage(destinationContext);
  }
}
