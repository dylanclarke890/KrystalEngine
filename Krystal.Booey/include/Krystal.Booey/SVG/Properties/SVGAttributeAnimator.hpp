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

#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  /// @see https://svgwg.org/specs/animations/#AnimationModes
  enum class AnimationMode : uint8
  {
    None,
    FromTo,
    FromBy,
    To,
    By,
    Values,
    Path
  };

  /// @see https://svgwg.org/specs/animations/#CalcModeAttribute
  enum class CalcMode : uint8
  {
    Discrete,
    Linear,
    Paced,
    Spline
  };

  /// @brief Animates one attribute of a target element for an animation element: an animated property's
  /// animated value, or a presentation attribute's animated style property.
  class SVGAttributeAnimator : public RefCounted<SVGAttributeAnimator>,
                               public CanMakeWeakPtr<SVGAttributeAnimator>
  {
  protected:
    dom::QualifiedName _attributeName;

  public:
    explicit SVGAttributeAnimator(const dom::QualifiedName &attributeName) noexcept
        : _attributeName(attributeName)
    {
    }

    virtual ~SVGAttributeAnimator() noexcept = default;

    KRYS_NODISCARD virtual bool IsDiscrete() const noexcept
    {
      return false;
    }

    virtual void SetFromAndToValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept
    {
    }

    virtual void SetFromAndByValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept
    {
    }

    virtual void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView) noexcept
    {
    }

    virtual void Start(SVGElement &) noexcept = 0;
    virtual void Animate(SVGElement &, float progress, uint32 repeatCount) noexcept = 0;
    virtual void Apply(SVGElement &) noexcept = 0;
    virtual void Stop(SVGElement &targetElement) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView,
                                                          dom::DOMStringView) const noexcept
    {
      return {};
    }

  protected:
    KRYS_NODISCARD bool IsAnimatedStylePropertyAnimator(const SVGElement &) const noexcept;

    static void InvalidateStyle(SVGElement &) noexcept;
    static void ApplyAnimatedStylePropertyChange(SVGElement &, css::PropertyId,
                                                 dom::DOMStringView value) noexcept;
    static void RemoveAnimatedStyleProperty(SVGElement &, css::PropertyId) noexcept;
    static void ApplyAnimatedPropertyChange(SVGElement &, const dom::QualifiedName &) noexcept;

    void ApplyAnimatedStylePropertyChange(SVGElement &, dom::DOMStringView value) noexcept;
    void RemoveAnimatedStyleProperty(SVGElement &) noexcept;
    void ApplyAnimatedPropertyChange(SVGElement &) noexcept;
  };
}
