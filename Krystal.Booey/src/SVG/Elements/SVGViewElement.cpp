/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGViewElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGViewElement::SVGViewElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::View),
        SVGFitToViewBox(this)
  {
  }

  const SVGSvgElement *SVGViewElement::TargetElement() const noexcept
  {
    return _targetElement.get();
  }

  void SVGViewElement::SetTargetElement(const SVGSvgElement &targetElement) noexcept
  {
    _targetElement = CreateWeakPtr(&targetElement);
  }

  void SVGViewElement::ResetTargetElement() noexcept
  {
    _targetElement = nullptr;
  }

  void SVGViewElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    SVGFitToViewBox::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGViewElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    // We ignore changes to SVGNames::viewTargetAttr, which is deprecated and unused.
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      return;
    }

    if (SVGFitToViewBox::IsKnownAttribute(attributeName))
    {
      auto *targetElement = _targetElement.get();
      if (!targetElement)
      {
        return;
      }
      targetElement->InheritViewAttributes(*this);
      targetElement->UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }
}
