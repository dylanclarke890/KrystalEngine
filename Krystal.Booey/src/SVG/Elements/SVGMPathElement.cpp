/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGMPathElement.hpp"
#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGMPathElement::SVGMPathElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::MPath),
        SVGURIReference(this)
  {
  }

  SVGMPathElement::~SVGMPathElement() noexcept
  {
    ClearResourceReferences();
  }

  void SVGMPathElement::BuildPendingResource() noexcept
  {
    ClearResourceReferences();
    if (!IsConnected())
    {
      return;
    }

    auto target = SVGURIReference::TargetElementFromIRIString(Href(), TreeScopeForSVGReferences());
    if (!target.Element)
    {
      // Do not register as pending if we are already pending this resource.
      auto &treeScope = TreeScopeForSVGReferences();
      if (treeScope.IsPendingSVGResource(*this, target.Identifier))
      {
        return;
      }

      if (!target.Identifier.View().empty())
      {
        treeScope.AddPendingSVGResource(target.Identifier, *this);
        krys_debug_assert(HasPendingResources());
      }
    }
    else if (auto *svgElement = DynamicDowncast<SVGElement>(target.Element.get()))
    {
      svgElement->AddReferencingElement(*this);
    }

    TargetPathChanged();
  }

  void SVGMPathElement::ClearResourceReferences() noexcept
  {
    RemoveElementReference();
  }

  void SVGMPathElement::OnPostConnection() noexcept
  {
    SVGElement::OnPostConnection();
    BuildPendingResource();
  }

  void SVGMPathElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    bool disconnectedFromDocument = oldAncestor.IsConnected() && !IsConnected();
    SVGElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (disconnectedFromDocument)
    {
      ClearResourceReferences();
    }
  }

  void SVGMPathElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                         Maybe<dom::DOMStringView> value) noexcept
  {
    SVGURIReference::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGMPathElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      BuildPendingResource();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  RefPtr<SVGPathElement> SVGMPathElement::PathElement() noexcept
  {
    auto target = TargetElementFromIRIString(Href(), TreeScopeForSVGReferences());
    auto *svgElement = DynamicDowncast<SVGElement>(target.Element.get());
    return svgElement ? ShareRefPtr(DynamicDowncast<SVGPathElement>(*svgElement)) : nullptr;
  }

  void SVGMPathElement::TargetPathChanged() noexcept
  {
    if (RefPtr animateMotionElement = ShareRefPtr(DynamicDowncast<SVGAnimateMotionElement>(ParentNode())))
    {
      animateMotionElement->UpdateAnimationPath();
    }
  }
}
