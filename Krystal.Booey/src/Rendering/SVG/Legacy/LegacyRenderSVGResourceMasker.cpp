/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMaskElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceMasker::LegacyRenderSVGResourceMasker(SVGMaskElement &element,
                                                               RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourceMasker, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceMasker::~LegacyRenderSVGResourceMasker() noexcept = default;

  SVGMaskElement &LegacyRenderSVGResourceMasker::MaskElement() const noexcept
  {
    return Downcast<SVGMaskElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceMasker::MaskUnits() const noexcept
  {
    return MaskElement().MaskUnits();
  }

  SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceMasker::MaskContentUnits() const noexcept
  {
    return MaskElement().MaskContentUnits();
  }

  void LegacyRenderSVGResourceMasker::RemoveAllClientsFromCache() noexcept
  {
    _maskContentBoundaries.fill(RectF {});
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
    RectF decoratedBounds = renderer.DecoratedBoundingBox();

    // Ignore 2D rotation, as it doesn't affect the size of the mask.
    SizeF scale(static_cast<float>(absoluteTransform.XScale()),
                static_cast<float>(absoluteTransform.YScale()));

    // Determine scale factor for the mask. The size of intermediate ImageBuffers shouldn't be bigger than
    // kMaxFilterSize.
    (void)ImageBuffer::SizeNeedsClamping(decoratedBounds.Size(), scale);

    if (!maskerData->MaskImage && !decoratedBounds.IsEmpty())
    {
      // The content is drawn in sRGB; with `color-interpolation: linearRGB`, a luminance mask's luminance
      // is of its colours in linear light.
      const auto &drawColorSpace = Style().GetColorInterpolation() == ColorInterpolation::LinearRGB
                                     ? DestinationColorSpace::LinearSRGB()
                                     : DestinationColorSpace::SRGB();
      maskerData->MaskImage = context->CreateScaledImageBuffer(decoratedBounds, scale);
      if (!maskerData->MaskImage)
      {
        return {};
      }

      if (!DrawContentIntoMaskImage(maskerData, drawColorSpace, &renderer))
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
                                                               const DestinationColorSpace &colorSpace,
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
      maskImage->ConvertToLuminanceMask(colorSpace);
    }

    return true;
  }

  bool LegacyRenderSVGResourceMasker::DrawContentIntoContext(GraphicsContext &context,
                                                             const RectF &objectBoundingBox) noexcept
  {
    // Eventually adjust the mask image context according to the target objectBoundingBox.
    AffineTransform maskContentTransformation;

    auto &maskElement = MaskElement();
    if (maskElement.MaskContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      maskContentTransformation.Translate(objectBoundingBox.Location());
      maskContentTransformation.Scale(objectBoundingBox.Size());
      context.ConcatCTM(maskContentTransformation);
    }

    // Draw the content into the ImageBuffer.
    for (auto *childNode = maskElement.FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *child = DynamicDowncast<SVGElement>(*childNode);
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
                                                             const RectF &destinationRect,
                                                             const RectF &sourceRect,
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
      if (!renderer || !Is<SVGElement>(*childNode))
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

  RectF
    LegacyRenderSVGResourceMasker::ResourceBoundingBox(const RenderObject &object,
                                                       RepaintRectCalculation repaintRectCalculation) noexcept
  {
    RectF objectBoundingBox = object.ObjectBoundingBox();
    auto &maskElement = MaskElement();
    RectF maskBoundaries =
      SVGLengthContext::ResolveRectangle(maskElement, maskElement.MaskUnits(), objectBoundingBox);

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

    RectF maskRect = maskContentBoundaries;
    if (maskElement.MaskContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
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
