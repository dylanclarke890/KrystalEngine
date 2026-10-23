#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Loader/ImageLoader.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class Image;
}

namespace krys::boo::html
{
  class HTMLImageLoader;
  class HTMLMapElement;
  class HTMLPictureElement;
  class HTMLSourceElement;
  struct ImageCandidate;

  /// @see https://html.spec.whatwg.org/#htmlimageelement
  // FIXME: Forms (form association), lazy loading, decode(), the legacy Image()
  // constructor, and re-selecting the source when the media queries it depends on change.
  class HTMLImageElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLImageElement);
    friend class HTMLPictureElement;

  private:
    UniquePtr<HTMLImageLoader> _imageLoader;
    dom::DOMString _bestFitImageURL;
    url::URL _currentURL;
    float _imageDevicePixelRatio {1.0f};
    WeakPtr<HTMLPictureElement> _pictureElement;
    // The source element that was selected to provide the source URL.
    WeakPtr<HTMLSourceElement> _sourceElement;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CanStartSelection() const noexcept override
    {
      if (ShadowRoot())
      {
        return HTMLElement::CanStartSelection();
      }
      return false;
    }

    HTMLImageElement(dom::Document &document) noexcept;
    ~HTMLImageElement() noexcept override;

    /// @brief The image map `usemap` names: the first `map` in the image's tree whose `id` or `name` is the
    /// name after its `#`.
    /// @see https://html.spec.whatwg.org/#image-map
    KRYS_NODISCARD HTMLMapElement *AssociatedMapElement() const noexcept;

#pragma region HTMLImageElement - https://html.spec.whatwg.org/#htmlimageelement

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    KRYS_NODISCARD dom::DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    void Alt(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    void Src(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    KRYS_NODISCARD dom::USVString SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    void SrcSet(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD dom::DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    KRYS_NODISCARD dom::DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    void UseMap(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-ismap
    KRYS_NODISCARD bool IsMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-ismap
    void IsMap(bool value) noexcept;

    /// @brief Whether the image is a server-side image map: it has `ismap` and no client-side map
    /// (`usemap`), so a link around it is followed with the coordinates clicked.
    KRYS_NODISCARD bool IsServerMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-controls
    KRYS_NODISCARD bool Controls() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-controls
    void Controls(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-naturalwidth
    KRYS_NODISCARD uint32 NaturalWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-naturalheight
    KRYS_NODISCARD uint32 NaturalHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-complete
    KRYS_NODISCARD bool Complete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-currentsrc
    KRYS_NODISCARD dom::USVString CurrentSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    KRYS_NODISCARD dom::DOMString Decoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    void Decoding(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    KRYS_NODISCARD dom::DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    void Loading(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    KRYS_NODISCARD dom::DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    void FetchPriority(dom::DOMString &&value) noexcept;

    // TODO(HTMLIMAGEELEMENT, HTML): Promise<undefined> decode();

#pragma endregion

#pragma region HTMLImageElement Obsolete members - https://html.spec.whatwg.org/#HTMLImageElement-partial

    /// @see https://html.spec.whatwg.org/#dom-img-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    KRYS_NODISCARD dom::USVString LowSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    void LowSrc(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    KRYS_NODISCARD dom::USVString LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-longdesc
    void LongDesc(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    KRYS_NODISCARD dom::DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    void Border(dom::DOMString &&value) noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsReplaced(const RenderStyle *style = nullptr) const noexcept override;

    /// @brief The URL of the selected source, resolved against the document.
    KRYS_NODISCARD const url::URL &CurrentURL() const noexcept
    {
      return _currentURL;
    }

    /// @brief The `alt` attribute, or else the `title` attribute, or null if it has neither.
    KRYS_NODISCARD Maybe<dom::DOMString> AltText() const noexcept;

    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;
    KRYS_NODISCARD Image *GetImage() const noexcept;
    KRYS_NODISCARD ImageLoader &GetImageLoader() const noexcept;

    /// @brief The selected source: the best fitting candidate of a `<picture>`'s sources, or else of `srcset`
    /// and `src`.
    KRYS_NODISCARD Maybe<dom::DOMString> ImageSourceURL() const noexcept override;

    KRYS_NODISCARD HTMLPictureElement *PictureElement() const noexcept;
    KRYS_NODISCARD bool UsesSrcsetOrPicture() const noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
    void DidAttachRenderers() noexcept override;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnAdopt(dom::Document &oldDocument) noexcept override;

  private:
    void SetPictureElement(HTMLPictureElement *pictureElement) noexcept;
    KRYS_NODISCARD HTMLSourceElement *SourceElement() const noexcept;
    void SetSourceElement(HTMLSourceElement *sourceElement) noexcept;
    void InvalidateAttributeMapping() noexcept;

    void SelectImageSource(RelevantMutation relevantMutation) noexcept;
    KRYS_NODISCARD ImageCandidate BestFitSourceFromPictureElement() noexcept;
    void SetBestFitURLAndDPRFromImageCandidate(const ImageCandidate &candidate) noexcept;
    KRYS_NODISCARD float EffectiveImageDevicePixelRatio() const noexcept;
    void UpdatePictureElement() noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
    void CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLImageElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();