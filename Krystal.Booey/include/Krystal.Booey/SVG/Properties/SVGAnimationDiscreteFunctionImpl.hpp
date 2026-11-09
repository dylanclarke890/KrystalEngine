/*
 * Copyright (C) 2019-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGAnimationDiscreteFunction.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"

namespace krys::boo::svg
{
  class SVGAnimatedAngleOrientAnimator;

  class SVGAnimationBooleanFunction : public SVGAnimationDiscreteFunction<bool>
  {
  public:
    using Base = SVGAnimationDiscreteFunction<bool>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<bool>::FromString(targetElement, from);
      _to = SVGPropertyTraits<bool>::FromString(targetElement, to);
    }
  };

  template <typename TEnum>
  class SVGAnimationEnumerationFunction : public SVGAnimationDiscreteFunction<TEnum>
  {
    using Base = SVGAnimationDiscreteFunction<TEnum>;
    using Base::_from;
    using Base::_to;

  public:
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<TEnum>::FromString(targetElement, from);
      _to = SVGPropertyTraits<TEnum>::FromString(targetElement, to);
    }
  };

  class SVGAnimationOrientTypeFunction : public SVGAnimationDiscreteFunction<SVGMarkerOrientType>
  {
    friend class SVGAnimatedAngleOrientAnimator;

  public:
    using Base = SVGAnimationDiscreteFunction<SVGMarkerOrientType>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept override
    {
      // Values will be set by SVGAnimatedAngleOrientAnimator.
      krys_debug_assert_msg(false, "The orient type's values are set by its orient animator");
    }
  };

  class SVGAnimationPreserveAspectRatioFunction
      : public SVGAnimationDiscreteFunction<SVGPreserveAspectRatioValue>
  {
  public:
    using Base = SVGAnimationDiscreteFunction<SVGPreserveAspectRatioValue>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      _from = SVGPreserveAspectRatioValue(from);
      _to = SVGPreserveAspectRatioValue(to);
    }
  };

  class SVGAnimationStringFunction : public SVGAnimationDiscreteFunction<dom::DOMString>
  {
  public:
    using Base = SVGAnimationDiscreteFunction<dom::DOMString>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      _from = dom::DOMString(from);
      _to = dom::DOMString(to);
    }
  };
}
