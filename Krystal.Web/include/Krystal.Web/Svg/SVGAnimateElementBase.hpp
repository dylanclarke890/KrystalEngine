/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2008-2019 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Web/Svg/Elements/SVGAnimationElement.hpp"

namespace krys::web
{
  class SVGAttributeAnimator;

  /// @brief The base of the animation elements that animate an attribute or property through an animator:
  /// `<animate>`, `<set>` and `<animateTransform>`.
  class SVGAnimateElementBase : public SVGAnimationElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAnimateElementBase);

  private:
    mutable RefPtr<SVGAttributeAnimator> _animator;
    mutable Maybe<bool> _hasInvalidCSSAttributeType;

  public:
    KRYS_NODISCARD bool IsDiscreteAnimator() const noexcept;

  protected:
    SVGAnimateElementBase(Document &, SVGElementInterface) noexcept;

    KRYS_NODISCARD bool HasValidAttributeType() const noexcept override;

    KRYS_NODISCARD virtual DOMString AnimateRangeString(DOMStringView string) const noexcept
    {
      return DOMString {string};
    }

  private:
    KRYS_NODISCARD SVGAttributeAnimator *Animator() const noexcept;

    KRYS_NODISCARD SVGAttributeAnimator *AnimatorIfExists() const noexcept
    {
      return _animator.get();
    }

    void SetTargetElement(SVGElement *) noexcept override;

    void SetAttributeName(const QualifiedName &) noexcept override;

    void ResetAnimation() noexcept override;

    bool SetFromAndToValues(DOMStringView fromString, DOMStringView toString) noexcept override;

    bool SetFromAndByValues(DOMStringView fromString, DOMStringView byString) noexcept override;

    bool SetToAtEndOfDurationValue(DOMStringView toAtEndOfDurationString) noexcept override;

    void StartAnimation() noexcept override;

    void CalculateAnimatedValue(float progress, uint32 repeatCount) noexcept override;

    void ApplyResultsToTarget() noexcept override;

    void StopAnimation(SVGElement *targetElement) noexcept override;

    KRYS_NODISCARD Maybe<float> CalculateDistance(DOMStringView fromString,
                                                  DOMStringView toString) noexcept override;

    KRYS_NODISCARD bool HasInvalidCSSAttributeType() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGAnimateElementBase)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &element) noexcept
  {
    using krys::web::SVGElementInterface;
    auto interface = element.Interface();
    return interface == SVGElementInterface::Animate || interface == SVGElementInterface::AnimateTransform
           || interface == SVGElementInterface::Set;
  }
  KRYS_NODISCARD static bool IsType(const krys::web::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::web::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
