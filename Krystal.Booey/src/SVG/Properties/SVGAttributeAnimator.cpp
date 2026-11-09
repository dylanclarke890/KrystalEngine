/*
 * Copyright (C) 2018-2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo::svg
{
  bool SVGAttributeAnimator::IsAnimatedStylePropertyAnimator(const SVGElement &targetElement) const noexcept
  {
    return targetElement.IsAnimatedStyleAttribute(_attributeName);
  }

  void SVGAttributeAnimator::InvalidateStyle(SVGElement &targetElement) noexcept
  {
    SVGElement::InstanceInvalidationGuard guard(targetElement);
    targetElement.SetPresentationalHintStyleIsDirty();
  }

  void SVGAttributeAnimator::ApplyAnimatedStylePropertyChange(SVGElement &element, css::PropertyId id,
                                                              dom::DOMStringView value) noexcept
  {
    krys_debug_assert(id != css::PropertyId::Invalid);

    if (!element.EnsureAnimatedSMILStyleProperties().SetProperty(id, css::CSSOMString(value)))
    {
      return;
    }
    element.InvalidateStyle();
  }

  void SVGAttributeAnimator::ApplyAnimatedStylePropertyChange(SVGElement &targetElement,
                                                              dom::DOMStringView value) noexcept
  {
    // FIXME: Do we really need to check both isConnected and !parentNode?
    if (!targetElement.IsConnected() || !targetElement.ParentNode())
    {
      return;
    }

    auto id = css::PropertyIdFromName(_attributeName.LocalName().View());

    SVGElement::InstanceUpdateBlocker blocker(targetElement);
    ApplyAnimatedStylePropertyChange(targetElement, id, value);

    // If the target element has instances, update them as well, w/o requiring the <use> tree to be rebuilt.
    for (auto &instance : targetElement.Instances())
    {
      ApplyAnimatedStylePropertyChange(*instance, id, value);
    }
  }

  void SVGAttributeAnimator::RemoveAnimatedStyleProperty(SVGElement &element, css::PropertyId id) noexcept
  {
    krys_debug_assert(id != css::PropertyId::Invalid);

    element.EnsureAnimatedSMILStyleProperties().RemoveProperty(id);
    element.InvalidateStyle();
  }

  void SVGAttributeAnimator::RemoveAnimatedStyleProperty(SVGElement &targetElement) noexcept
  {
    // FIXME: Do we really need to check both isConnected and !parentNode?
    if (!targetElement.IsConnected() || !targetElement.ParentNode())
    {
      return;
    }

    auto id = css::PropertyIdFromName(_attributeName.LocalName().View());

    SVGElement::InstanceUpdateBlocker blocker(targetElement);
    RemoveAnimatedStyleProperty(targetElement, id);

    // If the target element has instances, update them as well, w/o requiring the <use> tree to be rebuilt.
    for (auto &instance : targetElement.Instances())
    {
      RemoveAnimatedStyleProperty(*instance, id);
    }
  }

  void SVGAttributeAnimator::ApplyAnimatedPropertyChange(SVGElement &element,
                                                         const dom::QualifiedName &attributeName) noexcept
  {
    element.SVGAttributeChanged(attributeName);
  }

  void SVGAttributeAnimator::ApplyAnimatedPropertyChange(SVGElement &targetElement) noexcept
  {
    // FIXME: Do we really need to check both isConnected and !parentNode?
    if (!targetElement.IsConnected() || !targetElement.ParentNode())
    {
      return;
    }

    SVGElement::InstanceUpdateBlocker blocker(targetElement);
    ApplyAnimatedPropertyChange(targetElement, _attributeName);

    // If the target element has instances, update them as well, w/o requiring the <use> tree to be rebuilt.
    for (auto &instance : targetElement.Instances())
    {
      ApplyAnimatedPropertyChange(*instance, _attributeName);
    }
  }
}
