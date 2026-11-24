#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSourceElement.hpp"
#include "Krystal.Booey/HTML/Images/HTMLImageLoader.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/SrcsetParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Image/ImageDecoder.hpp"
#include <limits>

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#attr-img-decoding
  template <>
  struct html::EnumeratedAttributeTraits<"decoding", HTMLImageElement>
  {
    using State = html::DecodingAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
      {u8"sync", State::Sync}, {u8"async", State::Async}, {u8"auto", State::Auto}};

    constexpr static State MissingValueDefault = State::Auto;
    constexpr static State InvalidValueDefault = State::Auto;
  };

  namespace
  {
    KRYS_NODISCARD DOMStringView
      ExtractMimeTypeFromTypeAttributeForLookup(DOMStringView typeAttribute) noexcept
    {
      auto semicolonIndex = typeAttribute.find(u8';');
      if (semicolonIndex == DOMStringView::npos)
      {
        return infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(typeAttribute);
      }
      return infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(
        typeAttribute.substr(0, semicolonIndex));
    }

    KRYS_NODISCARD uint32 ClampedAttributeLength(uint64 value) noexcept
    {
      return static_cast<uint32>(std::min<uint64>(value, std::numeric_limits<int32>::max()));
    }
  }

  HTMLImageElement::HTMLImageElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Image),
        _imageLoader(CreateUnique<html::HTMLImageLoader>(*this))
  {
  }

  HTMLImageElement::~HTMLImageElement() noexcept
  {
    SetForm(nullptr);
  }

  bool HTMLImageElement::CanStartSelection() const noexcept
  {
    if (ShadowRoot())
    {
      return HTMLElement::CanStartSelection();
    }
    return false;
  }

  void HTMLImageElement::ResetFormOwner() noexcept
  {
    SetForm(HTMLFormElement::FindClosestFormAncestor(*this));
  }

  void HTMLImageElement::SetFormInternal(HTMLFormElement *newForm) noexcept
  {
    if (auto *form = Form())
    {
      form->UnregisterImageElement(*this);
    }
    html::FormAssociatedElement::SetFormInternal(newForm);
    if (newForm)
    {
      newForm->RegisterImageElement(*this);
    }
  }

  void HTMLImageElement::FormOwnerRemovedFromTree(const Node &formRoot) noexcept
  {
    if (&dom::TreeQueries::Root(*this) != &formRoot)
    {
      SetForm(nullptr);
    }
  }

