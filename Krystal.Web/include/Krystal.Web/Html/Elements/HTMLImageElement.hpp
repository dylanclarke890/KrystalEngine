#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Forms/FormAssociatedElement.hpp"
#include "Krystal.Web/Loader/ImageLoader.hpp"

namespace krys::web::html
{
  class HTMLImageLoader;
  struct ImageCandidate;
}

namespace krys::web
{
  class CachedImage;
  class HTMLMapElement;
  class HTMLPictureElement;
  class HTMLSourceElement;
  class Image;

  /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#htmlimageelement
  // FIXME: decode(), the legacy Image()
  // constructor, and re-selecting the source when the media queries it depends on change.
  class HTMLImageElement : public HTMLElement, public html::FormAssociatedElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLImageElement);
    friend class HTMLPictureElement;

  private:
    UniquePtr<html::HTMLImageLoader> _imageLoader;
    DOMString _bestFitImageUrl;
    net::Url _currentUrl;
    float _imageDevicePixelRatio {1.0f};
    WeakPtr<HTMLPictureElement> _pictureElement;
    // The picture's source element the current image was selected from, if any.
    WeakPtr<HTMLSourceElement> _sourceElement;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool CanStartSelection() const noexcept override;

    HTMLImageElement(Document &document) noexcept;

    ~HTMLImageElement() noexcept override;

    /// @brief An image is interactive content when it has a `usemap` attribute.
    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return HasAttribute(u8"usemap");
    }

    /// @brief The image map `usemap` names: the first `map` in the image's tree whose `id` or `name` is the
    /// name after its `#`.
    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#image-map
    KRYS_NODISCARD HTMLMapElement *AssociatedMapElement() const noexcept;

