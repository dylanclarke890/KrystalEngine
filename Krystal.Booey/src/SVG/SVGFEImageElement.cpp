#include "Krystal.Booey/SVG/SVGFEImageElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Filters/FEImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
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
