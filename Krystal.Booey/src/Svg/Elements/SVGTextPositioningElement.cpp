/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Rob Buis <buis@kde.org>
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/Svg/Elements/SVGTextPositioningElement.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"

namespace krys::boo
{
  SVGTextPositioningElement::SVGTextPositioningElement(Document &document,
                                                       UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                                       SVGElementInterface interface) noexcept
      : SVGTextContentElement(document, krys::move(propertyRegistry), interface,
                              SVGElementFlag::IsSvgTextPositioningElement),
        _x(SVGAnimatedLengthList::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLengthList::Create(this, SVGLengthMode::Height)),
        _dx(SVGAnimatedLengthList::Create(this, SVGLengthMode::Width)),
        _dy(SVGAnimatedLengthList::Create(this, SVGLengthMode::Height)),
        _rotate(SVGAnimatedNumberList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_dy>(SVGNames::DyAttr());
      PropertyRegistry::RegisterProperty<&SVGTextPositioningElement::_rotate>(SVGNames::RotateAttr());
    }
  }

  void SVGTextPositioningElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                                   Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});
    if (name == SVGNames::XAttr())
    {
      _x->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::YAttr())
    {
      _y->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::DxAttr())
    {
      _dx->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::RotateAttr())
    {
      _rotate->BaseVal()->Parse(string);
    }

    SVGTextContentElement::AttributeChanged(name, oldValue, value);
  }

  namespace
  {
    KRYS_NODISCARD bool IsXOrYAttribute(const DOMStringAtom &localName,
                                        const DOMStringAtom &namespaceURI) noexcept
    {
      return namespaceURI == null && (localName == u8"x" || localName == u8"y");
    }
  }

  void SVGTextPositioningElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    if (IsXOrYAttribute(localName, namespaceURI))
    {
      return;
    }
    SVGTextContentElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  bool SVGTextPositioningElement::HasPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI) const noexcept
  {
    if (IsXOrYAttribute(localName, namespaceURI))
    {
      return false;
    }
    return SVGTextContentElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGTextPositioningElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);

      if (attributeName != SVGNames::RotateAttr())
      {
        UpdateRelativeLengthsInformation();
      }

      if (auto *renderer = Renderer())
      {
        if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*renderer))
        {
          textAncestor->SetNeedsPositioningValuesUpdate();
        }
      }
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGTextContentElement::SVGAttributeChanged(attributeName);
  }

  RefPtr<SVGTextPositioningElement>
    SVGTextPositioningElement::ElementFromRenderer(RenderBoxModelObject &renderer) noexcept
  {
    if (!Is<RenderSVGText>(renderer) && !Is<RenderSVGInline>(renderer))
    {
      return nullptr;
    }

    krys_debug_assert(renderer.GetElement());
    return ShareRefPtr(
      DynamicDowncast<SVGTextPositioningElement>(Downcast<SVGElement>(renderer.GetElement())));
  }
}
