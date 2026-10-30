#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief Animates an element's animated property (and its instances' in `<use>` trees) with
  /// @p TAnimationFunction.
  template <typename TAnimatedProperty, typename TAnimationFunction>
  class SVGAnimatedPropertyAnimator : public SVGAttributeAnimator
  {
  protected:
    const Ref<TAnimatedProperty> _animated;
    List<Ref<TAnimatedProperty>> _animatedInstances;
    TAnimationFunction _function;

  public:
    using AnimatorAnimatedProperty = TAnimatedProperty;

    template <typename... TArgs>
    SVGAnimatedPropertyAnimator(const dom::QualifiedName &attributeName, Ref<TAnimatedProperty> &animated,
                                TArgs &&...arguments) noexcept
        : SVGAttributeAnimator(attributeName), _animated(animated),
          _function(std::forward<TArgs>(arguments)...)
    {
    }

    void AppendAnimatedInstance(Ref<TAnimatedProperty> &animated) noexcept
    {
      _animatedInstances.push_back(animated);
    }

    KRYS_NODISCARD bool IsDiscrete() const noexcept override
    {
      return _function.IsDiscrete();
    }

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _function.SetFromAndToValues(targetElement, from, to);
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

    void Start(SVGElement &) noexcept override
    {
      _animated->StartAnimation(*this);
      for (auto &instance : _animatedInstances)
      {
        instance->InstanceStartAnimation(*this, *_animated);
      }
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      if (IsAnimatedStylePropertyAnimator(targetElement))
      {
        ApplyAnimatedStylePropertyChange(targetElement, _animated->AnimValAsString());
      }
      ApplyAnimatedPropertyChange(targetElement);
    }

    void Stop(SVGElement &targetElement) noexcept override
    {
      if (!_animated->IsAnimating())
      {
        return;
      }

      _animated->StopAnimation(*this);
      for (auto &instance : _animatedInstances)
      {
        instance->InstanceStopAnimation(*this);
      }

      ApplyAnimatedPropertyChange(targetElement);
      if (IsAnimatedStylePropertyAnimator(targetElement))
      {
        RemoveAnimatedStyleProperty(targetElement);
      }
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &targetElement, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override
    {
      return _function.CalculateDistance(targetElement, from, to);
    }
  };
}
