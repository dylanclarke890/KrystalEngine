/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2010 Apple Inc. All rights reserved.
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
 */

#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"

namespace krys::boo::html
{
  HTMLMapElement::HTMLMapElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Map)
  {
  }

  bool HTMLMapElement::MapMouseEvent(LayoutPoint location, const LayoutSize &size,
                                     HitTestResult &result) noexcept
  {
    HTMLAreaElement *defaultArea = nullptr;

    for (auto &node : dom::DescendantRange(*this))
    {
      auto *area = DynamicDowncast<HTMLAreaElement>(node);
      if (!area)
      {
        continue;
      }
      if (area->IsDefault())
      {
        if (!defaultArea)
        {
          defaultArea = area;
        }
      }
      else if (area->MapMouseEvent(location, size, result))
      {
        return true;
      }
    }

    if (defaultArea)
    {
      result.SetInnerNode(defaultArea);
      result.SetURLElement(defaultArea);
    }
    return defaultArea;
  }

  RefPtr<HTMLImageElement> HTMLMapElement::ImageElement() const noexcept
  {
    auto &root = dom::TreeQueries::Root(const_cast<HTMLMapElement &>(*this));
    for (auto &node : dom::DescendantRange(root))
    {
      auto *image = DynamicDowncast<HTMLImageElement>(node);
      if (image && image->AssociatedMapElement() == this)
      {
        return ShareRefPtr(image);
      }
    }
    return nullptr;
  }

#pragma region HTMLMapElement

  dom::DOMString HTMLMapElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMapElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  Ref<dom::HTMLCollection> HTMLMapElement::Areas() noexcept
  {
    return CreateRef<dom::LiveHTMLCollection>(*this, [](const dom::Element &element)
                                              { return Is<HTMLAreaElement>(element); });
  }

#pragma endregion

  void HTMLMapElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value,
                                          dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"id" || localName == u8"name")
        && oldValue != value && (IsConnected() || IsInShadowTree()))
    {
      auto &scope = TreeScope();
      if (oldValue && !oldValue->empty())
      {
        scope.RemoveImageMap(dom::DOMStringAtom(*oldValue), *this);
      }
      if (value && !value->empty())
      {
        scope.AddImageMap(dom::DOMStringAtom(*value), *this);
      }
    }
  }
}
