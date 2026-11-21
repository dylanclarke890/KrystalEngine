/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2010-2015 Google Inc. All rights reserved.
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
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Forms/FormAssociatedElement.hpp"
#include "Krystal.Booey/Loader/ImageLoader.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class HTMLMapElement;
  class HTMLPictureElement;
  class HTMLSourceElement;
  class Image;
}

namespace krys::boo::html
{
  class HTMLImageLoader;
  struct ImageCandidate;
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmlimageelement
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
    // The source element that was selected to provide the source URL.
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
    /// @see https://html.spec.whatwg.org/#image-map
    KRYS_NODISCARD HTMLMapElement *AssociatedMapElement() const noexcept;

#pragma region HTMLImageElement - https://html.spec.whatwg.org/#htmlimageelement

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-alt
    void Alt(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    KRYS_NODISCARD dom::USVString SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-srcset
    void SrcSet(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-crossorigin
    void CrossOrigin(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-usemap
    void UseMap(DOMString value) noexcept;

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
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    KRYS_NODISCARD DOMString Decoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-decoding
    void Decoding(DOMString value) noexcept;

    /// @brief The decoding hint `decoding` gives: synchronous, asynchronous, or the engine's choice.
    /// @see https://html.spec.whatwg.org/#image-decoding-hint
    KRYS_NODISCARD DecodingMode GetDecodingMode() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    KRYS_NODISCARD DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-loading
    void Loading(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-fetchpriority
    void FetchPriority(DOMString value) noexcept;

    /// @brief Calls @p promise, as a microtask, once the image has loaded and decoded, so it can be drawn
    /// without decoding; it's rejected with an `EncodingError` if it can't be.
    /// @see https://html.spec.whatwg.org/#dom-img-decode
    void Decode(ImageLoader::DecodePromise promise) noexcept;

#pragma endregion

#pragma region HTMLImageElement Obsolete members - https://html.spec.whatwg.org/#HTMLImageElement-partial

    /// @see https://html.spec.whatwg.org/#dom-img-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    KRYS_NODISCARD dom::USVString LowSrc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-lowsrc
    void LowSrc(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-align
    void Align(DOMString value) noexcept;

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
    void LongDesc(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-border
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

    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;

    KRYS_NODISCARD Image *GetImage() const noexcept;

    KRYS_NODISCARD ImageLoader &GetImageLoader() const noexcept;

    /// @brief Starts the deferred load of the image, if it's lazily loaded.
    void LoadDeferredImage() noexcept;

    /// @brief Whether the image is lazily loaded, and its load is deferred until it intersects the viewport.
    KRYS_NODISCARD bool IsDeferred() const noexcept;

    /// @brief Whether the image will load lazily: its `loading` attribute is `lazy`, and scripting is
    /// enabled.
    /// @see https://html.spec.whatwg.org/#will-lazy-load-element-steps
    KRYS_NODISCARD bool IsLazyLoadable() const noexcept;

    KRYS_NODISCARD static bool HasLazyLoadableAttributeValue(Maybe<DOMStringView> attributeValue) noexcept;

    /// @brief The selected source: the best fitting candidate of a `<picture>`'s sources, or else of `srcset`
    /// and `src`.
    KRYS_NODISCARD Maybe<DOMString> ImageSourceUrl() const noexcept override;

    KRYS_NODISCARD HTMLPictureElement *PictureElement() const noexcept;

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
    void SetPictureElement(HTMLPictureElement *pictureElement) noexcept;

    KRYS_NODISCARD HTMLSourceElement *SourceElement() const noexcept;

    void SetSourceElement(HTMLSourceElement *sourceElement) noexcept;

    void InvalidateAttributeMapping() noexcept;

    void SelectImageSource(RelevantMutation relevantMutation) noexcept;

    KRYS_NODISCARD html::ImageCandidate BestFitSourceFromPictureElement() noexcept;

    void SetBestFitUrlAndDPRFromImageCandidate(const html::ImageCandidate &candidate) noexcept;

    KRYS_NODISCARD float EffectiveImageDevicePixelRatio() const noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLImageElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::HTMLElement>(target);
    return element && element->IsHTMLImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
