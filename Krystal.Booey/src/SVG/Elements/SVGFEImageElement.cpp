/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2010 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2018-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGFEImageElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Filters/FEImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEImageElement::SVGFEImageElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEImage),
        SVGURIReference(this), _preserveAspectRatio(SVGAnimatedPreserveAspectRatio::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEImageElement::_preserveAspectRatio>(
        SVGNames::PreserveAspectRatioAttr());
    }
  }

  SVGFEImageElement::~SVGFEImageElement() noexcept
  {
    ClearResourceReferences();
  }

  bool SVGFEImageElement::RenderingTaintsOrigin() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }
    auto *image = _cachedImage->GetImage();
    return image && image->RenderingTaintsOrigin();
  }

  void SVGFEImageElement::ClearResourceReferences() noexcept
  {
    if (auto cachedImage = std::exchange(_cachedImage, CachedResourceHandle<CachedImage>()))
    {
      cachedImage->RemoveClient(*this);
    }

    RemoveElementReference();
  }

  void SVGFEImageElement::RequestImageResource() noexcept
  {
    ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();
    options.ContentSecurityPolicy = IsInUserAgentShadowTree()
                                      ? ContentSecurityPolicyImposition::SkipPolicyCheck
                                      : ContentSecurityPolicyImposition::DoPolicyCheck;

    auto &document = NodeDocument();
    CachedResourceRequest request(ResourceRequest(document.CompleteURL(Href())), options);
    request.SetInitiatorType(LocalName().View());
    auto requested = document.GetCachedResourceLoader().RequestImage(krys::move(request));
    _cachedImage = requested ? *requested : CachedResourceHandle<CachedImage>();

    if (_cachedImage)
    {
      _cachedImage->AddClient(*this);
    }
  }

  void SVGFEImageElement::BuildPendingResource() noexcept
  {
    ClearResourceReferences();
    if (!IsConnected())
    {
      return;
    }

    auto target = SVGURIReference::TargetElementFromIRIString(Href(), TreeScopeForSVGReferences());
    if (!target.Element)
    {
      if (target.Identifier.View().empty())
      {
        RequestImageResource();
      }
      else
      {
        TreeScopeForSVGReferences().AddPendingSVGResource(target.Identifier, *this);
        krys_debug_assert(HasPendingResources());
      }
    }
    else if (auto *element = DynamicDowncast<SVGElement>(target.Element.get()))
    {
      element->AddReferencingElement(*this);
    }

    UpdateSVGRendererForElementChange();
  }

  void SVGFEImageElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::PreserveAspectRatioAttr())
    {
      _preserveAspectRatio->SetBaseValInternal(
        SVGPreserveAspectRatioValue {value.value_or(dom::DOMStringView {})});
    }

    SVGURIReference::ParseAttribute(name, value);
    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEImageElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::PreserveAspectRatioAttr());
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      BuildPendingResource();
      MarkFilterEffectForRebuild();
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  void SVGFEImageElement::OnPostConnection() noexcept
  {
    SVGFilterPrimitiveStandardAttributes::OnPostConnection();
    BuildPendingResource();
  }

  void SVGFEImageElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    bool disconnectedFromDocument = oldAncestor.IsConnected() && !IsConnected();
    SVGFilterPrimitiveStandardAttributes::OnRemove(isSubtreeRoot, oldAncestor);
    if (disconnectedFromDocument)
    {
      ClearResourceReferences();
    }
  }

  void SVGFEImageElement::NotifyFinished(CachedResource &) noexcept
  {
    if (!IsConnected())
    {
      return;
    }

    auto *parent = DynamicDowncast<SVGElement>(ParentElement());
    if (!parent || !Is<SVGFilterElement>(*parent))
    {
      return;
    }

    auto *parentRenderer = parent->Renderer();
    if (!parentRenderer)
    {
      return;
    }

    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(*parentRenderer);
  }

  auto SVGFEImageElement::GetImageBufferForEffect(const GraphicsContext &destinationContext) const noexcept
    -> ImageBufferForEffect
  {
    auto target = SVGURIReference::TargetElementFromIRIString(
      Href(), const_cast<SVGFEImageElement &>(*this).TreeScopeForSVGReferences());
    auto *targetElement = DynamicDowncast<SVGElement>(target.Element.get());
    if (!targetElement)
    {
      return {};
    }

    if (IsShadowIncludingDescendantOf(targetElement))
    {
      return {};
    }

    auto *renderer = targetElement->Renderer();
    if (!renderer)
    {
      return {};
    }

    auto absoluteTransform =
      SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(*renderer);
    if (!absoluteTransform.IsInvertible())
    {
      return {};
    }

    // Ignore 2D rotation, as it doesn't affect the image size.
    FloatSize scale(static_cast<float>(absoluteTransform.XScale()),
                    static_cast<float>(absoluteTransform.YScale()));
    auto imageRect = renderer->RepaintRectInLocalCoordinates();

    auto imageBuffer = destinationContext.CreateScaledImageBuffer(imageRect, scale);
    if (!imageBuffer)
    {
      return {};
    }

    auto &context = imageBuffer->Context();
    SVGRenderingContext::RenderSubtreeToContext(context, *renderer, AffineTransform());

    return {krys::move(imageBuffer), imageRect};
  }

  RefPtr<FilterEffect>
    SVGFEImageElement::CreateFilterEffect(const FilterEffectVector &,
                                          const GraphicsContext &destinationContext) const noexcept
  {
    if (_cachedImage)
    {
      auto *image = _cachedImage->ImageForRenderer(Renderer());
      if (!image || image->IsNull())
      {
        return nullptr;
      }

      auto nativeImage = image->CurrentPreTransformedNativeImage();
      if (!nativeImage)
      {
        return nullptr;
      }

      auto imageRect = FloatRect {{}, image->Size()};
      return FEImage::Create({Ref<NativeImage>(nativeImage)}, imageRect, PreserveAspectRatio());
    }

    auto [imageBuffer, imageRect] = GetImageBufferForEffect(destinationContext);
    if (!imageBuffer)
    {
      return nullptr;
    }

    return FEImage::Create({Ref<ImageBuffer>(imageBuffer)}, imageRect, PreserveAspectRatio());
  }
}
