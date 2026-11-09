/*
 * Copyright (C) 2018-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief Animates a pair of animated properties that reflect one attribute (`orient`, `order`,
  /// `stdDeviation`, ...).
  template <typename TAnimatedPropertyAnimator1, typename TAnimatedPropertyAnimator2>
  class SVGAnimatedPropertyPairAnimator : public SVGAttributeAnimator
  {
  public:
    using AnimatedProperty1 = typename TAnimatedPropertyAnimator1::AnimatorAnimatedProperty;
    using AnimatedProperty2 = typename TAnimatedPropertyAnimator2::AnimatorAnimatedProperty;

  protected:
    const Ref<TAnimatedPropertyAnimator1> _animatedPropertyAnimator1;
    const Ref<TAnimatedPropertyAnimator2> _animatedPropertyAnimator2;

  public:
    SVGAnimatedPropertyPairAnimator(const dom::QualifiedName &attributeName,
                                    Ref<AnimatedProperty1> &animated1, Ref<AnimatedProperty2> &animated2,
                                    AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                    bool isAdditive) noexcept
        : SVGAttributeAnimator(attributeName),
          _animatedPropertyAnimator1(TAnimatedPropertyAnimator1::Create(
            attributeName, animated1, animationMode, calcMode, isAccumulated, isAdditive)),
          _animatedPropertyAnimator2(TAnimatedPropertyAnimator2::Create(
            attributeName, animated2, animationMode, calcMode, isAccumulated, isAdditive))
    {
    }

    void AppendAnimatedInstance(Ref<AnimatedProperty1> &animated1, Ref<AnimatedProperty2> &animated2) noexcept
    {
      _animatedPropertyAnimator1->AppendAnimatedInstance(animated1);
      _animatedPropertyAnimator2->AppendAnimatedInstance(animated2);
    }

  protected:
    void Start(SVGElement &targetElement) noexcept override
    {
      _animatedPropertyAnimator1->Start(targetElement);
      _animatedPropertyAnimator2->Start(targetElement);
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept override
    {
      _animatedPropertyAnimator1->Animate(targetElement, progress, repeatCount);
      _animatedPropertyAnimator2->Animate(targetElement, progress, repeatCount);
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      ApplyAnimatedPropertyChange(targetElement);
    }

    void Stop(SVGElement &targetElement) noexcept override
    {
      _animatedPropertyAnimator1->Stop(targetElement);
      _animatedPropertyAnimator2->Stop(targetElement);
    }
  };
}