#pragma region HTMLImageElement

  DOMString HTMLImageElement::Alt() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLImageElement::Alt(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::USVString HTMLImageElement::Src() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src");
  }

  void HTMLImageElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::USVString HTMLImageElement::SrcSet() const noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"srcset");
  }

  void HTMLImageElement::SrcSet(dom::USVString value) noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  DOMString HTMLImageElement::Sizes() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLImageElement::Sizes(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"sizes", krys::move(value));
  }

  Maybe<DOMString> HTMLImageElement::CrossOrigin() const noexcept
  {
    return html::ReflectEnumerated<"crossorigin", HTMLElement, Maybe<DOMString>>(*this);
  }

  void HTMLImageElement::CrossOrigin(Maybe<DOMString> value) noexcept
  {
    html::Reflect<Maybe<DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  HTMLMapElement *HTMLImageElement::AssociatedMapElement() const noexcept
  {
    // The rules for parsing a hash-name reference to a map element, given the image:
    // https://html.spec.whatwg.org/#rules-for-parsing-a-hash-name-reference
    auto usemap = GetAttribute(u8"usemap");
    if (!usemap)
    {
      return nullptr;
    }

    // 1. If the string being parsed does not contain a U+0023 NUMBER SIGN character, or if the first such
    //    character in the string is the last character in the string, then return null.
    DOMStringView value = *usemap;
    auto numberSign = value.find(u8'#');
    if (numberSign == DOMStringView::npos || numberSign == value.size() - 1)
    {
      return nullptr;
    }

    // 2. Let s be the string from the character immediately after the first U+0023 NUMBER SIGN character in
    //    the string being parsed up to the end of that string.
    auto name = value.substr(numberSign + 1);

    // 3. Return the first element of type type in scope's tree, in tree order, that has an id or name
    //    attribute whose value is s, or null if there is no such element.
    // The tree scope maps them; a tree outside one has to look.
    if (IsConnected() || IsInShadowTree())
    {
      return TreeScope().FirstImageMap(name);
    }

    auto &root = dom::TreeQueries::Root(const_cast<HTMLImageElement &>(*this));
    for (auto &node : dom::DescendantRange(root))
    {
      auto *map = DynamicDowncast<HTMLMapElement>(node);
      if (!map)
      {
        continue;
      }
      if (map->Id() == name || map->Name() == name)
      {
        return map;
      }
    }
    return nullptr;
  }

  DOMString HTMLImageElement::UseMap() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLImageElement::UseMap(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"usemap", krys::move(value));
  }

  bool HTMLImageElement::IsMap() const noexcept
  {
    return html::Reflect<bool>(*this, u8"ismap");
  }

  void HTMLImageElement::IsMap(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"ismap", krys::move(value));
  }

  bool HTMLImageElement::IsServerMap() const noexcept
  {
    if (!HasAttribute(u8"ismap"))
    {
      return false;
    }

    auto usemap = GetAttribute(u8"usemap");
    if (!usemap)
    {
      return true;
    }
    // A usemap of the form #name refers to a map in the document.
    if (usemap->starts_with(u8'#'))
    {
      return false;
    }

    return NodeDocument().CompleteUrl(*usemap).IsEmpty();
  }

  bool HTMLImageElement::Controls() const noexcept
  {
    return html::Reflect<bool>(*this, u8"controls");
  }

  void HTMLImageElement::Controls(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"controls", krys::move(value));
  }

  uint32 HTMLImageElement::Width() const noexcept
  {
    if (IsConnected())
    {
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(
        {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
         dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
        this);
    }

    if (!Renderer())
    {
      // Without a renderer, the width attribute, or else the image's own width.
      if (auto width = html::Numbers::ParseNonNegativeInteger(AttributeOrEmpty(u8"width")); width.Success())
      {
        return ClampedAttributeLength(width.Value);
      }

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
    html::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLImageElement::Height() const noexcept
  {
    if (IsConnected())
    {
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(
        {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
         dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
        this);
    }

    if (!Renderer())
    {
      // Without a renderer, the height attribute, or else the image's own height.
      if (auto height = html::Numbers::ParseNonNegativeInteger(AttributeOrEmpty(u8"height"));
          height.Success())
      {
        return ClampedAttributeLength(height.Value);
      }

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
    html::Reflect<uint32>(*this, u8"height", krys::move(value));
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
    // The currentSrc IDL attribute must return the img element's current request's current URL.
    return dom::USVString(CurrentUrl().String());
  }

  DOMString HTMLImageElement::ReferrerPolicy() const noexcept
  {
    return html::ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLImageElement::ReferrerPolicy(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  DOMString HTMLImageElement::Decoding() const noexcept
  {
    return html::ReflectEnumerated<"decoding", HTMLImageElement>(*this);
  }

  void HTMLImageElement::Decoding(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"decoding", krys::move(value));
  }

  DecodingMode HTMLImageElement::GetDecodingMode() const noexcept
  {
    auto decoding = Decoding();
    if (decoding == u8"sync")
    {
      return DecodingMode::Synchronous;
    }
    if (decoding == u8"async")
    {
      return DecodingMode::Asynchronous;
    }
    return DecodingMode::Auto;
  }

  DOMString HTMLImageElement::Loading() const noexcept
  {
    return html::ReflectEnumerated<"loading">(*this);
  }

  void HTMLImageElement::Loading(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"loading", krys::move(value));
  }

  DOMString HTMLImageElement::FetchPriority() const noexcept
  {
    return html::ReflectEnumerated<"fetchpriority">(*this);
  }

  void HTMLImageElement::FetchPriority(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

  void HTMLImageElement::Decode(ImageLoader::DecodePromise promise) noexcept
  {
    GetImageLoader().Decode(krys::move(promise));
  }

#pragma endregion

#pragma region HTMLImageElement Obsolete members

  DOMString HTMLImageElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLImageElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  dom::USVString HTMLImageElement::LowSrc() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"lowsrc");
  }

  void HTMLImageElement::LowSrc(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"lowsrc", krys::move(value));
  }

  DOMString HTMLImageElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLImageElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  uint32 HTMLImageElement::HSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLImageElement::HSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  uint32 HTMLImageElement::VSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLImageElement::VSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::USVString HTMLImageElement::LongDesc() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"longdesc");
  }

  void HTMLImageElement::LongDesc(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"longdesc", krys::move(value));
  }

  DOMString HTMLImageElement::Border() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLImageElement::Border(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion

  bool HTMLImageElement::IsReplaced(const RenderStyle *style) const noexcept
  {
    return !style || !style->HasContent();
  }

#pragma region Loading

  Maybe<DOMString> HTMLImageElement::ImageSourceUrl() const noexcept
  {
    if (_bestFitImageUrl.empty())
    {
      return GetAttribute(u8"src");
    }
    return _bestFitImageUrl;
  }

  Maybe<DOMString> HTMLImageElement::AltText() const noexcept
  {
    // The alt attribute, or else the title.
    if (auto alt = GetAttribute(u8"alt"))
    {
      return alt;
    }
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

  void HTMLImageElement::LoadDeferredImage() noexcept
  {
    _imageLoader->LoadDeferredImage();
  }

  bool HTMLImageElement::IsDeferred() const noexcept
  {
    return _imageLoader->IsDeferred();
  }

  bool HTMLImageElement::IsLazyLoadable() const noexcept
  {
    // 1. If scripting is disabled for element, then return false.
    //    (Scripting is enabled for any element in a frame: the application plays the part of scripts.)
    if (!NodeDocument().Frame())
    {
      return false;
    }

    // 2. If element's lazy loading attribute is in the Lazy state, then return true.
    // 3. Return false.
    return HasLazyLoadableAttributeValue(GetAttribute(u8"loading"));
  }

  bool HTMLImageElement::HasLazyLoadableAttributeValue(Maybe<DOMStringView> attributeValue) noexcept
  {
    return attributeValue && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*attributeValue, u8"lazy");
  }

  HTMLPictureElement *HTMLImageElement::PictureElement() const noexcept
  {
    return _pictureElement.get();
  }

  void HTMLImageElement::SetPictureElement(HTMLPictureElement *pictureElement) noexcept
  {
    _pictureElement = CreateWeakPtr(pictureElement);
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
    _sourceElement = CreateWeakPtr(sourceElement);
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

    RefPtr<Image> image = ShareRefPtr(_imageLoader->Image()->GetImage());
    if (image && image->DrawsSVGImage())
    {
      return 1.0f;
    }

    return _imageDevicePixelRatio;
  }

  void HTMLImageElement::SetBestFitUrlAndDPRFromImageCandidate(const html::ImageCandidate &candidate) noexcept
  {
    _bestFitImageUrl = candidate.String;
    auto source = ImageSourceUrl();
    _currentUrl = source ? NodeDocument().CompleteUrl(*source) : net::Url();
    if (candidate.Density >= 0)
    {
      _imageDevicePixelRatio = 1 / candidate.Density;
    }
    if (auto *renderImage = DynamicDowncast<RenderImage>(Renderer()))
    {
      renderImage->SetImageDevicePixelRatio(_imageDevicePixelRatio);
    }
  }

  // Steps 3 and 5b to 5l of updating the source set, for the picture's source elements:
  // https://html.spec.whatwg.org/#update-the-source-set
  html::ImageCandidate HTMLImageElement::BestFitSourceFromPictureElement() noexcept
  {
    // 3. If el is an img element whose parent node is a picture element, then replace the contents of
    //    elements with el's parent node's child elements, retaining relative order.
    auto *picture = PictureElement();
    if (!picture)
    {
      return {};
    }

    html::ImageCandidate candidate;

    // 5. For each child in elements:
    // 5a. If child is el: (SelectImageSource, with the img's own attributes.)
    for (auto *child = picture->FirstChild(); child && child != this; child = child->NextSibling())
    {
      // 5b. If child is not a source element, then continue.
      auto *source = DynamicDowncast<HTMLSourceElement>(*child);
      if (!source)
      {
        continue;
      }

      // 5c. If child does not have a srcset attribute, continue to the next child.
      // 5e. If source set has zero image sources, continue to the next child.
      // (An empty srcset has none; another without any is skipped below.)
      auto srcset = source->GetAttribute(u8"srcset");
      if (!srcset || srcset->empty())
      {
        continue;
      }

      // 5h. If child has a type attribute, and its value is an unknown or unsupported MIME type, continue to
      //     the next child.
      if (auto typeAttribute = source->GetAttribute(u8"type"))
      {
        auto type = ExtractMimeTypeFromTypeAttributeForLookup(*typeAttribute);
        if (!type.empty() && !img::ImageDecoder::SupportsMimeType(type)
            && !text::IsASCIICaselessEqual(type, u8"image/svg+xml"))
        {
          continue;
        }
      }

      auto &document = NodeDocument();
      auto documentElement = document.DocumentElement();
      css::mq::MediaQueryEvaluator evaluator(
        document.Printing() ? css::mq::PrintAtom() : css::mq::ScreenAtom(), document,
        documentElement ? documentElement->ComputedStyle() : nullptr);
      auto &queries = source->ParsedMediaAttribute(document);

      // 5f. If child has a media attribute, and its value does not match the environment, continue to the
      //     next child.
      // FIXME: Remember the queries that depend on the viewport, to select again when they change.
      if (!evaluator.Evaluate(queries))
      {
        continue;
      }

      // 5g. Parse child's sizes attribute with img, and let source set's source size be the returned value.
      css::SizesAttributeParser sizesParser(source->GetAttribute(u8"sizes").value_or(DOMString()), document);
      auto sourceSize = sizesParser.Length();

      // 5d. Parse child's srcset attribute and let source set be the returned source set.
      // 5j. Normalize the source densities of source set.
      // (And select an image from it.)
      candidate = html::BestFitSourceForImageAttributes(document.DeviceScaleFactor(), null,
                                                        DOMStringView(*srcset), sourceSize);

      // 5i. If child has width or height attributes, set el's dimension attribute source to child. Otherwise,
      //     set el's dimension attribute source to el.
      // (The source element is the dimension attribute source when it has them.)
      // 5k. Set el's source set to source set.
      // 5l. Return.
      if (!candidate.IsEmpty())
      {
        SetSourceElement(source);
        break;
      }
    }

    return candidate;
  }

  // https://html.spec.whatwg.org/#select-an-image-source
  void HTMLImageElement::SelectImageSource(RelevantMutation relevantMutation) noexcept
  {
    // 1. Update the source set for el.
    // (A picture parent's sources first.)
    html::ImageCandidate candidate = BestFitSourceFromPictureElement();

    // 5a. If child is el: (Updating the source set, from the img's own attributes.)
    if (candidate.IsEmpty())
    {
      SetSourceElement(nullptr);
      auto srcAttribute = GetAttribute(u8"src");
      auto srcsetAttribute = GetAttribute(u8"srcset");
      // Without srcset (the common case), src is the only candidate and sizes isn't parsed.
      if (!srcsetAttribute)
      {
        if (srcAttribute)
        {
          candidate = html::ImageCandidate(*srcAttribute, html::DescriptorParsingResult(),
                                           html::ImageCandidate::OriginAttribute::Src);
        }
      }
      else
      {
        auto &document = NodeDocument();
        css::SizesAttributeParser sizesParser(AttributeOrEmpty(u8"sizes"), document);
        auto sourceSize = sizesParser.Length();
        candidate = html::BestFitSourceForImageAttributes(
          document.DeviceScaleFactor(), srcAttribute ? Maybe<DOMStringView>(*srcAttribute) : null,
          DOMStringView(*srcsetAttribute), sourceSize);
      }
    }
    // 2. If el's source set is empty, return null as the URL and undefined as the pixel density.
    // 3. Return the result of selecting an image from el's source set.
    SetBestFitUrlAndDPRFromImageCandidate(candidate);

    // Then update the image data.
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

  void HTMLImageElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                            Maybe<DOMStringView> value,
                                            const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"alt")
    {
      if (auto *renderImage = DynamicDowncast<RenderImage>(Renderer()))
      {
        renderImage->UpdateForAltText();
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
    else if (localName == u8"loading")
    {
      // Becoming eager loads a deferred image; becoming lazy changes nothing.
      if (!HasLazyLoadableAttributeValue(value))
      {
        LoadDeferredImage();
      }
    }
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-insertion-steps
  void HTMLImageElement::OnInsert() noexcept
  {
    html::FormAssociatedElement::ElementInsertedIntoAncestor(*this, IsConnected());
    if (!Form())
    {
      ResetFormOwner();
    }

    // Insert needs to complete first, before we start updating the loader. Loader dispatches events which
    // could result in callbacks back to this node.
    HTMLElement::OnInsert();

    // 1. If insertedNode's parent is a picture element, then, count this as a relevant mutation for
    //    insertedNode.
    if (DynamicDowncast<HTMLPictureElement>(ParentElement()) != PictureElement())
    {
      UpdatePictureElement();
      return;
    }

    // The image may not have been fetched while the element was outside a document.
    if (IsConnected() && !_imageLoader->Image())
    {
      _imageLoader->UpdateFromElement();
    }
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-removing-steps
  void HTMLImageElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    // 1. If isSubtreeRoot is true and oldAncestor is a picture element, then count this as a relevant
    //    mutation for removedNode.
    // 2. Set removedNode's last auto-sizes width to null.
    // (There are no auto sizes.)
    if (isSubtreeRoot && Is<HTMLPictureElement>(oldAncestor))
    {
      krys_debug_assert(PictureElement() == &oldAncestor);
      SetPictureElement(nullptr);
      SelectImageSource(RelevantMutation::Yes);
    }

    html::FormAssociatedElement::ElementRemovedFromAncestor(*this, !IsConnected());

    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-moving-steps
  void HTMLImageElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    ResetFormOwner();

    // 1. If isSubtreeRoot is true and oldAncestor is a picture element, then count this as a relevant
    //    mutation for movedNode.
    // (And moving into a picture, as inserting does.)
    UpdatePictureElement();
  }

  void HTMLImageElement::OnAdopt(Document &oldDocument) noexcept
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

    // Without an image (no src), the renderer is sized for the alt text.
    if (!_imageLoader->Image() && !renderImageResource.GetCachedImage())
    {
      (void)renderImage->SetImageSizeForAltText();
    }
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLImageElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"height" || localName == u8"border"
            || localName == u8"vspace" || localName == u8"hspace" || localName == u8"valign"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLImageElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                                const DOMStringAtom &namespaceURI,
                                                                DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
    // A source with neither width nor height leaves the img's dimensions; with either, the source's replace
    // them both, and the aspect ratio.
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
