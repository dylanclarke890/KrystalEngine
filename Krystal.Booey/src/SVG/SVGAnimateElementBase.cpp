/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/SVGAnimateElementBase.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"

namespace krys::boo::svg
{
  SVGAnimateElementBase::SVGAnimateElementBase(dom::Document &document,
                                               SVGElementInterface interface) noexcept
      : SVGAnimationElement(document, interface)
  {
    krys_debug_assert(interface == SVGElementInterface::Animate || interface == SVGElementInterface::Set
                      || interface == SVGElementInterface::AnimateTransform);
  }

  SVGAttributeAnimator *SVGAnimateElementBase::Animator() const noexcept
  {
    krys_debug_assert(TargetElement());
    krys_debug_assert(!HasInvalidCSSAttributeType());

    if (!_animator)
    {
      _animator = TargetElement()->CreateAnimator(AttributeName(), GetAnimationMode(), GetCalcMode(),
                                                  IsAccumulated(), IsAdditive());
    }

    return _animator.get();
  }

  bool SVGAnimateElementBase::HasValidAttributeType() const noexcept
  {
    if (!TargetElement() || HasInvalidCSSAttributeType())
    {
      return false;
    }

    return TargetElement()->IsAnimatedAttribute(AttributeName());
  }

  bool SVGAnimateElementBase::HasInvalidCSSAttributeType() const noexcept
  {
    if (!TargetElement())
    {
      return false;
    }

    if (!_hasInvalidCSSAttributeType)
    {
      _hasInvalidCSSAttributeType = HasValidAttributeName() && GetAttributeType() == AttributeType::CSS
                                    && !IsTargetAttributeCSSProperty(TargetElement(), AttributeName());
    }

    return *_hasInvalidCSSAttributeType;
  }

  bool SVGAnimateElementBase::IsDiscreteAnimator() const noexcept
  {
    if (!HasValidAttributeType())
    {
      return false;
    }

    RefPtr animator = ShareRefPtr(Animator());
    return animator && animator->IsDiscrete();
  }

  void SVGAnimateElementBase::SetTargetElement(SVGElement *targetElement) noexcept
  {
    SVGAnimationElement::SetTargetElement(targetElement);
    ResetAnimation();
  }

  void SVGAnimateElementBase::SetAttributeName(const dom::QualifiedName &attributeName) noexcept
  {
    SVGSMILElement::SetAttributeName(attributeName);
    ResetAnimation();
  }

  void SVGAnimateElementBase::ResetAnimation() noexcept
  {
    SVGAnimationElement::ResetAnimation();
    _animator = nullptr;
    _hasInvalidCSSAttributeType = {};
  }

  bool SVGAnimateElementBase::SetFromAndToValues(dom::DOMStringView fromString,
                                                 dom::DOMStringView toString) noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return false;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->SetFromAndToValues(*target, AnimateRangeString(fromString), AnimateRangeString(toString));
      return true;
    }
    return false;
  }

  bool SVGAnimateElementBase::SetFromAndByValues(dom::DOMStringView fromString,
                                                 dom::DOMStringView byString) noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return false;
    }

    if (GetAnimationMode() == AnimationMode::By && (!IsAdditive() || IsDiscreteAnimator()))
    {
      return false;
    }

    if (GetAnimationMode() == AnimationMode::FromBy && IsDiscreteAnimator())
    {
      return false;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->SetFromAndByValues(*target, AnimateRangeString(fromString), AnimateRangeString(byString));
      return true;
    }
    return false;
  }

  bool SVGAnimateElementBase::SetToAtEndOfDurationValue(dom::DOMStringView toAtEndOfDurationString) noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target || toAtEndOfDurationString.empty())
    {
      return false;
    }

    if (IsDiscreteAnimator())
    {
      return true;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->SetToAtEndOfDurationValue(*target, AnimateRangeString(toAtEndOfDurationString));
      return true;
    }
    return false;
  }

  void SVGAnimateElementBase::StartAnimation() noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->Start(*target);
    }
  }

  void SVGAnimateElementBase::CalculateAnimatedValue(float progress, uint32 repeatCount) noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return;
    }

    krys_debug_assert(progress >= 0 && progress <= 1);
    if (Interface() == SVGElementInterface::Set)
    {
      progress = 1;
    }

    if (GetCalcMode() == CalcMode::Discrete)
    {
      progress = progress < 0.5f ? 0.0f : 1.0f;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->Animate(*target, progress, repeatCount);
    }
  }

  void SVGAnimateElementBase::ApplyResultsToTarget() noexcept
  {
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return;
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      animator->Apply(*target);
    }
  }

  void SVGAnimateElementBase::StopAnimation(SVGElement *targetElement) noexcept
  {
    RefPtr target = ShareRefPtr(targetElement);
    if (!target)
    {
      return;
    }

    if (RefPtr animator = ShareRefPtr(AnimatorIfExists()))
    {
      animator->Stop(*target);
    }
  }

  Maybe<float> SVGAnimateElementBase::CalculateDistance(dom::DOMStringView fromString,
                                                        dom::DOMStringView toString) noexcept
  {
    // FIXME: A return value of float is not enough to support paced animations on lists.
    RefPtr target = ShareRefPtr(TargetElement());
    if (!target)
    {
      return {};
    }

    if (RefPtr animator = ShareRefPtr(Animator()))
    {
      return animator->CalculateDistance(*target, fromString, toString);
    }

    return {};
  }
}
