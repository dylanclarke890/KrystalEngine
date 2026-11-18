/*
 * Copyright (C) 2007-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  class HTMLPictureElement;

  /// @see https://html.spec.whatwg.org/#htmlsourceelement
  // FIXME: Media element sources (sourceWasAdded, error events), once there are media elements.
  class HTMLSourceElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSourceElement);

  private:
    mutable Maybe<css::mq::MediaQueryList> _cachedParsedMediaAttribute;
    // The <picture> the source was inserted into, while it's its parent.
    HTMLPictureElement *_insertedIntoPicture {nullptr};
    // Whether the source precedes the picture's images, so changes to it can change their selected source.
    bool _shouldCallSourcesChanged {false};

  public:
    HTMLSourceElement(Document &document) noexcept;

#pragma region HTMLSourceElement - https://html.spec.whatwg.org/#htmlsourceelement

    /// @see https://html.spec.whatwg.org/#dom-source-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    KRYS_NODISCARD dom::USVString SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    void SrcSet(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    void Sizes(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    void Media(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    void Height(uint32 value) noexcept;

#pragma endregion

    /// @brief The `media` attribute, parsed: an empty or all-whitespace one is an empty list, which matches
    /// the environment.
    /// @see https://html.spec.whatwg.org/#matches-the-environment
    KRYS_NODISCARD const css::mq::MediaQueryList &ParsedMediaAttribute(Document &document) const noexcept;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

  private:
    /// @brief Records that the source is in @p pictureElement, and whether it's one of its image's sources
    /// (whether it precedes an img).
    void UpdateInsertedIntoPicture(HTMLPictureElement &pictureElement) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLSourceElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLSourceElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::HTMLElement>(target);
    return element && element->IsHTMLSourceElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
