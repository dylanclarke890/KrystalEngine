/*
 * Copyright (C) 2006, 2011, 2012 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Collections/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"

namespace krys::boo::html
{
  HTMLOptionsCollection::HTMLOptionsCollection(HTMLSelectElement &select) noexcept
      : LiveHTMLCollection(select,
                           [&select](const dom::Element &element)
                           {
                             auto *option = DynamicDowncast<HTMLOptionElement>(element);
                             return option && option->OwnerSelectElement() == &select;
                           }),
        _selectElement(ShareRef(select))
  {
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::Length(uint32 length) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->Length(length);
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::SetItem(uint32 index, HTMLOptionElement *option) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->SetItem(index, option);
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::Add(const OptionOrOptGroupElement &element,
                                                    const Maybe<HTMLElementOrInt> &before) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->Add(element, before);
  }

  void HTMLOptionsCollection::Remove(int32 index) noexcept
  {
    CEReactionsScope ceReactions;
    _selectElement->Remove(index);
  }

  int32 HTMLOptionsCollection::SelectedIndex() const noexcept
  {
    return _selectElement->SelectedIndex();
  }

  void HTMLOptionsCollection::SelectedIndex(int32 index) noexcept
  {
    _selectElement->SelectedIndex(index);
  }
}
