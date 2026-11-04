#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSourceElement.hpp"
#include "Krystal.Booey/HTML/Images/HTMLImageLoader.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/SrcsetParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <limits>

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-img-decoding
  template <>
  struct EnumeratedAttributeTraits<"decoding", HTMLImageElement>
  {
    using State = DecodingAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"sync", State::Sync}, {u8"async", State::Async}, {u8"auto", State::Auto}};

    constexpr static State MissingValueDefault = State::Auto;
    constexpr static State InvalidValueDefault = State::Auto;
  };

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
    return Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLImageElement::Alt(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::USVString HTMLImageElement::Src() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLImageElement::Src(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::USVString HTMLImageElement::SrcSet() const noexcept
  {
    return Reflect<dom::USVString>(*this, u8"srcset");
  }

  void HTMLImageElement::SrcSet(dom::USVString value) noexcept
  {
    return Reflect<dom::USVString>(*this, u8"srcset", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Sizes() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLImageElement::Sizes(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLImageElement::CrossOrigin() const noexcept
  {
    return ReflectEnumerated<"crossorigin", HTMLElement, Maybe<dom::DOMString>>(*this);
  }

  void HTMLImageElement::CrossOrigin(Maybe<dom::DOMString> value) noexcept
  {
    Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
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
    dom::DOMStringView value = *usemap;
    auto numberSign = value.find(u8'#');
    if (numberSign == dom::DOMStringView::npos || numberSign == value.size() - 1)
    {
      return nullptr;
    }

    // 2. Let s be the string from the character immediately after the first U+0023 NUMBER SIGN character in
    // the
    //    string being parsed up to the end of that string.
    auto name = value.substr(numberSign + 1);

    // 3. Return the first element of type type in scope's tree, in tree order, that has an id or name
    // attribute
    //    whose value is s, or null if there is no such element.
    auto &root = dom::TreeQueries::Root(const_cast<HTMLImageElement &>(*this));
    for (auto &node : dom::DescendantRange(root))
    {
      auto *map = DynamicDowncast<HTMLMapElement>(node);
      if (!map)
      {
        continue;
      }
      auto id = map->GetAttribute(u8"id");
      auto mapName = map->GetAttribute(u8"name");
      if ((id && *id == name) || (mapName && *mapName == name))
      {
        return map;
      }
    }
    return nullptr;
  }

  dom::DOMString HTMLImageElement::UseMap() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLImageElement::UseMap(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

  bool HTMLImageElement::IsMap() const noexcept
  {
    return Reflect<bool>(*this, u8"ismap");
  }

  void HTMLImageElement::IsMap(bool value) noexcept
  {
    Reflect<bool>(*this, u8"ismap", krys::move(value));
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
    // If the usemap attribute starts with '#', it refers to a map element in the document.
    if (usemap->starts_with(u8'#'))
    {
      return false;
    }

    return NodeDocument().CompleteURL(*usemap).IsEmpty();
  }

  bool HTMLImageElement::Controls() const noexcept
  {
    return Reflect<bool>(*this, u8"controls");
  }

  void HTMLImageElement::Controls(bool value) noexcept
  {
    Reflect<bool>(*this, u8"controls", krys::move(value));
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
    Reflect<uint32>(*this, u8"width", krys::move(value));
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
    Reflect<uint32>(*this, u8"height", krys::move(value));
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
    return dom::USVString(CurrentURL().String());
  }

  dom::DOMString HTMLImageElement::ReferrerPolicy() const noexcept
  {
    return ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLImageElement::ReferrerPolicy(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Decoding() const noexcept
  {
    return ReflectEnumerated<"decoding", HTMLImageElement>(*this);
  }

  void HTMLImageElement::Decoding(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"decoding", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Loading() const noexcept
  {
    return ReflectEnumerated<"loading">(*this);
  }

  void HTMLImageElement::Loading(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"loading", krys::move(value));
  }

  dom::DOMString HTMLImageElement::FetchPriority() const noexcept
  {
    return ReflectEnumerated<"fetchpriority">(*this);
  }

  void HTMLImageElement::FetchPriority(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLImageElement Obsolete members

  dom::DOMString HTMLImageElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLImageElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::USVString HTMLImageElement::LowSrc() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"lowsrc");
  }

  void HTMLImageElement::LowSrc(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"lowsrc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLImageElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  uint32 HTMLImageElement::HSpace() const noexcept
  {
    return Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLImageElement::HSpace(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  uint32 HTMLImageElement::VSpace() const noexcept
  {
    return Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLImageElement::VSpace(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::USVString HTMLImageElement::LongDesc() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"longdesc");
  }

  void HTMLImageElement::LongDesc(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"longdesc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Border() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLImageElement::Border(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
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
    auto *frame = NodeDocument().Frame();
    if (!frame || !frame->GetMediaEnvironment().CanExecuteScripts)
    {
      return false;
    }

    // 2. If element's lazy loading attribute is in the Lazy state, then return true.
    // 3. Return false.
    return HasLazyLoadableAttributeValue(GetAttribute(u8"loading"));
  }

  bool HTMLImageElement::HasLazyLoadableAttributeValue(Maybe<dom::DOMStringView> attributeValue) noexcept
  {
    return attributeValue && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*attributeValue, u8"lazy");
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

    RefPtr<Image> image = ShareRefPtr(_imageLoader->Image()->GetImage());
    if (image && image->DrawsSVGImage())
    {
      return 1.0f;
    }

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

  // Steps 3 and 5b to 5l of updating the source set, for the picture's source elements:
  // https://html.spec.whatwg.org/#update-the-source-set
  ImageCandidate HTMLImageElement::BestFitSourceFromPictureElement() noexcept
  {
    // 3. If el is an img element whose parent node is a picture element, then replace the contents of
    // elements
    //    with el's parent node's child elements, retaining relative order.
    auto *picture = PictureElement();
    if (!picture)
    {
      return {};
    }

    ImageCandidate candidate;

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
        auto type = ExtractMIMETypeFromTypeAttributeForLookup(*typeAttribute);
        if (!type.empty() && !MIMETypeRegistry::IsSupportedImageVideoOrSVGMIMEType(type))
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
      // next
      //     child.
      // FIXME: Remember the queries that depend on the viewport, to select again when they change.
      if (!evaluator.Evaluate(queries))
      {
        continue;
      }

      // 5g. Parse child's sizes attribute with img, and let source set's source size be the returned value.
      css::SizesAttributeParser sizesParser(source->GetAttribute(u8"sizes").value_or(dom::DOMString()),
                                            document);
      auto sourceSize = sizesParser.Length();

      // 5d. Parse child's srcset attribute and let source set be the returned source set.
      // 5j. Normalize the source densities of source set.
      // (And select an image from it.)
      candidate = BestFitSourceForImageAttributes(document.DeviceScaleFactor(), null,
                                                  dom::DOMStringView(*srcset), sourceSize);

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
    // First look for the best fit source from our <picture> parent if we have one.
    ImageCandidate candidate = BestFitSourceFromPictureElement();

    // 5a. If child is el: (Updating the source set, from the img's own attributes.)
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
    // 2. If el's source set is empty, return null as the URL and undefined as the pixel density.
    // 3. Return the result of selecting an image from el's source set.
    SetBestFitURLAndDPRFromImageCandidate(candidate);

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
    else if (localName == u8"loading")
    {
      // No action needed for eager to lazy transition.
      if (!HasLazyLoadableAttributeValue(value))
      {
        LoadDeferredImage();
      }
    }
    // FIXME: referrerpolicy and crossorigin, once there are network loads.
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-insertion-steps
  void HTMLImageElement::OnInsert() noexcept
  {
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

    // If we have been inserted from a renderer-less document,
    // our loader may have not fetched the image, so do it now.
    if (IsConnected() && !_imageLoader->Image())
    {
      _imageLoader->UpdateFromElement();
    }
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-removing-steps
  void HTMLImageElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
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

    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  // https://html.spec.whatwg.org/#the-img-element:html-element-moving-steps
  void HTMLImageElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // 1. If isSubtreeRoot is true and oldAncestor is a picture element, then count this as a relevant
    //    mutation for movedNode.
    // (And moving into a picture, as inserting does.)
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
