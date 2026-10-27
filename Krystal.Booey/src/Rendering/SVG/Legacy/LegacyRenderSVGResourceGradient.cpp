#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceGradient::LegacyRenderSVGResourceGradient(Type type, svg::SVGGradientElement &node,
                                                                   RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(type, node, krys::move(style))
  {
  }

  LegacyRenderSVGResourceGradient::~LegacyRenderSVGResourceGradient() noexcept = default;

  void LegacyRenderSVGResourceGradient::RemoveAllClientsFromCache() noexcept
  {
    _gradientMap.clear();
    _shouldCollectGradientAttributes = true;
  }

  void LegacyRenderSVGResourceGradient::RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
    bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept
  {
    RemoveAllClientsFromCache();
    MarkAllClientsForInvalidationIfNeeded(markForInvalidation ? InvalidationMode::RepaintInvalidation
                                                              : InvalidationMode::ParentOnlyInvalidation,
                                          visitedRenderers);
  }

  void LegacyRenderSVGResourceGradient::RemoveClientFromCache(RenderElement &client) noexcept
  {
    _gradientMap.erase(&client);
  }

  GradientData::Inputs
    LegacyRenderSVGResourceGradient::ComputeInputs(RenderElement &renderer,
                                                   EnumSet<RenderSVGResourceMode> resourceMode) noexcept
  {
    Maybe<FloatRect> objectBoundingBox;
    if (GradientUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      objectBoundingBox = renderer.ObjectBoundingBox();
    }

    float textPaintingScale = 1;
    // FIXME: The text painting scale (computeTextPaintingScale), with text (B11g).
    (void)resourceMode;

    return {objectBoundingBox, textPaintingScale};
  }

  GradientData *LegacyRenderSVGResourceGradient::GradientDataForRenderer(
    RenderElement &renderer, const RenderStyle &style, EnumSet<RenderSVGResourceMode> resourceMode) noexcept
  {
    // Be sure to synchronize all SVG properties on the gradientElement _before_ processing any further.
    // Otherwhise the call to collectGradientAttributes() in createTileImage(), may cause the SVG DOM property
    // synchronization to kick in, which causes removeAllClientsFromCacheAndMarkForInvalidation() to be
    // called, which in turn deletes our GradientData object! Leaving out the line below will cause
    // svg/dynamic-updates/SVG*GradientElement-svgdom* to crash.
    if (_shouldCollectGradientAttributes)
    {
      GradientElement().SynchronizeAllAttributes();
      if (!CollectGradientAttributes())
      {
        return nullptr;
      }

      _shouldCollectGradientAttributes = false;
    }

    // Spec: When the geometry of the applicable element has no width or height and objectBoundingBox is
    // specified, then the given effect (e.g. a gradient or a filter) will be ignored.
    auto inputs = ComputeInputs(renderer, resourceMode);
    if (inputs.ObjectBoundingBox && inputs.ObjectBoundingBox->IsEmpty())
    {
      return nullptr;
    }

    auto &slot = _gradientMap[&renderer];
    if (!slot)
    {
      slot = CreateUnique<GradientData>();
    }
    auto &gradientData = *slot;

    if (!GradientTransform().IsInvertible())
    {
      return nullptr;
    }

    if (gradientData.Invalidate(inputs))
    {
      gradientData.Gradient = BuildGradient(style);
      krys_debug_assert(gradientData.UserspaceTransform.IsIdentity());

      // CG platforms will handle the gradient space transform for text after applying the
      // resource, so don't apply it here. For non-CG platforms, we want the text bounding
      // box applied to the gradient space transform now, so the gradient shader can use it.
      if (gradientData.GradientInputs.ObjectBoundingBox)
      {
        gradientData.UserspaceTransform.Translate(gradientData.GradientInputs.ObjectBoundingBox->Location());
        gradientData.UserspaceTransform.Scale(gradientData.GradientInputs.ObjectBoundingBox->Size());
      }

      gradientData.UserspaceTransform *= GradientTransform();

      // Depending on font scaling factor, we may need to rescale the gradient here since
      // text painting removes the scale factor from the context.
      if (gradientData.GradientInputs.TextPaintingScale != 1)
      {
        gradientData.UserspaceTransform.Scale(gradientData.GradientInputs.TextPaintingScale);
      }
    }

    return &gradientData;
  }

  namespace
  {
    void ApplyGradientResource(RenderElement &renderer, const RenderStyle &style, GraphicsContext &context,
                               const GradientData &gradientData,
                               EnumSet<RenderSVGResourceMode> resourceMode) noexcept
    {
      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToText))
      {
        context.SetTextDrawingMode(resourceMode.Contains(RenderSVGResourceMode::ApplyToFill)
                                     ? TextDrawingMode::Fill
                                     : TextDrawingMode::Stroke);
      }

      auto userspaceTransform = gradientData.UserspaceTransform;

      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
      {
        context.SetAlpha(style.FillOpacity().Value.Value);
        context.SetFillGradient(ShareRef(*gradientData.Gradient), userspaceTransform);
        context.SetFillRule(style.GetFillRule());
      }
      else if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
      {
        if (style.GetVectorEffect() == VectorEffect::NonScalingStroke)
        {
          userspaceTransform = LegacyRenderSVGResourceContainer::TransformOnNonScalingStroke(
            &renderer, gradientData.UserspaceTransform);
        }
        context.SetAlpha(style.StrokeOpacity().Value.Value);
        context.SetStrokeGradient(ShareRef(*gradientData.Gradient), userspaceTransform);
        SVGRenderSupport::ApplyStrokeStyleToContext(context, style, renderer);
      }
    }

    class PathOrShapeGradientApplier final : public GradientApplier
    {
      KRYS_NON_COPY_MOVABLE(PathOrShapeGradientApplier);

    public:
      PathOrShapeGradientApplier() noexcept = default;

    private:
      bool ApplyResource(RenderElement &renderer, const RenderStyle &style, GraphicsContext *&context,
                         const GradientData &gradientData,
                         EnumSet<RenderSVGResourceMode> resourceMode) noexcept final
      {
        context->Save();
        ApplyGradientResource(renderer, style, *context, gradientData, resourceMode);
        return true;
      }

      void PostApplyResource(RenderElement &, GraphicsContext *&context, const GradientData &,
                             svg::SVGUnitTypes::SVGUnitType, const AffineTransform &,
                             EnumSet<RenderSVGResourceMode> resourceMode, const Path *path,
                             const RenderElement *shape) noexcept final
      {
        LegacyRenderSVGResource::FillAndStrokePathOrShape(*context, resourceMode, path, shape);
        context->Restore();
      }
    };
  }

  auto LegacyRenderSVGResourceGradient::ApplyResource(RenderElement &renderer, const RenderStyle &style,
                                                      GraphicsContext *&context,
                                                      EnumSet<RenderSVGResourceMode> resourceMode) noexcept
    -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());

    auto *gradientData = GradientDataForRenderer(renderer, style, resourceMode);
    if (!gradientData)
    {
      return {};
    }

    if (!_gradientApplier)
    {
      _gradientApplier = CreateUnique<PathOrShapeGradientApplier>();
    }

    if (!_gradientApplier->ApplyResource(renderer, style, context, *gradientData, resourceMode))
    {
      _gradientApplier = nullptr;
      return {};
    }

    return {ApplyResult::ResourceApplied};
  }

  void LegacyRenderSVGResourceGradient::PostApplyResource(RenderElement &renderer, GraphicsContext *&context,
                                                          EnumSet<RenderSVGResourceMode> resourceMode,
                                                          const Path *path,
                                                          const RenderElement *shape) noexcept
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());

    if (!_gradientApplier)
    {
      return;
    }

    if (auto gradientData = _gradientMap.find(&renderer); gradientData != _gradientMap.end())
    {
      _gradientApplier->PostApplyResource(renderer, context, *gradientData->second, GradientUnits(),
                                          GradientTransform(), resourceMode, path, shape);
    }

    _gradientApplier = nullptr;
  }

  GradientColorStops
    LegacyRenderSVGResourceGradient::StopsByApplyingColorFilter(const GradientColorStops &stops,
                                                                const RenderStyle &style) noexcept
  {
    if (!style.HasAppleColorFilter())
    {
      return stops;
    }

    return stops.MapColors([&](const Color &color) { return style.ColorByApplyingColorFilter(color); });
  }

  GradientSpreadMethod
    LegacyRenderSVGResourceGradient::PlatformSpreadMethodFromSVGType(svg::SVGSpreadMethodType method) noexcept
  {
    switch (method)
    {
      case svg::SVGSpreadMethodUnknown:
      case svg::SVGSpreadMethodPad:
      {
        return GradientSpreadMethod::Pad;
      }
      case svg::SVGSpreadMethodReflect:
      {
        return GradientSpreadMethod::Reflect;
      }
      case svg::SVGSpreadMethodRepeat:
      {
        return GradientSpreadMethod::Repeat;
      }
    }

    krys_debug_assert(false);
    return GradientSpreadMethod::Pad;
  }
}
