#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceFilter::LegacyRenderSVGResourceFilter(svg::SVGFilterElement &element,
                                                               RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourceFilter, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceFilter::~LegacyRenderSVGResourceFilter() noexcept = default;

  svg::SVGFilterElement &LegacyRenderSVGResourceFilter::FilterElement() const noexcept
  {
    return Downcast<svg::SVGFilterElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  svg::SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceFilter::FilterUnits() const noexcept
  {
    return FilterElement().FilterUnits();
  }

  svg::SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceFilter::PrimitiveUnits() const noexcept
  {
    return FilterElement().PrimitiveUnits();
  }

  bool LegacyRenderSVGResourceFilter::IsIdentity() const noexcept
  {
    return SVGFilterRenderer::IsIdentity(FilterElement());
  }

  void LegacyRenderSVGResourceFilter::RemoveAllClientsFromCache() noexcept
  {
    _rendererFilterDataMap.clear();
  }

  void LegacyRenderSVGResourceFilter::RemoveClientFromCache(RenderElement &client) noexcept
  {
    auto findResult = _rendererFilterDataMap.find(&client);
    if (findResult != _rendererFilterDataMap.end())
    {
      FilterData &filterData = *findResult->second;
      if (filterData.SavedContext)
      {
        filterData.State = FilterData::MarkedForRemoval;
      }
      else
      {
        _rendererFilterDataMap.erase(findResult);
      }
    }
  }

  auto LegacyRenderSVGResourceFilter::ApplyResource(
    RenderElement &renderer, const RenderStyle &, GraphicsContext *&context,
    KRYS_MAYBE_UNUSED EnumSet<RenderSVGResourceMode> resourceMode) noexcept -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(resourceMode.IsEmpty());

    if (auto it = _rendererFilterDataMap.find(&renderer); it != _rendererFilterDataMap.end())
    {
      FilterData *filterData = it->second.get();
      if (filterData->State == FilterData::PaintingSource || filterData->State == FilterData::Applying)
      {
        filterData->State = FilterData::CycleDetected;
        return {}; // Already built, or we're in a cycle, or we're marked for removal. Regardless, just do
                   // nothing more now.
      }

      krys_debug_assert(filterData->TargetSwitcher);
      if (filterData->TargetSwitcher->HasSourceImage())
      {
        return {};
      }

      filterData->TargetSwitcher->BeginDrawSourceImage(*context);
      return {ApplyResult::ResourceApplied};
    }

    auto &filterDataSlot = _rendererFilterDataMap[&renderer];
    filterDataSlot = CreateUnique<FilterData>();
    auto *filterData = filterDataSlot.get();

    auto &filterElement = FilterElement();
    auto *contextElement = DynamicDowncast<svg::SVGElement>(renderer.GetElement());

    auto targetBoundingBox = renderer.ObjectBoundingBox();
    auto filterRegion = svg::SVGLengthContext::ResolveRectangle(
      contextElement, filterElement, filterElement.FilterUnits(), targetBoundingBox);
    if (filterRegion.IsEmpty())
    {
      _rendererFilterDataMap.erase(&renderer);
      return {};
    }

    // Determine absolute transformation matrix for filter.
    auto absoluteTransform =
      SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(renderer);
    if (!absoluteTransform.IsInvertible())
    {
      _rendererFilterDataMap.erase(&renderer);
      return {};
    }

    // Eliminate shear of the absolute transformation matrix, to be able to produce unsheared tile images for
    // feTile.
    FloatSize filterScale(static_cast<float>(absoluteTransform.XScale()),
                          static_cast<float>(absoluteTransform.YScale()));

    // Determine absolute boundaries of the filter and the drawing region.
    filterData->SourceImageRect = renderer.StrokeBoundingBox();
    filterData->SourceImageRect.Intersect(filterRegion);

    // Determine scale factor for filter. The size of intermediate ImageBuffers shouldn't be bigger than
    // kMaxFilterSize.
    (void)ImageBuffer::SizeNeedsClamping(filterData->SourceImageRect.Size(), filterScale);

    auto *page = renderer.GetDocument().GetPage();
    if (!page)
    {
      _rendererFilterDataMap.erase(&renderer);
      return {};
    }
    auto preferredFilterModes = page->PreferredFilterRenderingModes(*context);

    // Create the SVGFilterRenderer object.
    filterData->Filter = SVGFilterRenderer::Create(contextElement, filterElement,
                                                   {
                                                     .ReferenceBox = targetBoundingBox,
                                                     .FilterRegion = filterRegion,
                                                     .Scale = filterScale,
                                                   },
                                                   preferredFilterModes, *context);
    if (!filterData->Filter)
    {
      _rendererFilterDataMap.erase(&renderer);
      return {};
    }

    filterData->Filter->ClampFilterRegionIfNeeded();

    auto &results = filterData->Filter->EnsureResults([] { return CreateUnique<FilterResults>(); });
    filterData->TargetSwitcher = GraphicsContextSwitcher::Create(
      *context, filterData->SourceImageRect, RefPtr<Filter>(filterData->Filter), &results);
    if (!filterData->TargetSwitcher)
    {
      _rendererFilterDataMap.erase(&renderer);
      return {};
    }

    // If the sourceImageRect is empty, we have something like <g filter=".."/>.
    // Even if the target objectBoundingBox() is empty, we still have to draw the last effect result image in
    // postApplyResource.
    if (filterData->SourceImageRect.IsEmpty())
    {
      krys_debug_assert(_rendererFilterDataMap.contains(&renderer));
      filterData->SavedContext = context;
      return {};
    }

    filterData->TargetSwitcher->BeginDrawSourceImage(*context);

    filterData->SavedContext = context;
    context = filterData->TargetSwitcher->DrawingContext(*context);

    krys_debug_assert(_rendererFilterDataMap.contains(&renderer));
    return {ApplyResult::ResourceApplied};
  }

  void LegacyRenderSVGResourceFilter::PostApplyResource(
    RenderElement &renderer, GraphicsContext *&context,
    KRYS_MAYBE_UNUSED EnumSet<RenderSVGResourceMode> resourceMode, const Path *,
    const RenderElement *) noexcept
  {
    krys_debug_assert(context);
    krys_debug_assert(resourceMode.IsEmpty());

    auto findResult = _rendererFilterDataMap.find(&renderer);
    if (findResult == _rendererFilterDataMap.end())
    {
      return;
    }

    FilterData &filterData = *findResult->second;

    switch (filterData.State)
    {
      case FilterData::MarkedForRemoval: _rendererFilterDataMap.erase(findResult); return;

      case FilterData::CycleDetected:
      case FilterData::Applying:
        // We have a cycle if we are already applying the data.
        // This can occur due to FeImage referencing a source that makes use of the FEImage itself.
        // This is the first place we've hit the cycle, so set the state back to PaintingSource so the return
        // stack will continue correctly.
        filterData.State = FilterData::PaintingSource;
        return;

      case FilterData::PaintingSource:
        if (!filterData.SavedContext)
        {
          RemoveClientFromCacheAndMarkForInvalidation(renderer);
          return;
        }

        context = filterData.SavedContext;
        filterData.SavedContext = nullptr;
        break;

      case FilterData::Built: break;
    }

    if (filterData.TargetSwitcher)
    {
      filterData.State = FilterData::Built;
      filterData.TargetSwitcher->EndDrawSourceImage(*context);
    }
  }

  FloatRect LegacyRenderSVGResourceFilter::ResourceBoundingBox(const RenderObject &object,
                                                               RepaintRectCalculation) noexcept
  {
    auto &filterElement = FilterElement();
    auto *renderer = DynamicDowncast<RenderElement>(object);
    if (!renderer)
    {
      return svg::SVGLengthContext::ResolveRectangle(filterElement, filterElement.FilterUnits(),
                                                     object.ObjectBoundingBox());
    }

    auto *contextElement = DynamicDowncast<svg::SVGElement>(renderer->GetElement());
    return svg::SVGLengthContext::ResolveRectangle(contextElement, filterElement, filterElement.FilterUnits(),
                                                   object.ObjectBoundingBox());
  }

  void LegacyRenderSVGResourceFilter::MarkFilterForRepaint(FilterEffect &effect) noexcept
  {
    for (const auto &[object, filterData] : _rendererFilterDataMap)
    {
      if (filterData->State != FilterData::Built)
      {
        continue;
      }

      // Repaint the image on the screen.
      MarkClientForInvalidation(const_cast<RenderObject &>(*object), InvalidationMode::RepaintInvalidation);

      filterData->Filter->ClearEffectResult(effect);
    }
  }

  void LegacyRenderSVGResourceFilter::MarkFilterForRebuild() noexcept
  {
    RemoveAllClientsFromCacheAndMarkForInvalidation();
  }

  FloatRect LegacyRenderSVGResourceFilter::DrawingRegion(RenderObject &object) const noexcept
  {
    auto it = _rendererFilterDataMap.find(&object);
    return it != _rendererFilterDataMap.end() ? it->second->SourceImageRect : FloatRect();
  }

  TextStream &operator<<(TextStream &ts, FilterData::FilterDataState state) noexcept
  {
    switch (state)
    {
      case FilterData::PaintingSource:   ts << u8"painting source"; break;
      case FilterData::Applying:         ts << u8"applying"; break;
      case FilterData::Built:            ts << u8"built"; break;
      case FilterData::CycleDetected:    ts << u8"cycle detected"; break;
      case FilterData::MarkedForRemoval: ts << u8"marked for removal"; break;
    }
    return ts;
  }
}
