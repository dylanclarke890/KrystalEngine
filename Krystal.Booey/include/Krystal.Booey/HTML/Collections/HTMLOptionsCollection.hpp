/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLOptGroupElement;
  class HTMLOptionElement;
  class HTMLSelectElement;

  /// @brief A select's `options`: the options in its list of options.
  /// @see https://html.spec.whatwg.org/#htmloptionscollection
  class HTMLOptionsCollection final : public dom::LiveHTMLCollection
  {
  public:
    using OptionOrOptGroupElement = Variant<RefPtr<HTMLOptionElement>, RefPtr<HTMLOptGroupElement>>;
    using HTMLElementOrInt = Variant<RefPtr<HTMLElement>, int32>;

  private:
    Ref<HTMLSelectElement> _selectElement;

  public:
    explicit HTMLOptionsCollection(HTMLSelectElement &select) noexcept;

    KRYS_NODISCARD HTMLSelectElement &SelectElement() const noexcept
    {
      return *_selectElement;
    }

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-length
    dom::ExceptionOr<void> Length(uint32 length) noexcept;
    using LiveHTMLCollection::Length;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-setter
    dom::ExceptionOr<void> SetItem(uint32 index, HTMLOptionElement *option) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-add
    dom::ExceptionOr<void> Add(const OptionOrOptGroupElement &element,
                               const Maybe<HTMLElementOrInt> &before = {}) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-remove
    void Remove(int32 index) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex
    KRYS_NODISCARD int32 SelectedIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex
    void SelectedIndex(int32 index) noexcept;
  };
}