#pragma region HTMLImageElement - https://html.spec.whatwg.org/multipage/embedded-content.html#htmlimageelement

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-alt
    void Alt(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-src
    KRYS_NODISCARD USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-src
    void Src(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-srcset
    KRYS_NODISCARD USVString SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-srcset
    void SrcSet(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-sizes
    void Sizes(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-crossorigin
    void CrossOrigin(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-usemap
    void UseMap(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-ismap
    KRYS_NODISCARD bool IsMap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-ismap
    void IsMap(bool value) noexcept;

    /// @brief Whether the image is a server-side image map: it has `ismap` and no client-side map
    /// (`usemap`), so a link around it is followed with the coordinates clicked.
    KRYS_NODISCARD bool IsServerMap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-controls
    KRYS_NODISCARD bool Controls() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-controls
    void Controls(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-naturalwidth
    KRYS_NODISCARD uint32 NaturalWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-naturalheight
    KRYS_NODISCARD uint32 NaturalHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-complete
    KRYS_NODISCARD bool Complete() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-currentsrc
    KRYS_NODISCARD USVString CurrentSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-decoding
    KRYS_NODISCARD DOMString Decoding() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-decoding
    void Decoding(DOMString value) noexcept;

    /// @brief The decoding hint `decoding` gives: synchronous, asynchronous, or the engine's choice.
    /// @see https://html.spec.whatwg.org/multipage/images.html#image-decoding-hint
    KRYS_NODISCARD DecodingMode GetDecodingMode() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-loading
    KRYS_NODISCARD DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-loading
    void Loading(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-fetchpriority
    void FetchPriority(DOMString value) noexcept;

    /// @brief Calls @p promise, as a microtask, once the image has loaded and decoded, so it can be drawn
    /// without decoding; it's rejected with an `EncodingError` if it can't be.
    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-decode
    void Decode(ImageLoader::DecodePromise promise) noexcept;

#pragma endregion

#pragma region HTMLImageElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLImageElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-lowsrc
    KRYS_NODISCARD USVString LowSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-lowsrc
    void LowSrc(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-longdesc
    KRYS_NODISCARD USVString LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-longdesc
    void LongDesc(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-img-border
    void Border(DOMString value) noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsReplaced(const RenderStyle *style = nullptr) const noexcept override;

    /// @brief The URL of the selected source, resolved against the document.
    KRYS_NODISCARD const net::Url &CurrentUrl() const noexcept
    {
      return _currentUrl;
    }

    /// @brief The `alt` attribute, or else the `title` attribute, or null if it has neither.
    KRYS_NODISCARD Maybe<DOMString> AltText() const noexcept;

    /// @brief The image the element shows, as loaded, as decoded, and the loader that loads it.
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;

    KRYS_NODISCARD Image *GetImage() const noexcept;

    KRYS_NODISCARD ImageLoader &GetImageLoader() const noexcept;

    /// @brief Starts the deferred load of the image, if it's lazily loaded.
    void LoadDeferredImage() noexcept;

    /// @brief Whether the image is lazily loaded, and its load is deferred until it intersects the viewport.
    KRYS_NODISCARD bool IsDeferred() const noexcept;

    /// @brief Whether the image will load lazily: its `loading` attribute is `lazy`, and scripting is
    /// enabled.
    /// @see https://html.spec.whatwg.org/multipage/urls-and-fetching.html#will-lazy-load-element-steps
    KRYS_NODISCARD bool IsLazyLoadable() const noexcept;

    /// @brief Whether a `loading` attribute value is `lazy`.
    KRYS_NODISCARD static bool HasLazyLoadableAttributeValue(Maybe<DOMStringView> attributeValue) noexcept;

    /// @brief The selected source: the best fitting candidate of a `<picture>`'s sources, or else of `srcset`
    /// and `src`.
    KRYS_NODISCARD Maybe<DOMString> ImageSourceUrl() const noexcept override;

    /// @brief The picture element the img is the child of, if any.
    KRYS_NODISCARD HTMLPictureElement *PictureElement() const noexcept;

    /// @brief Whether the image comes from a source set: the img's srcset, or its picture's sources.
    KRYS_NODISCARD bool UsesSrcsetOrPicture() const noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

    void DidAttachRenderers() noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    KRYS_NODISCARD HTMLElement &AsHTMLElement() noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD const HTMLElement &AsHTMLElement() const noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD html::FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    /// @brief Leaves the form owner, removed from the tree, unless the image went with it.
    /// @param formRoot The root of the form's tree.
    void FormOwnerRemovedFromTree(const Node &formRoot) noexcept;

    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAdopt(Document &oldDocument) noexcept override;

  private:
    /// @brief Sets the picture parent; and the source the current image came from.
    void SetPictureElement(HTMLPictureElement *pictureElement) noexcept;

    KRYS_NODISCARD HTMLSourceElement *SourceElement() const noexcept;

    void SetSourceElement(HTMLSourceElement *sourceElement) noexcept;

    /// @brief Restyles the element, as the attributes its presentational hints come from changed.
    void InvalidateAttributeMapping() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/images.html#select-an-image-source
    void SelectImageSource(RelevantMutation relevantMutation) noexcept;

    /// @brief The best image candidate among the picture parent's sources, or none.
    KRYS_NODISCARD html::ImageCandidate BestFitSourceFromPictureElement() noexcept;

    /// @brief Takes @p candidate's URL and the pixel density its descriptor gives.
    void SetBestFitUrlAndDPRFromImageCandidate(const html::ImageCandidate &candidate) noexcept;

    /// @brief The image's pixel density, from its candidate's descriptor; 1 for SVG images and before
    /// loading.
    KRYS_NODISCARD float EffectiveImageDevicePixelRatio() const noexcept;

    /// @brief Follows a change of the img's parent into or out of a picture, selecting the image again.
    void UpdatePictureElement() noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept override;

  protected:
    void ResetFormOwner() noexcept override;

    void SetFormInternal(HTMLFormElement *newForm) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLImageElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLImageElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::web::HTMLElement>(target);
    return element && element->IsHTMLImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
