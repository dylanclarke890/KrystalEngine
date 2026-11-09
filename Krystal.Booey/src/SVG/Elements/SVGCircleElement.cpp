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

#include "Krystal.Booey/SVG/Elements/SVGCircleElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGEllipse.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGCircleElement::SVGCircleElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Circle),
        _cx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _cy(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _r(SVGAnimatedLength::Create(this, SVGLengthMode::Other))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_cx>(SVGNames::CxAttr());
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_cy>(SVGNames::CyAttr());
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_r>(SVGNames::RAttr());
    }
  }

  SVGAnimatedProperty *SVGCircleElement::PropertyForAttribute(const dom::QualifiedName &name) const noexcept
  {
    if (name == SVGNames::CxAttr())
    {
      return _cx.get();
    }
    if (name == SVGNames::CyAttr())
    {
      return _cy.get();
    }
    if (name == SVGNames::RAttr())
    {
      return _r.get();
    }
    return nullptr;
  }

  void SVGCircleElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::CxAttr())
    {
      _cx->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::CyAttr())
    {
      _cy->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::RAttr())
    {
      _r->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError,
                                                       SVGLengthNegativeValuesMode::Forbid));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGCircleElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGCircleElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGEllipse>(*this, krys::move(style));
  }
}
