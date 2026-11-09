/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGFEMergeElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGFEMergeElement::SVGFEMergeElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEMerge)
  {
  }

  void SVGFEMergeElement::OnChildrenChanged() noexcept
  {
    SVGFilterPrimitiveStandardAttributes::OnChildrenChanged();
    InstanceInvalidationGuard guard(*this);
    MarkFilterEffectForRebuild();
  }

  List<dom::DOMStringAtom> SVGFEMergeElement::FilterEffectInputsNames() const noexcept
  {
    List<dom::DOMStringAtom> inputsNames;
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *mergeNode = DynamicDowncast<SVGFEMergeNodeElement>(*child))
      {
        inputsNames.push_back(dom::DOMStringAtom(mergeNode->In1()));
      }
    }
    return inputsNames;
  }

  RefPtr<FilterEffect> SVGFEMergeElement::CreateFilterEffect(const FilterEffectVector &inputs,
                                                             const GraphicsContext &) const noexcept
  {
    return FEMerge::Create(static_cast<uint32>(inputs.size()));
  }
}
