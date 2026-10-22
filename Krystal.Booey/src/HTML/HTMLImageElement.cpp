#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLImageLoader.hpp"
#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/HTMLSourceElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLSrcsetParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD dom::DOMStringView
      ExtractMIMETypeFromTypeAttributeForLookup(dom::DOMStringView typeAttribute) noexcept
    {
      auto semicolonIndex = typeAttribute.find(u8';');
      if (semicolonIndex == dom::DOMStringView::npos)
      {
        return infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(typeAttribute);
      }
      return infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(
        typeAttribute.substr(0, semicolonIndex));
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &ScreenAtom() noexcept
    {
      static const css::CSSOMStringAtom screen {u8"screen"};
      return screen;
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &PrintAtom() noexcept
    {
      static const css::CSSOMStringAtom print {u8"print"};
      return print;
    }

    KRYS_NODISCARD uint32 ClampedAttributeLength(uint64 value) noexcept
    {
      return static_cast<uint32>(std::min<uint64>(value, std::numeric_limits<int32>::max()));
    }
  }

  HTMLImageElement::HTMLImageElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Image), _imageLoader(CreateUnique<HTMLImageLoader>(*this))
  {
  }

  HTMLImageElement::~HTMLImageElement() noexcept = default;

#pragma region HTMLImageElement

  dom::DOMString HTMLImageElement::Alt() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLImageElement::Alt(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLImageElement::Src() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLImageElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLImageElement::SrcSet() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"srcset");
  }

  dom::ExceptionOr<void> HTMLImageElement::SrcSet(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Sizes() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLImageElement::Sizes(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLImageElement::CrossOrigin() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): CrossOrigin() is limited to known values.
    return Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin");
  }

  void HTMLImageElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLImageElement::UseMap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLImageElement::UseMap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

  bool HTMLImageElement::IsMap() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"ismap");
  }

  void HTMLImageElement::IsMap(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"ismap", krys::move(value));
  }

  bool HTMLImageElement::Controls() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"controls");
  }

  void HTMLImageElement::Controls(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"controls", krys::move(value));
  }

  uint32 HTMLImageElement::Width() const noexcept
  {
    if (IsConnected())
    {
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    }

    if (!Renderer())
    {
      // check the attribute first for an explicit pixel value
      if (auto width = Numbers::ParseNonNegativeInteger(AttributeOrEmpty(u8"width")); width.Success())
      {
        return ClampedAttributeLength(width.Value);
      }

      // if the image is available, use its width
      if (auto *image = _imageLoader->Image())
      {
        return image->ImageSizeForRenderer(nullptr, 1.0f).Width().ToUnsigned();
      }
    }

    auto *box = DynamicDowncast<RenderBox>(Renderer());
    if (!box)
    {
      return 0;
    }
    LayoutRect contentRect = box->ContentBoxRect();
    return static_cast<uint32>(AdjustLayoutUnitForAbsoluteZoom(contentRect.Width(), box->Style()).Round());
  }

  void HTMLImageElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLImageElement::Height() const noexcept
  {
    if (IsConnected())
    {
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    }

    if (!Renderer())
    {
      // check the attribute first for an explicit pixel value
      if (auto height = Numbers::ParseNonNegativeInteger(AttributeOrEmpty(u8"height")); height.Success())
      {
        return ClampedAttributeLength(height.Value);
      }

      // if the image is available, use its height
      if (auto *image = _imageLoader->Image())
      {
        return image->ImageSizeForRenderer(nullptr, 1.0f).Height().ToUnsigned();
      }
    }

    auto *box = DynamicDowncast<RenderBox>(Renderer());
    if (!box)
    {
      return 0;
    }
    LayoutRect contentRect = box->ContentBoxRect();
    return static_cast<uint32>(AdjustLayoutUnitForAbsoluteZoom(contentRect.Height(), box->Style()).Round());
  }

  void HTMLImageElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  uint32 HTMLImageElement::NaturalWidth() const noexcept
  {
    auto *image = _imageLoader->Image();
    if (!image)
    {
      return 0u;
    }

    return image->UnclampedImageSizeForRenderer(Renderer(), EffectiveImageDevicePixelRatio())
      .Width()
      .ToUnsigned();
  }

  uint32 HTMLImageElement::NaturalHeight() const noexcept
  {
    auto *image = _imageLoader->Image();
    if (!image)
    {
      return 0u;
    }

    return image->UnclampedImageSizeForRenderer(Renderer(), EffectiveImageDevicePixelRatio())
      .Height()
      .ToUnsigned();
  }

  bool HTMLImageElement::Complete() const noexcept
  {
    return _imageLoader->ImageComplete();
  }

  dom::USVString HTMLImageElement::CurrentSrc() const noexcept
  {
    // FIXME: The current URL as a string (CurrentURL()), once USVString holds one.
    return {};
  }

  dom::DOMString HTMLImageElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): ReferrerPolicy() is limited to known values.
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLImageElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Decoding() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Decoding() is limited to known values.
    return Reflection::Reflect<dom::DOMString>(*this, u8"decoding").value_or(u8"");
  }

  void HTMLImageElement::Decoding(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"decoding", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Loading() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Loading() is limited to known values.
    return Reflection::Reflect<dom::DOMString>(*this, u8"loading").value_or(u8"");
  }

  void HTMLImageElement::Loading(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"loading", krys::move(value));
  }

  dom::DOMString HTMLImageElement::FetchPriority() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): FetchPriority() is limited to known values.
    return Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLImageElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLImageElement Obsolete members

  dom::DOMString HTMLImageElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLImageElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLImageElement::LowSrc() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"lowsrc");
  }

  dom::ExceptionOr<void> HTMLImageElement::LowSrc(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"lowsrc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLImageElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  uint32 HTMLImageElement::HSpace() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLImageElement::HSpace(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  uint32 HTMLImageElement::VSpace() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLImageElement::VSpace(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLImageElement::LongDesc() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"longdesc");
  }

  dom::ExceptionOr<void> HTMLImageElement::LongDesc(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"longdesc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Border() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLImageElement::Border(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion

  bool HTMLImageElement::IsReplaced(const RenderStyle *style) const noexcept
  {
    return !style || !style->HasContent();
  }

#pragma region Loading

  Maybe<dom::DOMString> HTMLImageElement::ImageSourceURL() const noexcept
  {
    if (_bestFitImageURL.empty())
    {
      return GetAttribute(u8"src");
    }
    return _bestFitImageURL;
  }

  Maybe<dom::DOMString> HTMLImageElement::AltText() const noexcept
  {
    // lets figure out the alt text.. magic stuff
    // http://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#altgen
    // also heavily discussed by Hixie on bugzilla
    if (auto alt = GetAttribute(u8"alt"))
    {
      return alt;
    }
    // fall back to title attribute
    return GetAttribute(u8"title");
  }

  CachedImage *HTMLImageElement::GetCachedImage() const noexcept
  {
    return _imageLoader->Image();
  }

  Image *HTMLImageElement::GetImage() const noexcept
  {
    if (auto *cachedImage = GetCachedImage())
    {
      return cachedImage->GetImage();
    }
    return nullptr;
  }

  ImageLoader &HTMLImageElement::GetImageLoader() const noexcept
  {
    return *_imageLoader;
  }

  HTMLPictureElement *HTMLImageElement::PictureElement() const noexcept
  {
    return _pictureElement.get();
  }

  void HTMLImageElement::SetPictureElement(HTMLPictureElement *pictureElement) noexcept
  {
    _pictureElement =
      pictureElement ? WeakPtr<HTMLPictureElement>::WithRef(*pictureElement) : WeakPtr<HTMLPictureElement>();
  }

  HTMLSourceElement *HTMLImageElement::SourceElement() const noexcept
  {
    return _sourceElement.get();
  }

  void HTMLImageElement::SetSourceElement(HTMLSourceElement *sourceElement) noexcept
  {
    if (_sourceElement.get() == sourceElement)
    {
      return;
    }
    _sourceElement =
      sourceElement ? WeakPtr<HTMLSourceElement>::WithRef(*sourceElement) : WeakPtr<HTMLSourceElement>();
    InvalidateAttributeMapping();
  }

  void HTMLImageElement::InvalidateAttributeMapping() noexcept
  {
    InvalidatePresentationalHintStyle();
    InvalidateStyle();
  }

  bool HTMLImageElement::UsesSrcsetOrPicture() const noexcept
  {
    return HasAttribute(u8"srcset") || PictureElement();
  }

  float HTMLImageElement::EffectiveImageDevicePixelRatio() const noexcept
  {
    if (!_imageLoader->Image())
    {
      return 1.0f;
    }

    // FIXME: SVG images draw at 1x, once there are SVG images.
    return _imageDevicePixelRatio;
  }

  void HTMLImageElement::SetBestFitURLAndDPRFromImageCandidate(const ImageCandidate &candidate) noexcept
  {
    _bestFitImageURL = candidate.String;
    auto source = ImageSourceURL();
    _currentURL = source ? NodeDocument().CompleteURL(*source) : url::URL();
    if (candidate.Density >= 0)
    {
      _imageDevicePixelRatio = 1 / candidate.Density;
    }
    if (auto *renderImage = DynamicDowncast<RenderImage>(Renderer()))
    {
      renderImage->SetImageDevicePixelRatio(_imageDevicePixelRatio);
    }
  }

  ImageCandidate HTMLImageElement::BestFitSourceFromPictureElement() noexcept
  {
    auto *picture = PictureElement();
    if (!picture)
    {
      return {};
    }

    ImageCandidate candidate;

    for (auto *child = picture->FirstChild(); child && child != this; child = child->NextSibling())
    {
      auto *source = DynamicDowncast<HTMLSourceElement>(*child);
      if (!source)
      {
        continue;
      }

      auto srcset = source->GetAttribute(u8"srcset");
      if (!srcset || srcset->empty())
      {
        continue;
      }

      if (auto typeAttribute = source->GetAttribute(u8"type"))
      {
        auto type = ExtractMIMETypeFromTypeAttributeForLookup(*typeAttribute);
        if (!type.empty() && !MIMETypeRegistry::IsSupportedImageVideoOrSVGMIMEType(type))
        {
          continue;
        }
      }

      auto &document = NodeDocument();
      auto documentElement = document.DocumentElement();
      css::mq::MediaQueryEvaluator evaluator(document.Printing() ? PrintAtom() : ScreenAtom(), document,
                                             documentElement ? documentElement->ComputedStyle() : nullptr);
      auto &queries = source->ParsedMediaAttribute(document);

      // FIXME: Remember the queries that depend on the viewport, to select again when they change.
      if (!evaluator.Evaluate(queries))
      {
        continue;
      }

      css::SizesAttributeParser sizesParser(source->GetAttribute(u8"sizes").value_or(dom::DOMString()),
                                            document);
      auto sourceSize = sizesParser.Length();

      candidate = BestFitSourceForImageAttributes(document.DeviceScaleFactor(), null,
                                                  dom::DOMStringView(*srcset), sourceSize);

      if (!candidate.IsEmpty())
      {
        SetSourceElement(source);
        break;
      }
    }

    return candidate;
  }

  void HTMLImageElement::SelectImageSource(RelevantMutation relevantMutation) noexcept
  {
    // First look for the best fit source from our <picture> parent if we have one.
    ImageCandidate candidate = BestFitSourceFromPictureElement();
    if (candidate.IsEmpty())
    {
      SetSourceElement(nullptr);
      auto srcAttribute = GetAttribute(u8"src");
      auto srcsetAttribute = GetAttribute(u8"srcset");
      // This is extremely common case. We should not invoke SizesAttributeParser at all.
      if (!srcsetAttribute)
      {
        if (srcAttribute)
        {
          candidate =
            ImageCandidate(*srcAttribute, DescriptorParsingResult(), ImageCandidate::OriginAttribute::Src);
        }
      }
      else
      {
        // If we don't have a <picture> or didn't find a source, then we use our own attributes.
        auto &document = NodeDocument();
        css::SizesAttributeParser sizesParser(AttributeOrEmpty(u8"sizes"), document);
        auto sourceSize = sizesParser.Length();
        candidate = BestFitSourceForImageAttributes(
          document.DeviceScaleFactor(), srcAttribute ? Maybe<dom::DOMStringView>(*srcAttribute) : null,
          dom::DOMStringView(*srcsetAttribute), sourceSize);
      }
    }
    SetBestFitURLAndDPRFromImageCandidate(candidate);
    _imageLoader->UpdateFromElementIgnoringPreviousError(relevantMutation);
  }

  void HTMLImageElement::UpdatePictureElement() noexcept
  {
    auto *parentPicture = DynamicDowncast<HTMLPictureElement>(ParentElement());
    if (parentPicture == PictureElement())
    {
      return;
    }
    SetPictureElement(parentPicture);
    SelectImageSource(RelevantMutation::Yes);
  }

  void HTMLImageElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value,
                                            dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"alt")
    {
      if (auto *renderImage = DynamicDowncast<RenderImage>(Renderer()))
      {
        renderImage->UpdateAltText();
      }
    }
    else if (localName == u8"src" || localName == u8"srcset" || localName == u8"sizes")
    {
      if (oldValue != value)
      {
        SelectImageSource(RelevantMutation::Yes);
      }
      else
      {
        _imageLoader->UpdateFromElementIgnoringPreviousErrorToSameValue();
      }
    }
    // FIXME: usemap, loading, referrerpolicy and crossorigin, once there are image maps, lazy loading and
    // network loads.
  }

  void HTMLImageElement::OnInsert() noexcept
  {
    // Insert needs to complete first, before we start updating the loader. Loader dispatches events which
    // could result in callbacks back to this node.
    HTMLElement::OnInsert();

    if (DynamicDowncast<HTMLPictureElement>(ParentElement()) != PictureElement())
    {
      UpdatePictureElement();
      return;
    }

    // If we have been inserted from a renderer-less document,
    // our loader may have not fetched the image, so do it now.
    if (IsConnected() && !_imageLoader->Image())
    {
      _imageLoader->UpdateFromElement();
    }
  }

  void HTMLImageElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    if (isSubtreeRoot && Is<HTMLPictureElement>(oldAncestor))
    {
      krys_debug_assert(PictureElement() == &oldAncestor);
      SetPictureElement(nullptr);
      SelectImageSource(RelevantMutation::Yes);
    }

    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  void HTMLImageElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    UpdatePictureElement();
  }

  void HTMLImageElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);
    SelectImageSource(RelevantMutation::No);
    _imageLoader->ElementDidMoveToNewDocument(oldDocument);
    if (auto *picture = PictureElement())
    {
      picture->SourcesChanged();
    }
  }

  RenderPtr<RenderElement> HTMLImageElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (style.HasContent())
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderImage>(RenderObject::Type::Image, *this, krys::move(style), nullptr,
                                       _imageDevicePixelRatio);
  }

  void HTMLImageElement::DidAttachRenderers() noexcept
  {
    auto *renderImage = DynamicDowncast<RenderImage>(Renderer());
    if (!renderImage)
    {
      return;
    }

    auto &renderImageResource = renderImage->ImageResource();
    if (renderImageResource.GetCachedImage())
    {
      return;
    }
    renderImageResource.SetCachedImage(CachedResourceHandle<CachedImage>(_imageLoader->Image()));

    // If we have no image at all because we have no src attribute, set
    // image height and width for the alt text instead.
    if (!_imageLoader->Image() && !renderImageResource.GetCachedImage())
    {
      (void)renderImage->SetImageSizeForAltText();
    }
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLImageElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"width" || localName == u8"height" || localName == u8"border"
            || localName == u8"vspace" || localName == u8"hspace" || localName == u8"valign"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLImageElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      AddHTMLMultiLengthToStyle(style, css::PropertyId::Width, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(value, AttributeOrEmpty(u8"height"), style);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLMultiLengthToStyle(style, css::PropertyId::Height, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"), value, style);
      return;
    }
    if (localName == u8"border")
    {
      ApplyBorderAttributeToStyle(value, style);
      return;
    }
    if (localName == u8"vspace")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      return;
    }
    if (localName == u8"hspace")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      return;
    }
    if (localName == u8"align")
    {
      ApplyAlignmentAttributeToStyle(value, style);
      return;
    }
    if (localName == u8"valign")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, ToCSSOMString(value));
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  void HTMLImageElement::CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept
  {
    auto *sourceElement = SourceElement();
    if (!sourceElement)
    {
      return;
    }
    auto widthAttrFromSource = sourceElement->GetAttribute(u8"width");
    auto heightAttrFromSource = sourceElement->GetAttribute(u8"height");
    // If both width and height attributes of <source> is undefined, the style's value should not
    // be overwritten. Otherwise, <souce> will overwrite it. I.e., if <source> only has one attribute
    // defined, the other one and aspect-ratio shouldn't be set to auto.
    if (!widthAttrFromSource && !heightAttrFromSource)
    {
      return;
    }

    if (widthAttrFromSource)
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, *widthAttrFromSource);
    }
    else
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Width, css::ValueId::Auto);
    }

    if (heightAttrFromSource)
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, *heightAttrFromSource);
    }
    else
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Height, css::ValueId::Auto);
    }

    if (widthAttrFromSource && heightAttrFromSource)
    {
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(*widthAttrFromSource, *heightAttrFromSource, style);
    }
    else
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::AspectRatio, css::ValueId::Auto);
    }
  }

#pragma endregion
}
