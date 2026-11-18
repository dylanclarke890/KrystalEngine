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

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmltablecellelement
  class HTMLTableCellElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCellElement);

  public:
    HTMLTableCellElement(Document &document) noexcept;

#pragma region HTMLTableCellElement - https://html.spec.whatwg.org/#htmltablecellelement

    /// @see https://html.spec.whatwg.org/#dom-tdth-colspan
    KRYS_NODISCARD uint32 ColSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-colspan
    void ColSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-rowspan
    KRYS_NODISCARD uint32 RowSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-rowspan
    void RowSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-headers
    KRYS_NODISCARD DOMString Headers() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-headers
    void Headers(DOMString value) noexcept;

    /// @brief The cell's index in its row's cells, or -1 if its parent isn't a row.
    /// @see https://html.spec.whatwg.org/#dom-tdth-cellindex
    KRYS_NODISCARD int32 CellIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    KRYS_NODISCARD DOMString Scope() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    void Scope(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    KRYS_NODISCARD DOMString Abbr() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    void Abbr(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCellElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    KRYS_NODISCARD DOMString Axis() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    void Axis(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    void Height(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    void Char(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    void CharOff(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    KRYS_NODISCARD bool NoWrap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    void NoWrap(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    void VAlign(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    void BgColor(DOMString value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTableCellElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTableCellElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCellElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
