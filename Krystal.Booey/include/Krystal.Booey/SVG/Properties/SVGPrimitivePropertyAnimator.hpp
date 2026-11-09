/*
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGPropertyAnimator.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute whose value is a primitive (a colour, number or string).
  template <typename TProperty, typename TAnimationFunction>
  class SVGPrimitivePropertyAnimator : public SVGPropertyAnimator<TAnimationFunction>
  {
    using Base = SVGPropertyAnimator<TAnimationFunction>;
    using ValuePropertyType = SVGValueProperty<TProperty>;
    using Base::_attributeName;
    using Base::_function;
    using Base::ApplyAnimatedStylePropertyChange;
    using Base::ComputeCSSPropertyValue;

  protected:
    const Ref<ValuePropertyType> _property;

  public:
    KRYS_NODISCARD static Ref<SVGPrimitivePropertyAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGPrimitivePropertyAnimator(attributeName, krys::move(property), animationMode,
                                                        calcMode, isAccumulated, isAdditive));
    }

    template <typename... TArgs>
    SVGPrimitivePropertyAnimator(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
                                 TArgs &&...arguments) noexcept
        : Base(attributeName, std::forward<TArgs>(arguments)...),
          _property(ShareRef(static_cast<ValuePropertyType &>(*property)))
    {
    }

    void Start(SVGElement &targetElement) noexcept override
    {
      dom::DOMString baseValue =
        ComputeCSSPropertyValue(targetElement, css::PropertyIdFromName(_attributeName.LocalName().View()));
      _property->SetValue(SVGPropertyTraits<TProperty>::FromString(targetElement, baseValue));
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept override
    {
      TProperty &animated = _property->Value();
      _function.Animate(targetElement, progress, repeatCount, animated);
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      ApplyAnimatedStylePropertyChange(targetElement,
                                       SVGPropertyTraits<TProperty>::ToString(_property->Value()));
    }
  };
}
