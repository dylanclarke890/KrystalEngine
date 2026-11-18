/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGTransformable.hpp"

namespace krys::boo
{
  SVGAnimateTransformElement::SVGAnimateTransformElement(Document &document) noexcept
      : SVGAnimateElementBase(document, SVGElementInterface::AnimateTransform)
  {
  }

  bool SVGAnimateTransformElement::HasValidAttributeType() const noexcept
  {
    if (!TargetElement())
    {
      return false;
    }

    if (GetAttributeType() == AttributeType::CSS)
    {
      return false;
    }

    return SVGAnimateElementBase::HasValidAttributeType();
  }

  void SVGAnimateTransformElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                                    Maybe<DOMStringView> value) noexcept
  {
    if (name == SVGNames::TypeAttr())
    {
      _type = SVGTransformable::ParseTransformTypeFromString(value.value_or(DOMStringView {}))
                .value_or(SVGTransformValue::SVG_TRANSFORM_UNKNOWN);
      if (_type == SVGTransformValue::SVG_TRANSFORM_MATRIX)
      {
        _type = SVGTransformValue::SVG_TRANSFORM_UNKNOWN;
      }
    }

    SVGAnimateElementBase::AttributeChanged(name, oldValue, value);
  }

  DOMString SVGAnimateTransformElement::AnimateRangeString(DOMStringView string) const noexcept
  {
    DOMString result {SVGTransformValue::PrefixForTransformType(_type)};
    result += string;
    result += u8')';
    return result;
  }
}
