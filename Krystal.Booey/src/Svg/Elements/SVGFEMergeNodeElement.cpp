/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/Svg/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"

namespace krys::boo
{
  SVGFEMergeNodeElement::SVGFEMergeNodeElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::FEMergeNode),
        _in1(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEMergeNodeElement::_in1>(SVGNames::InAttr());
    }
  }

  void SVGFEMergeNodeElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                               Maybe<DOMStringView> value) noexcept
  {
    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(DOMString(value.value_or(DOMStringView {})));
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGFEMergeNodeElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::InAttr());
      InstanceInvalidationGuard guard(*this);
      SVGFilterPrimitiveStandardAttributes::InvalidateFilterPrimitiveParent(this);
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }
}
