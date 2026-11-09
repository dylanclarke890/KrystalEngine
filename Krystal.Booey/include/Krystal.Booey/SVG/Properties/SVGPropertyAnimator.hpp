/*
 * Copyright (C) 2019-2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Style/Extractor/Extractor.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute's animated style property with @p TAnimationFunction.
  template <typename TAnimationFunction>
  class SVGPropertyAnimator : public SVGAttributeAnimator
  {
  protected:
    TAnimationFunction _function;

  public:
    KRYS_NODISCARD bool IsDiscrete() const noexcept override
    {
      return _function.IsDiscrete();
    }

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _function.SetFromAndToValues(targetElement, AdjustForInheritance(targetElement, from),
                                   AdjustForInheritance(targetElement, to));
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept override
    {
      _function.SetFromAndByValues(targetElement, from, by);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _function.SetToAtEndOfDurationValue(targetElement, toAtEndOfDuration);
    }

  protected:
    template <typename... TArgs>
    SVGPropertyAnimator(const dom::QualifiedName &attributeName, TArgs &&...arguments) noexcept
        : SVGAttributeAnimator(attributeName), _function(std::forward<TArgs>(arguments)...)
    {
    }

    void Stop(SVGElement &targetElement) noexcept override
    {
      RemoveAnimatedStyleProperty(targetElement);
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &targetElement, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override
    {
      return _function.CalculateDistance(targetElement, from, to);
    }

    KRYS_NODISCARD dom::DOMString AdjustForInheritance(SVGElement &targetElement,
                                                       dom::DOMStringView value) const noexcept
    {
      return value == u8"inherit" ? ComputeInheritedCSSPropertyValue(targetElement) : dom::DOMString(value);
    }

    KRYS_NODISCARD dom::DOMString ComputeCSSPropertyValue(SVGElement &targetElement,
                                                          css::PropertyId id) const noexcept
    {
      Ref protector = ShareRef(targetElement);

      // Don't include any properties resulting from CSS Transitions/Animations or SMIL animations, as we want
      // to retrieve the "base value".
      targetElement.SetUseOverrideComputedStyle(true);
      auto serialization =
        style::Extractor(&targetElement).PropertyValueSerialisation(id, css::DefaultSerialisationContext());
      targetElement.SetUseOverrideComputedStyle(false);

      return dom::DOMString(serialization);
    }

    KRYS_NODISCARD dom::DOMString ComputeInheritedCSSPropertyValue(SVGElement &targetElement) const noexcept
    {
      RefPtr svgParent = ShareRefPtr(DynamicDowncast<SVGElement>(targetElement.ParentElement()));
      if (!svgParent)
      {
        return {};
      }
      return ComputeCSSPropertyValue(*svgParent, css::PropertyIdFromName(_attributeName.LocalName().View()));
    }
  };
}
