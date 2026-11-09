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

#include "Krystal.Booey/SVG/Properties/SVGAnimationFunction.hpp"

namespace krys::boo::svg
{
  /// @brief An animation function that interpolates, and can add to the underlying value (`additive="sum"`)
  /// and accumulate over repeats (`accumulate="sum"`).
  class SVGAnimationAdditiveFunction : public SVGAnimationFunction
  {
  protected:
    CalcMode _calcMode;
    bool _isAccumulated;
    bool _isAdditive;

  public:
    SVGAnimationAdditiveFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                 bool isAdditive) noexcept
        : SVGAnimationFunction(animationMode), _calcMode(calcMode), _isAccumulated(isAccumulated),
          _isAdditive(isAdditive)
    {
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept override
    {
      SetFromAndToValues(targetElement, from, by);
      AddFromAndToValues(targetElement);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView) noexcept override
    {
      krys_debug_assert_msg(false, "Additive functions set the value at the end of the duration themselves");
    }

  protected:
    KRYS_NODISCARD float Animate(float progress, uint32 repeatCount, float from, float to,
                                 float toAtEndOfDuration, float animated) const noexcept
    {
      float number;
      if (_calcMode == CalcMode::Discrete)
      {
        number = progress < 0.5 ? from : to;
      }
      else
      {
        number = (to - from) * progress + from;
      }

      if (_isAccumulated && repeatCount)
      {
        number += toAtEndOfDuration * static_cast<float>(repeatCount);
      }

      if (_isAdditive && _animationMode != AnimationMode::To)
      {
        number += animated;
      }

      return number;
    }
  };
}
