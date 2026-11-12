/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTablePartElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLTableRowElement;

  /// @see https://html.spec.whatwg.org/#htmltablesectionelement
  class HTMLTableSectionElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableSectionElement);

  private:
    WeakPtr<dom::HTMLCollection> _rows;

  public:
    HTMLTableSectionElement(dom::Document &document) noexcept;

#pragma region HTMLTableSectionElement - https://html.spec.whatwg.org/#htmltablesectionelement

    /// @brief The section's `tr` children.
    /// @see https://html.spec.whatwg.org/#dom-tbody-rows
    KRYS_NODISCARD Ref<dom::HTMLCollection> Rows() noexcept;

    /// @brief A new row at @p index in the section's rows (-1 for the end): an `IndexSizeError` if @p index
    /// is less than -1 or more than the number of rows.
    /// @see https://html.spec.whatwg.org/#dom-tbody-insertrow
    dom::ExceptionOr<Ref<HTMLTableRowElement>> InsertRow(int32 index = -1) noexcept;

    /// @brief Removes the row at @p index in the section's rows (-1 for the last, if any): an
    /// `IndexSizeError` if there's no such row.
    /// @see https://html.spec.whatwg.org/#dom-tbody-deleterow
    dom::ExceptionOr<void> DeleteRow(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableSectionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableSectionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    void Align(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    void Char(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    void CharOff(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    void VAlign(dom::DOMString value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableSectionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLTableSectionElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableSectionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();