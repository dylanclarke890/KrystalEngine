#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPropertyAnimator.hpp"

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute whose value is a value property (a length).
  template <typename TProperty, typename TAnimationFunction>
  class SVGValuePropertyAnimator : public SVGPropertyAnimator<TAnimationFunction>
  {
    using Base = SVGPropertyAnimator<TAnimationFunction>;
    using Base::_function;
    using Base::ApplyAnimatedStylePropertyChange;

  protected:
    using Base::_attributeName;
    using Base::ComputeCSSPropertyValue;

    const Ref<TProperty> _property;

  public:
    template <typename... TArgs>
    SVGValuePropertyAnimator(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
                             TArgs &&...arguments) noexcept
        : Base(attributeName, std::forward<TArgs>(arguments)...),
          _property(ShareRef(static_cast<TProperty &>(*property)))
    {
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept override
    {
      _function.Animate(targetElement, progress, repeatCount, _property->Value());
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      ApplyAnimatedStylePropertyChange(targetElement, _property->ValueAsString());
    }
  };
}
