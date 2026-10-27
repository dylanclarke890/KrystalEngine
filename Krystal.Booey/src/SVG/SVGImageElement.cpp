#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGImageLoader.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGImageElement::SVGImageElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Image),
        SVGURIReference(this), _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _preserveAspectRatio(SVGAnimatedPreserveAspectRatio::Create(this)),
        _imageLoader(CreateUnique<SVGImageLoader>(*this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGImageElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGImageElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGImageElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGImageElement::_height>(SVGNames::HeightAttr());
      PropertyRegistry::RegisterProperty<&SVGImageElement::_preserveAspectRatio>(
        SVGNames::PreserveAspectRatioAttr());
    }
  }

  SVGImageElement::~SVGImageElement() noexcept = default;

  CachedImage *SVGImageElement::GetCachedImage() const noexcept
  {
    return _imageLoader->Image();
  }

  void SVGImageElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                         Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::PreserveAspectRatioAttr())
    {
      _preserveAspectRatio->SetBaseValInternal(SVGPreserveAspectRatioValue {string});
      return;
    }
    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Forbid));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Forbid));
    }
    // FIXME: Load again when the CORS setting of crossorigin changes, once there are network loads.
    ReportAttributeParsingError(parseError, name, string);

    SVGURIReference::ParseAttribute(name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGImageElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::XAttr() || attributeName == SVGNames::YAttr())
      {
        UpdateRelativeLengthsInformation();

        if (auto *image = DynamicDowncast<LegacyRenderSVGImage>(Renderer()))
        {
          if (!image->UpdateImageViewport())
          {
            return;
          }
          UpdateSVGRendererForElementChange();
        }
      }
      else if (attributeName == SVGNames::WidthAttr() || attributeName == SVGNames::HeightAttr())
      {
        SetPresentationalHintStyleIsDirty();
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::PreserveAspectRatioAttr());
        UpdateSVGRendererForElementChange();
      }
      return;
    }

    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      _imageLoader->UpdateFromElementIgnoringPreviousError();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGImageElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGImage>(*this, krys::move(style));
  }

  bool SVGImageElement::HaveLoadedRequiredResources() noexcept
  {
    return !_imageLoader->HasPendingActivity();
  }

  void SVGImageElement::DidAttachRenderers() noexcept
  {
    SVGGraphicsElement::DidAttachRenderers();

    if (auto *image = DynamicDowncast<LegacyRenderSVGImage>(Renderer());
        image && !image->ImageResource().GetCachedImage())
    {
      image->ImageResource().SetCachedImage(CachedResourceHandle<CachedImage>(_imageLoader->Image()));
      return;
    }
  }

  void SVGImageElement::OnInsert() noexcept
  {
    SVGGraphicsElement::OnInsert();
    if (!IsConnected())
    {
      return;
    }
    // Update image loader, as soon as we're living in the tree.
    // We can only resolve base URIs properly, after that!
    _imageLoader->UpdateFromElement();
  }

  Maybe<dom::DOMString> SVGImageElement::ImageSourceURL() const noexcept
  {
    if (auto href = GetAttribute(SVGNames::HrefAttr().LocalName()))
    {
      return href;
    }
    return GetAttributeNS(XLinkNames::HrefAttr().NamespaceURI(), XLinkNames::HrefAttr().LocalName());
  }

  Maybe<dom::DOMString> SVGImageElement::CrossOrigin() const noexcept
  {
    // https://html.spec.whatwg.org/#cors-settings-attribute
    auto value = GetAttribute(u8"crossorigin");
    if (!value)
    {
      return null;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"use-credentials"))
    {
      return dom::DOMString(u8"use-credentials");
    }
    return dom::DOMString(u8"anonymous");
  }

  void SVGImageElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    if (value)
    {
      (void)SetAttribute(u8"crossorigin", krys::move(*value));
    }
    else
    {
      RemoveAttribute(u8"crossorigin");
    }
  }

  void SVGImageElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    _imageLoader->ElementDidMoveToNewDocument(oldDocument);
    SVGGraphicsElement::OnAdopt(oldDocument);
  }
}
