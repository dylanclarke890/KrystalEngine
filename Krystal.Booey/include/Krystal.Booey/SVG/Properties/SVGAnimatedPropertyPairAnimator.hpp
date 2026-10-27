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
