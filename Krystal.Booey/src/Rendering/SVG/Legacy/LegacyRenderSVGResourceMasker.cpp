#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGMaskElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceMasker::LegacyRenderSVGResourceMasker(svg::SVGMaskElement &element,
                                                               RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourceMasker, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceMasker::~LegacyRenderSVGResourceMasker() noexcept = default;

  svg::SVGMaskElement &LegacyRenderSVGResourceMasker::MaskElement() const noexcept
  {
    return Downcast<svg::SVGMaskElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  svg::SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceMasker::MaskUnits() const noexcept
  {
    return MaskElement().MaskUnits();
  }

  svg::SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceMasker::MaskContentUnits() const noexcept
  {
    return MaskElement().MaskContentUnits();
  }

  void LegacyRenderSVGResourceMasker::RemoveAllClientsFromCache() noexcept
  {
    _maskContentBoundaries.fill(FloatRect {});
    _masker.clear();
  }

  void LegacyRenderSVGResourceMasker::RemoveClientFromCache(RenderElement &client) noexcept
  {
    _masker.erase(&client);
  }

  auto LegacyRenderSVGResourceMasker::ApplyResource(
    RenderElement &renderer, const RenderStyle &, GraphicsContext *&context,
    KRYS_MAYBE_UNUSED EnumSet<RenderSVGResourceMode> resourceMode) noexcept -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(resourceMode.IsEmpty());

    bool missingMaskerData = !_masker.contains(&renderer);
    if (missingMaskerData)
    {
      _masker[&renderer] = CreateUnique<MaskerData>();
    }

    MaskerData *maskerData = _masker[&renderer].get();
    AffineTransform absoluteTransform =
      SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(renderer);
    FloatRect decoratedBounds = renderer.DecoratedBoundingBox();

    // Ignore 2D rotation, as it doesn't affect the size of the mask.
    FloatSize scale(static_cast<float>(absoluteTransform.XScale()),
                    static_cast<float>(absoluteTransform.YScale()));

    // Determine scale factor for the mask. The size of intermediate ImageBuffers shouldn't be bigger than
    // kMaxFilterSize.
    (void)ImageBuffer::SizeNeedsClamping(decoratedBounds.Size(), scale);

    if (!maskerData->MaskImage && !decoratedBounds.IsEmpty())
    {
      // FIXME: A mask image in linear sRGB for `color-interpolation: linearRGB`, once image buffers have
      // colour spaces.
      maskerData->MaskImage = context->CreateScaledImageBuffer(decoratedBounds, scale);
      if (!maskerData->MaskImage)
      {
        return {};
      }

      if (!DrawContentIntoMaskImage(maskerData, &renderer))
      {
        maskerData->MaskImage = nullptr;
      }
    }

    if (!maskerData->MaskImage)
    {
      return {};
    }

    SVGRenderingContext::ClipToImageBuffer(*context, decoratedBounds, scale, maskerData->MaskImage,
                                           missingMaskerData);
    return {ApplyResult::ResourceApplied};
  }

  bool LegacyRenderSVGResourceMasker::DrawContentIntoMaskImage(MaskerData *maskerData,
                                                               RenderObject *object) noexcept
  {
    RefPtr<ImageBuffer> maskImage = maskerData->MaskImage;
    auto &maskImageContext = maskImage->Context();
    auto objectBoundingBox = object->ObjectBoundingBox();

    if (!DrawContentIntoContext(maskImageContext, objectBoundingBox))
    {
      return false;
    }

    // Create the luminance mask.
    if (Style().GetMaskType() == MaskType::Luminance)
    {
      maskImage->ConvertToLuminanceMask();
    }

    return true;
  }

  bool LegacyRenderSVGResourceMasker::DrawContentIntoContext(GraphicsContext &context,
                                                             const FloatRect &objectBoundingBox) noexcept
  {
    // Eventually adjust the mask image context according to the target objectBoundingBox.
    AffineTransform maskContentTransformation;

    auto &maskElement = MaskElement();
    if (maskElement.MaskContentUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      maskContentTransformation.Translate(objectBoundingBox.Location());
      maskContentTransformation.Scale(objectBoundingBox.Size());
      context.ConcatCTM(maskContentTransformation);
    }

    // Draw the content into the ImageBuffer.
    for (auto *childNode = maskElement.FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *child = DynamicDowncast<svg::SVGElement>(*childNode);
      if (!child)
      {
        continue;
      }
      auto *renderer = child->Renderer();
      if (!renderer)
      {
        continue;
      }
      if (renderer->NeedsLayout())
      {
        return false;
      }
      const auto &style = renderer->Style();
      if (style.Display() == DisplayType::None || style.UsedVisibility() != Visibility::Visible)
      {
        continue;
      }
      SVGRenderingContext::RenderSubtreeToContext(context, *renderer, maskContentTransformation);
    }

    return true;
  }

  bool LegacyRenderSVGResourceMasker::DrawContentIntoContext(GraphicsContext &context,
                                                             const FloatRect &destinationRect,
                                                             const FloatRect &sourceRect,
                                                             ImagePaintingOptions options) noexcept
  {
    GraphicsContextStateSaver stateSaver(context);

    context.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());

    context.Translate(destinationRect.Location());

    if (destinationRect.Size() != sourceRect.Size())
    {
      context.Scale(destinationRect.Size() / sourceRect.Size());
    }

    context.Translate(-sourceRect.X(), -sourceRect.Y());

    return DrawContentIntoContext(context, {{}, destinationRect.Size()});
  }

  void LegacyRenderSVGResourceMasker::CalculateMaskContentRepaintRect(
    RepaintRectCalculation repaintRectCalculation) noexcept
  {
    auto &maskContentBoundaries = _maskContentBoundaries[krys::ToUnderlying(repaintRectCalculation)];
    for (auto *childNode = MaskElement().FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *renderer = DynamicDowncast<RenderElement>(childNode->Renderer());
      if (!renderer || !Is<svg::SVGElement>(*childNode))
      {
        continue;
      }
      const auto &style = renderer->Style();
      if (style.Display() == DisplayType::None || style.UsedVisibility() != Visibility::Visible)
      {
        continue;
      }
      maskContentBoundaries.Unite(renderer->LocalToParentTransform().MapRect(
        renderer->RepaintRectInLocalCoordinates(repaintRectCalculation)));
    }
  }

  FloatRect
    LegacyRenderSVGResourceMasker::ResourceBoundingBox(const RenderObject &object,
                                                       RepaintRectCalculation repaintRectCalculation) noexcept
  {
    FloatRect objectBoundingBox = object.ObjectBoundingBox();
    auto &maskElement = MaskElement();
    FloatRect maskBoundaries =
      svg::SVGLengthContext::ResolveRectangle(maskElement, maskElement.MaskUnits(), objectBoundingBox);

    // Resource was not layouted yet. Give back clipping rect of the mask.
    if (SelfNeedsLayout())
    {
      return maskBoundaries;
    }

    auto &maskContentBoundaries = _maskContentBoundaries[krys::ToUnderlying(repaintRectCalculation)];
    if (maskContentBoundaries.IsEmpty())
    {
      CalculateMaskContentRepaintRect(repaintRectCalculation);
    }

    FloatRect maskRect = maskContentBoundaries;
    if (maskElement.MaskContentUnits() == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      AffineTransform transform;
      transform.Translate(objectBoundingBox.Location());
      transform.Scale(objectBoundingBox.Size());
      maskRect = transform.MapRect(maskRect);
    }

    maskRect.Intersect(maskBoundaries);
    return maskRect;
  }
}
