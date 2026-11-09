/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGLineElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGLineElement::SVGLineElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Line),
        _x1(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y1(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _x2(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y2(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGLineElement::_x1>(SVGNames::X1Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_y1>(SVGNames::Y1Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_x2>(SVGNames::X2Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_y2>(SVGNames::Y2Attr());
    }
  }

  void SVGLineElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::X1Attr())
    {
      _x1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::Y1Attr())
    {
      _y1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::X2Attr())
    {
      _x2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::Y2Attr())
    {
      _y2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGLineElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      UpdateRelativeLengthsInformation();

      if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(Renderer()))
      {
        shape->SetNeedsShapeUpdate();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  bool SVGLineElement::SelfHasRelativeLengths() const noexcept
  {
    return X1().IsRelative() || Y1().IsRelative() || X2().IsRelative() || Y2().IsRelative();
  }
}
