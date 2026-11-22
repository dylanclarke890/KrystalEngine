/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  class HTMLImageElement;
  class HitTestResult;

  /// @see https://html.spec.whatwg.org/#htmlmapelement
  class HTMLMapElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMapElement);

  public:
    HTMLMapElement(Document &document) noexcept;

    /// @brief Hit tests the map's areas at @p location in an image of @p size: the first area whose shape
    /// contains it, or else the first default area, becomes the result's inner node and link.
    /// @return Whether an area was hit.
    bool MapMouseEvent(LayoutPoint location, const LayoutSize &size, HitTestResult &result) noexcept;

    /// @brief The first image in the map's tree that uses it.
    KRYS_NODISCARD RefPtr<HTMLImageElement> ImageElement() const noexcept;

#pragma region HTMLMapElement - https://html.spec.whatwg.org/#htmlmapelement

    /// @see https://html.spec.whatwg.org/#dom-map-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-areas
    KRYS_NODISCARD Ref<dom::HTMLCollection> Areas() noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLMapElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLMapElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
