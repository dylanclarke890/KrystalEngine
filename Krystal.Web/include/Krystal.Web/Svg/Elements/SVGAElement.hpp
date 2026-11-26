/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
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

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Web/Svg/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Web/Svg/SVGURIReference.hpp"

namespace krys::web
{
  class DOMTokenList;
}

namespace krys::web
{
  /// @brief The link types of an `<a>`'s `rel` that change how it's followed.
  enum class SVGALinkRelation : uint8
  {
    NoReferrer,
  };

  consteval SVGALinkRelation MaxEnumValue(SVGALinkRelation) noexcept
  {
    return SVGALinkRelation::NoReferrer;
  }

  /// @brief The `<a>` element: a hyperlink around graphics.
  /// @see https://svgwg.org/svg2-draft/linking.html#InterfaceSVGAElement
  class SVGAElement final : public SVGGraphicsElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAElement);

  private:
    Ref<SVGAnimatedString> _target;

    EnumSet<SVGALinkRelation> _linkRelations;

    UniquePtr<DOMTokenList> _relList;

  public:
    explicit SVGAElement(Document &document) noexcept;

    ~SVGAElement() noexcept override;

    /// @see https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__target
    KRYS_NODISCARD DOMStringAtom Target() const noexcept
    {
      return DOMStringAtom {_target->CurrentValue()};
    }

    KRYS_NODISCARD Ref<SVGAnimatedString> &TargetAnimated() noexcept
    {
      return _target;
    }

    /// @see https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__relList
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGAElement, SVGGraphicsElement, SVGURIReference>;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const Node &) const noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }

    void ActivationBehavior(Event &event) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

    KRYS_NODISCARD bool HasRel(SVGALinkRelation) const noexcept;

    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    void FollowTheHyperlink(Event &event) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGAElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGAElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
