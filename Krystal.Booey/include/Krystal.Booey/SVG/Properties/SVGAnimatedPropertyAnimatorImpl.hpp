/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAnimator.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveListFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimationDiscreteFunctionImpl.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  class SVGAnimatedAngleAnimator;
  class SVGAnimatedIntegerPairAnimator;
  class SVGAnimatedOrientTypeAnimator;

  template <typename TAnimatedPropertyAnimator1, typename TAnimatedPropertyAnimator2>
  class SVGAnimatedPropertyPairAnimator;

  class SVGAnimatedAngleAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedAngle, SVGAnimationAngleFunction>
  {
    friend class SVGAnimatedPropertyPairAnimator<SVGAnimatedAngleAnimator, SVGAnimatedOrientTypeAnimator>;
    friend class SVGAnimatedAngleOrientAnimator;
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedAngle, SVGAnimationAngleFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedAngleAnimator> Create(const dom::QualifiedName &attributeName,
                                                               Ref<SVGAnimatedAngle> &animated,
                                                               AnimationMode animationMode, CalcMode calcMode,
                                                               bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedAngleAnimator(attributeName, animated, animationMode, calcMode,
                                                    isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal()->Value());
    }
  };

  class SVGAnimatedBooleanAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedBoolean, SVGAnimationBooleanFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedBoolean, SVGAnimationBooleanFunction>;

  public:
    using Base::Base;

    KRYS_NODISCARD static Ref<SVGAnimatedBooleanAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedBoolean> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedBooleanAnimator(attributeName, animated, animationMode, calcMode,
                                                      isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      bool &animated = _animated->AnimVal();
      _function.Animate(targetElement, progress, repeatCount, animated);
    }
  };

  template <typename TEnum>
  class SVGAnimatedEnumerationAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedEnumeration, SVGAnimationEnumerationFunction<TEnum>>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedEnumeration, SVGAnimationEnumerationFunction<TEnum>>;
    using Base::_animated;
    using Base::_function;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedEnumerationAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedEnumeration> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedEnumerationAnimator<TEnum>(attributeName, animated, animationMode,
                                                                 calcMode, isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      TEnum animated {};
      _function.Animate(targetElement, progress, repeatCount, animated);
      _animated->template SetAnimVal<TEnum>(animated);
    }
  };

  class SVGAnimatedIntegerAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedInteger, SVGAnimationIntegerFunction>
  {
    friend class SVGAnimatedPropertyPairAnimator<SVGAnimatedIntegerAnimator, SVGAnimatedIntegerAnimator>;
    friend class SVGAnimatedIntegerPairAnimator;
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedInteger, SVGAnimationIntegerFunction>;

  public:
    using Base::Base;

    KRYS_NODISCARD static Ref<SVGAnimatedIntegerAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedInteger> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedIntegerAnimator(attributeName, animated, animationMode, calcMode,
                                                      isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };

  class SVGAnimatedLengthAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedLength, SVGAnimationLengthFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedLength, SVGAnimationLengthFunction>;

  public:
    SVGAnimatedLengthAnimator(const dom::QualifiedName &attributeName, Ref<SVGAnimatedLength> &animated,
                              AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                              bool isAdditive, SVGLengthMode lengthMode) noexcept
        : Base(attributeName, animated, animationMode, calcMode, isAccumulated, isAdditive, lengthMode)
    {
    }

    KRYS_NODISCARD static Ref<SVGAnimatedLengthAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedLength> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive,
             SVGLengthMode lengthMode) noexcept
    {
      return AdoptRef(*new SVGAnimatedLengthAnimator(attributeName, animated, animationMode, calcMode,
                                                     isAccumulated, isAdditive, lengthMode));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal()->Value());
    }
  };

  class SVGAnimatedLengthListAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedLengthList, SVGAnimationLengthListFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedLengthList, SVGAnimationLengthListFunction>;

  public:
    SVGAnimatedLengthListAnimator(const dom::QualifiedName &attributeName,
                                  Ref<SVGAnimatedLengthList> &animated, AnimationMode animationMode,
                                  CalcMode calcMode, bool isAccumulated, bool isAdditive,
                                  SVGLengthMode lengthMode) noexcept
        : Base(attributeName, animated, animationMode, calcMode, isAccumulated, isAdditive, lengthMode)
    {
    }

    KRYS_NODISCARD static Ref<SVGAnimatedLengthListAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedLengthList> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive,
             SVGLengthMode lengthMode) noexcept
    {
      return AdoptRef(*new SVGAnimatedLengthListAnimator(attributeName, animated, animationMode, calcMode,
                                                         isAccumulated, isAdditive, lengthMode));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };

  class SVGAnimatedNumberAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedNumber, SVGAnimationNumberFunction>
  {
    friend class SVGAnimatedPropertyPairAnimator<SVGAnimatedNumberAnimator, SVGAnimatedNumberAnimator>;
    friend class SVGAnimatedNumberPairAnimator;
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedNumber, SVGAnimationNumberFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedNumberAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedNumber> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedNumberAnimator(attributeName, animated, animationMode, calcMode,
                                                     isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };

  class SVGAnimatedNumberListAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedNumberList, SVGAnimationNumberListFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedNumberList, SVGAnimationNumberListFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedNumberListAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedNumberList> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedNumberListAnimator(attributeName, animated, animationMode, calcMode,
                                                         isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };

  class SVGAnimatedPathSegListAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedPathSegList, SVGAnimationPathSegListFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedPathSegList, SVGAnimationPathSegListFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedPathSegListAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedPathSegList> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedPathSegListAnimator(attributeName, animated, animationMode, calcMode,
                                                          isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _animated->AnimVal()->PathByteStreamWillChange();
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal()->PathByteStream());
    }
  };

  class SVGAnimatedPointListAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedPointList, SVGAnimationPointListFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedPointList, SVGAnimationPointListFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedPointListAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedPointList> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedPointListAnimator(attributeName, animated, animationMode, calcMode,
                                                        isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };

  class SVGAnimatedOrientTypeAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedOrientType, SVGAnimationOrientTypeFunction>
  {
    friend class SVGAnimatedPropertyPairAnimator<SVGAnimatedAngleAnimator, SVGAnimatedOrientTypeAnimator>;
    friend class SVGAnimatedAngleOrientAnimator;
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedOrientType, SVGAnimationOrientTypeFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedOrientTypeAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedOrientType> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedOrientTypeAnimator(attributeName, animated, animationMode, calcMode,
                                                         isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      SVGMarkerOrientType animated {};
      _function.Animate(targetElement, progress, repeatCount, animated);
      _animated->SetAnimVal(animated);
    }
  };

  class SVGAnimatedPreserveAspectRatioAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedPreserveAspectRatio,
                                           SVGAnimationPreserveAspectRatioFunction>
  {
    using Base =
      SVGAnimatedPropertyAnimator<SVGAnimatedPreserveAspectRatio, SVGAnimationPreserveAspectRatioFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedPreserveAspectRatioAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedPreserveAspectRatio> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedPreserveAspectRatioAnimator(attributeName, animated, animationMode,
                                                                  calcMode, isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      SVGPreserveAspectRatioValue &animated = _animated->AnimVal()->Value();
      _function.Animate(targetElement, progress, repeatCount, animated);
    }
  };

  class SVGAnimatedRectAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedRect, SVGAnimationRectFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedRect, SVGAnimationRectFunction>;

  public:
    using Base::Base;

    KRYS_NODISCARD static Ref<SVGAnimatedRectAnimator> Create(const dom::QualifiedName &attributeName,
                                                              Ref<SVGAnimatedRect> &animated,
                                                              AnimationMode animationMode, CalcMode calcMode,
                                                              bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedRectAnimator(attributeName, animated, animationMode, calcMode,
                                                   isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal()->Value());
    }
  };

  class SVGAnimatedStringAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedString, SVGAnimationStringFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedString, SVGAnimationStringFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedStringAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedString> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedStringAnimator(attributeName, animated, animationMode, calcMode,
                                                     isAccumulated, isAdditive));
    }

  private:
    KRYS_NODISCARD bool IsAnimatedStyleClassAnimator() const noexcept
    {
      return _attributeName.Matches(SVGNames::ClassAttr());
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      dom::DOMString &animated = _animated->AnimVal();
      _function.Animate(targetElement, progress, repeatCount, animated);
    }

    void Apply(SVGElement &targetElement) noexcept final
    {
      Base::Apply(targetElement);
      if (IsAnimatedStyleClassAnimator())
      {
        InvalidateStyle(targetElement);
      }
    }

    void Stop(SVGElement &targetElement) noexcept final
    {
      if (!_animated->IsAnimating())
      {
        return;
      }

      Base::Stop(targetElement);
      if (IsAnimatedStyleClassAnimator())
      {
        InvalidateStyle(targetElement);
      }
    }
  };

  class SVGAnimatedTransformListAnimator final
      : public SVGAnimatedPropertyAnimator<SVGAnimatedTransformList, SVGAnimationTransformListFunction>
  {
    using Base = SVGAnimatedPropertyAnimator<SVGAnimatedTransformList, SVGAnimationTransformListFunction>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedTransformListAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedTransformList> &animated,
             AnimationMode animationMode, CalcMode calcMode, bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedTransformListAnimator(attributeName, animated, animationMode, calcMode,
                                                            isAccumulated, isAdditive));
    }

  private:
    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      _function.Animate(targetElement, progress, repeatCount, _animated->AnimVal());
    }
  };
}
