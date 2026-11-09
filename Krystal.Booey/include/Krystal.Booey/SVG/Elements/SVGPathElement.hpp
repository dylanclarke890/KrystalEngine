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

#pragma once

#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"

namespace krys::boo::svg
{
  /// @brief The `<path>` element: a shape drawn by path data, from its `d` attribute or the `d` property.
  /// @see https://svgwg.org/svg2-draft/paths.html#InterfaceSVGPathElement
  class SVGPathElement final : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPathElement);

  private:
    Ref<SVGAnimatedPathSegList> _pathSegList;

  public:
    explicit SVGPathElement(dom::Document &document) noexcept;

    KRYS_NODISCARD float GetTotalLength() const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGPoint>> GetPointAtLength(float distance) const noexcept final;

    KRYS_NODISCARD FloatRect GetBBox(StyleUpdateStrategy = AllowStyleUpdate) noexcept final;

    /// @brief The path data the element is drawn with: the `d` property's once it has a renderer, otherwise
    /// the `d` attribute's.
    KRYS_NODISCARD const SVGPathByteStream &PathByteStream() const noexcept;
    KRYS_NODISCARD Path GetPath() const noexcept;

    void PathDidChange() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGPathElement, SVGGeometryElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SupportsMarkers() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    void InvalidateMPathDependencies() noexcept;

    void OnInsert() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;

    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept final;
    void CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept override;
    void CollectDPresentationalHint(css::MutableStyleProperties &style) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
