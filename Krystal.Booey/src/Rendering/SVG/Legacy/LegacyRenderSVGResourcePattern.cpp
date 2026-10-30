#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourcePattern.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourcePattern::LegacyRenderSVGResourcePattern(svg::SVGPatternElement &element,
                                                                 RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourcePattern, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourcePattern::~LegacyRenderSVGResourcePattern() noexcept = default;

  svg::SVGPatternElement &LegacyRenderSVGResourcePattern::PatternElement() const noexcept
  {
    return Downcast<svg::SVGPatternElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  void LegacyRenderSVGResourcePattern::RemoveAllClientsFromCache() noexcept
  {
    _patternMap.clear();
    _shouldCollectPatternAttributes = true;
  }

  void LegacyRenderSVGResourcePattern::RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
    bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept
  {
    RemoveAllClientsFromCache();
    MarkAllClientsForInvalidationIfNeeded(markForInvalidation ? InvalidationMode::RepaintInvalidation
                                                              : InvalidationMode::ParentOnlyInvalidation,
                                          visitedRenderers);
  }

  void LegacyRenderSVGResourcePattern::RemoveClientFromCache(RenderElement &client) noexcept
  {
    _patternMap.erase(&client);
  }

  void LegacyRenderSVGResourcePattern::CollectPatternAttributes(
    svg::PatternAttributes &attributes) const noexcept
  {
    const LegacyRenderSVGResourcePattern *current = this;

    while (current)
    {
      current->PatternElement().CollectPatternAttributes(attributes);

      auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*current);
      krys_debug_assert(!resources || !resources->LinkedResource()
                        || Is<LegacyRenderSVGResourcePattern>(*resources->LinkedResource()));
      current =
        resources ? DynamicDowncast<LegacyRenderSVGResourcePattern>(resources->LinkedResource()) : nullptr;
    }
  }

  PatternData *LegacyRenderSVGResourcePattern::BuildPattern(RenderElement &renderer,
                                                            EnumSet<RenderSVGResourceMode> resourceMode,
                                                            GraphicsContext &context) noexcept
  {
    krys_debug_assert(!_shouldCollectPatternAttributes);

    if (auto it = _patternMap.find(&renderer); it != _patternMap.end() && it->second && it->second->Pattern)
    {
      return it->second.get();
    }

    // If we couldn't determine the pattern content element root, stop here.
    if (!_attributes.PatternContentElement())
    {
      return nullptr;
    }

    // An empty viewBox disables rendering.
    if (_attributes.HasViewBox() && _attributes.ViewBox().IsEmpty())
    {
      return nullptr;
    }

    // Compute all necessary transformations to build the tile image & the pattern.
    FloatRect tileBoundaries;
    AffineTransform tileImageTransform;
    if (!BuildTileImageTransform(renderer, _attributes, PatternElement(), tileBoundaries, tileImageTransform))
    {
      return nullptr;
    }

    auto absoluteTransform =
      SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(renderer);

    // Ignore 2D rotation, as it doesn't affect the size of the tile.
    FloatSize tileScale(static_cast<float>(absoluteTransform.XScale()),
                        static_cast<float>(absoluteTransform.YScale()));

    // Scale the tile size to match the scale level of the patternTransform.
    tileScale.Scale(static_cast<float>(_attributes.PatternTransform().XScale()),
                    static_cast<float>(_attributes.PatternTransform().YScale()));

    // Build tile image.
    auto tileImage =
      CreateTileImage(context, tileBoundaries.Size(), tileScale, tileImageTransform, _attributes);
    if (!tileImage)
    {
      return nullptr;
    }

    auto tileImageSize = tileImage->LogicalSize();

    // Compute pattern space transformation.
    auto patternData = CreateUnique<PatternData>();
    patternData->Transform.Translate(tileBoundaries.Location());
    patternData->Transform.Scale(tileBoundaries.Size() / tileImageSize);

    AffineTransform patternTransform = _attributes.PatternTransform();
    if (!patternTransform.IsInvertible())
    {
      return nullptr;
    }

    if (!patternTransform.IsIdentity())
    {
      patternData->Transform = patternTransform * patternData->Transform;
    }

    // Account for text drawing resetting the context to non-scaled, see
    // SVGInlineTextBox::paintTextWithShadows.
    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToText))
    {
      auto textScale = ComputeTextPaintingScale(renderer);
      if (textScale != 1)
      {
        patternData->Transform.Scale(textScale);
      }
    }

    // Build pattern.
    Ref<ImageBuffer> tile = krys::move(tileImage);
    patternData->Pattern =
      Pattern::Create(SourceImage {krys::move(tile)}, {true, true, patternData->Transform});

    // Various calls above may trigger invalidations in some fringe cases (ImageBuffer allocation
    // failures in the SVG image cache for example). To avoid having our PatternData deleted by
    // removeAllClientsFromCacheAndMarkForInvalidation(), we only make it visible in the cache at the very
    // end.
    auto &slot = _patternMap[&renderer];
    slot = krys::move(patternData);
    return slot.get();
  }

  auto LegacyRenderSVGResourcePattern::ApplyResource(RenderElement &renderer, const RenderStyle &style,
                                                     GraphicsContext *&context,
                                                     EnumSet<RenderSVGResourceMode> resourceMode) noexcept
    -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());

    if (_shouldCollectPatternAttributes)
    {
      PatternElement().SynchronizeAllAttributes();

      _attributes = svg::PatternAttributes();
      CollectPatternAttributes(_attributes);
      _shouldCollectPatternAttributes = false;
    }

    // Spec: When the geometry of the applicable element has no width or height and objectBoundingBox is
    // specified, then the given effect (e.g. a gradient or a filter) will be ignored.
    FloatRect objectBoundingBox = renderer.ObjectBoundingBox();
    if (_attributes.PatternUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX
        && objectBoundingBox.IsEmpty())
    {
      return {};
    }

    auto *patternData = BuildPattern(renderer, resourceMode, *context);
    if (!patternData)
    {
      return {};
    }

    // Draw pattern
    context->Save();

    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
    {
      context->SetAlpha(style.FillOpacity().Value.Value);
      context->SetFillPattern(ShareRef(*patternData->Pattern));
      context->SetFillRule(style.GetFillRule());
    }
    else if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
    {
      if (style.GetVectorEffect() == VectorEffect::NonScalingStroke)
      {
        patternData->Pattern->SetPatternSpaceTransform(
          TransformOnNonScalingStroke(&renderer, patternData->Transform));
      }
      context->SetAlpha(style.StrokeOpacity().Value.Value);
      context->SetStrokePattern(ShareRef(*patternData->Pattern));
      SVGRenderSupport::ApplyStrokeStyleToContext(*context, style, renderer);
    }

    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToText))
    {
      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
      {
        context->SetTextDrawingMode(TextDrawingMode::Fill);
      }
      else if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
      {
        context->SetTextDrawingMode(TextDrawingMode::Stroke);
      }
    }

    return {ApplyResult::ResourceApplied};
  }

  void LegacyRenderSVGResourcePattern::PostApplyResource(RenderElement &, GraphicsContext *&context,
                                                         EnumSet<RenderSVGResourceMode> resourceMode,
                                                         const Path *path,
                                                         const RenderElement *shape) noexcept
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());
    FillAndStrokePathOrShape(*context, resourceMode, path, shape);
    context->Restore();
  }

  namespace
  {
    KRYS_NODISCARD FloatRect CalculatePatternBoundaries(const svg::PatternAttributes &attributes,
                                                        const FloatRect &objectBoundingBox,
                                                        const svg::SVGPatternElement &patternElement) noexcept
    {
      return svg::SVGLengthContext::ResolveRectangle(&patternElement, attributes.PatternUnits(),
                                                     objectBoundingBox, attributes.X(), attributes.Y(),
                                                     attributes.Width(), attributes.Height());
    }
  }

  bool LegacyRenderSVGResourcePattern::BuildTileImageTransform(
    RenderElement &renderer, const svg::PatternAttributes &attributes,
    const svg::SVGPatternElement &patternElement, FloatRect &patternBoundaries,
    AffineTransform &tileImageTransform) const noexcept
  {
    FloatRect objectBoundingBox = renderer.ObjectBoundingBox();
    patternBoundaries = CalculatePatternBoundaries(attributes, objectBoundingBox, patternElement);
    if (patternBoundaries.Width() <= 0 || patternBoundaries.Height() <= 0)
    {
      return false;
    }

    AffineTransform viewBoxCTM =
      svg::SVGFitToViewBox::ViewBoxToViewTransform(attributes.ViewBox(), attributes.PreserveAspectRatio(),
                                                   patternBoundaries.Width(), patternBoundaries.Height());

    // Apply viewBox/objectBoundingBox transformations.
    if (!viewBoxCTM.IsIdentity())
    {
      tileImageTransform = viewBoxCTM;
    }
    else if (attributes.PatternContentUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      tileImageTransform.Scale(objectBoundingBox.Width(), objectBoundingBox.Height());
    }

    return true;
  }

  RefPtr<ImageBuffer> LegacyRenderSVGResourcePattern::CreateTileImage(
    GraphicsContext &context, const FloatSize &size, const FloatSize &scale,
    const AffineTransform &tileImageTransform, const svg::PatternAttributes &attributes) const noexcept
  {
    // This is equivalent to making createImageBuffer() use roundedIntSize().
    auto roundedUnscaledImageBufferSize = [](const FloatSize &size, const FloatSize &scale) -> FloatSize
    {
      auto scaledSize = size * scale;
      return size
             - FloatSize(ExpandedIntSize(scaledSize) - RoundedIntSize(scaledSize))
                 * (scaledSize - FloatSize(FlooredIntSize(scaledSize))) / scale;
    };

    auto tileSize = roundedUnscaledImageBufferSize(size, scale);

    // FIXME: Use createImageBuffer(rect, scale), delete the above calculations and fix 'tileImageTransform'
    auto tileImage = context.CreateScaledImageBuffer(tileSize, scale.ExpandedTo(context.ScaleFactor()));
    if (!tileImage)
    {
      return nullptr;
    }

    GraphicsContext &tileImageContext = tileImage->Context();

    // Apply tile image transformations.
    if (!tileImageTransform.IsIdentity())
    {
      tileImageContext.ConcatCTM(tileImageTransform);
    }

    AffineTransform contentTransformation;
    if (attributes.PatternContentUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      contentTransformation = tileImageTransform;
    }

    // Draw the content into the ImageBuffer.
    Ref<const svg::SVGPatternElement> contentElement = ShareRef(*attributes.PatternContentElement());
    for (auto *node = contentElement->FirstChild(); node; node = node->NextSibling())
    {
      auto *child = DynamicDowncast<svg::SVGElement>(*node);
      if (!child || !child->Renderer())
      {
        continue;
      }
      if (child->Renderer()->NeedsLayout())
      {
        return nullptr;
      }
      SVGRenderingContext::RenderSubtreeToContext(tileImageContext, *child->Renderer(),
                                                  contentTransformation);
    }

    return tileImage;
  }
}
