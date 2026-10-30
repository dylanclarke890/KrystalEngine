#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAnimatorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyPairAnimator.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  class SVGAnimatedAngleOrientAnimator final
      : public SVGAnimatedPropertyPairAnimator<SVGAnimatedAngleAnimator, SVGAnimatedOrientTypeAnimator>
  {
    using Base = SVGAnimatedPropertyPairAnimator<SVGAnimatedAngleAnimator, SVGAnimatedOrientTypeAnimator>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedAngleOrientAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedAngle> &animated1,
             Ref<SVGAnimatedOrientType> &animated2, AnimationMode animationMode, CalcMode calcMode,
             bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedAngleOrientAnimator(attributeName, animated1, animated2, animationMode,
                                                          calcMode, isAccumulated, isAdditive));
    }

  private:
    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept final
    {
      auto pairFrom =
        SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>::FromString(targetElement, from);
      auto pairTo =
        SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>::FromString(targetElement, to);

      _animatedPropertyAnimator1->_function._from = pairFrom.first;
      _animatedPropertyAnimator1->_function._to = pairTo.first;

      _animatedPropertyAnimator2->_function._from = pairFrom.second;
      _animatedPropertyAnimator2->_function._to = pairTo.second;
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept final
    {
      SetFromAndToValues(targetElement, from, by);
      if (_animatedPropertyAnimator2->_function._from != SVGMarkerOrientAngle
          || _animatedPropertyAnimator2->_function._to != SVGMarkerOrientAngle)
      {
        return;
      }
      _animatedPropertyAnimator1->_function.AddFromAndToValues(targetElement);
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept final
    {
      if (_animatedPropertyAnimator2->_function._from != _animatedPropertyAnimator2->_function._to)
      {
        // Discrete animation - no linear interpolation possible between values (e.g. auto to angle).
        _animatedPropertyAnimator2->Animate(targetElement, progress, repeatCount);

        SVGAngleValue animatedAngle;
        if (progress < 0.5f && _animatedPropertyAnimator2->_function._from == SVGMarkerOrientAngle)
        {
          animatedAngle = _animatedPropertyAnimator1->_function._from;
        }
        else if (progress >= 0.5f && _animatedPropertyAnimator2->_function._to == SVGMarkerOrientAngle)
        {
          animatedAngle = _animatedPropertyAnimator1->_function._to;
        }

        _animatedPropertyAnimator1->_animated->SetAnimVal(animatedAngle);
        return;
      }

      if (_animatedPropertyAnimator2->_function._from == SVGMarkerOrientAngle)
      {
        // Regular from- toangle animation, with support for smooth interpolation, and additive and
        // accumulated animation.
        _animatedPropertyAnimator2->_animated->SetAnimVal(SVGMarkerOrientAngle);

        _animatedPropertyAnimator1->Animate(targetElement, progress, repeatCount);
        return;
      }

      // auto, auto-start-reverse, or unknown.
      _animatedPropertyAnimator1->_animated->AnimVal()->Value().SetValue(0);

      if (_animatedPropertyAnimator2->_function._from == SVGMarkerOrientAuto
          || _animatedPropertyAnimator2->_function._from == SVGMarkerOrientAutoStartReverse)
      {
        _animatedPropertyAnimator2->_animated->SetAnimVal(_animatedPropertyAnimator2->_function._from);
      }
      else
      {
        _animatedPropertyAnimator2->_animated->SetAnimVal(SVGMarkerOrientUnknown);
      }
    }

    void Stop(SVGElement &targetElement) noexcept final
    {
      if (!_animatedPropertyAnimator1->_animated->IsAnimating())
      {
        return;
      }
      Apply(targetElement);
      Base::Stop(targetElement);
    }
  };

  class SVGAnimatedIntegerPairAnimator final
      : public SVGAnimatedPropertyPairAnimator<SVGAnimatedIntegerAnimator, SVGAnimatedIntegerAnimator>
  {
    using Base = SVGAnimatedPropertyPairAnimator<SVGAnimatedIntegerAnimator, SVGAnimatedIntegerAnimator>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedIntegerPairAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedInteger> &animated1,
             Ref<SVGAnimatedInteger> &animated2, AnimationMode animationMode, CalcMode calcMode,
             bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedIntegerPairAnimator(attributeName, animated1, animated2, animationMode,
                                                          calcMode, isAccumulated, isAdditive));
    }

  private:
    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept final
    {
      auto pairFrom = SVGPropertyTraits<std::pair<int32, int32>>::FromString(targetElement, from);
      auto pairTo = SVGPropertyTraits<std::pair<int32, int32>>::FromString(targetElement, to);

      _animatedPropertyAnimator1->_function._from = pairFrom.first;
      _animatedPropertyAnimator1->_function._to = pairTo.first;

      _animatedPropertyAnimator2->_function._from = pairFrom.second;
      _animatedPropertyAnimator2->_function._to = pairTo.second;
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept final
    {
      auto pairFrom = SVGPropertyTraits<std::pair<int32, int32>>::FromString(targetElement, from);
      auto pairBy = SVGPropertyTraits<std::pair<int32, int32>>::FromString(targetElement, by);

      _animatedPropertyAnimator1->_function._from = pairFrom.first;
      _animatedPropertyAnimator1->_function._to = pairFrom.first + pairBy.first;

      _animatedPropertyAnimator2->_function._from = pairFrom.second;
      _animatedPropertyAnimator2->_function._to = pairFrom.second + pairBy.second;
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept final
    {
      auto pairToAtEndOfDuration =
        SVGPropertyTraits<std::pair<int32, int32>>::FromString(targetElement, toAtEndOfDuration);
      _animatedPropertyAnimator1->_function._toAtEndOfDuration = pairToAtEndOfDuration.first;
      _animatedPropertyAnimator2->_function._toAtEndOfDuration = pairToAtEndOfDuration.second;
    }
  };

  class SVGAnimatedNumberPairAnimator final
      : public SVGAnimatedPropertyPairAnimator<SVGAnimatedNumberAnimator, SVGAnimatedNumberAnimator>
  {
    using Base = SVGAnimatedPropertyPairAnimator<SVGAnimatedNumberAnimator, SVGAnimatedNumberAnimator>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedNumberPairAnimator>
      Create(const dom::QualifiedName &attributeName, Ref<SVGAnimatedNumber> &animated1,
             Ref<SVGAnimatedNumber> &animated2, AnimationMode animationMode, CalcMode calcMode,
             bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGAnimatedNumberPairAnimator(attributeName, animated1, animated2, animationMode,
                                                         calcMode, isAccumulated, isAdditive));
    }

  private:
    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept final
    {
      auto pairFrom = SVGPropertyTraits<std::pair<float, float>>::FromString(targetElement, from);
      auto pairTo = SVGPropertyTraits<std::pair<float, float>>::FromString(targetElement, to);

      _animatedPropertyAnimator1->_function._from = pairFrom.first;
      _animatedPropertyAnimator1->_function._to = pairTo.first;

      _animatedPropertyAnimator2->_function._from = pairFrom.second;
      _animatedPropertyAnimator2->_function._to = pairTo.second;
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept final
    {
      auto pairFrom = SVGPropertyTraits<std::pair<float, float>>::FromString(targetElement, from);
      auto pairBy = SVGPropertyTraits<std::pair<float, float>>::FromString(targetElement, by);

      _animatedPropertyAnimator1->_function._from = pairFrom.first;
      _animatedPropertyAnimator1->_function._to = pairFrom.first + pairBy.first;

      _animatedPropertyAnimator2->_function._from = pairFrom.second;
      _animatedPropertyAnimator2->_function._to = pairFrom.second + pairBy.second;
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept final
    {
      auto pairToAtEndOfDuration =
        SVGPropertyTraits<std::pair<float, float>>::FromString(targetElement, toAtEndOfDuration);
      _animatedPropertyAnimator1->_function._toAtEndOfDuration = pairToAtEndOfDuration.first;
      _animatedPropertyAnimator2->_function._toAtEndOfDuration = pairToAtEndOfDuration.second;
    }
  };
}
