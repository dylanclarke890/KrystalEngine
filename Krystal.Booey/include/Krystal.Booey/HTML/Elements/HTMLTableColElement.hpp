/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLTablePartElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltablecolelement
  class HTMLTableColElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableColElement);

  public:
    HTMLTableColElement(dom::Document &document) noexcept;

#pragma region HTMLTableColElement - https://html.spec.whatwg.org/#htmltablecolelement

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    KRYS_NODISCARD uint32 Span() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    void Span(uint32 value) noexcept;

#pragma endregion

#pragma region HTMLTableColElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableColElement-partial

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    void Align(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    void Char(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    void CharOff(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    void VAlign(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    void Width(dom::DOMString value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableColElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableColElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();