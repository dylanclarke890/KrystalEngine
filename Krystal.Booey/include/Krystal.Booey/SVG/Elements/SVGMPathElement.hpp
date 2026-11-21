/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#pragma once

#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  class SVGPathElement;

  /// @brief The `<mpath>` element: references the `<path>` its parent `<animateMotion>` moves along.
  /// @see https://svgwg.org/specs/animations/#MPathElement
  class SVGMPathElement final : public SVGElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMPathElement);

  public:
    explicit SVGMPathElement(Document &) noexcept;

    ~SVGMPathElement() noexcept override;

    KRYS_NODISCARD RefPtr<SVGPathElement> PathElement() noexcept;

    void TargetPathChanged() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGMPathElement, SVGElement, SVGURIReference>;

  private:
    void AttributeChanged(const QualifiedName &, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &) noexcept final;

    void BuildPendingResource() noexcept final;

    void ClearResourceReferences() noexcept;

    void OnPostConnection() noexcept final;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept final;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGMPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::SVGElementInterface::MPath;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
