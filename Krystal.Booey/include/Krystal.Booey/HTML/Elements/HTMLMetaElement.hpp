/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlmetaelement
  class HTMLMetaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMetaElement);

  public:
    HTMLMetaElement(dom::Document &document) noexcept;

#pragma region HTMLMetaElement - https://html.spec.whatwg.org/#htmlmetaelement

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-name
    void Name(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    KRYS_NODISCARD dom::DOMString HttpEquiv() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-httpequiv
    void HttpEquiv(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    KRYS_NODISCARD dom::DOMString Content() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-content
    void Content(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-media
    void Media(dom::DOMString value) noexcept;

#pragma endregion

#pragma region HTMLMetaElement Obsolete members - https://html.spec.whatwg.org/#HTMLMetaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    KRYS_NODISCARD dom::DOMString Scheme() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meta-scheme
    void Scheme(dom::DOMString value) noexcept;

#pragma endregion

  protected:
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    /// @brief Updates the document's colour schemes if the element's name is (or, as @p oldName, was)
    /// `color-scheme`, and runs its pragma directive, if it has one, while it's connected.
    /// @see https://html.spec.whatwg.org/#pragma-directives
    void Process(Maybe<dom::DOMStringView> oldName = null) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMetaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMetaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();