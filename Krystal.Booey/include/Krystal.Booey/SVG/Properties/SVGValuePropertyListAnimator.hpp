#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPropertyAnimator.hpp"

namespace krys::boo::svg
{
  /// @brief Animates a presentation attribute whose value is a list (`stroke-dasharray`).
  template <typename TList, typename TAnimationFunction>
  class SVGValuePropertyListAnimator : public SVGPropertyAnimator<TAnimationFunction>
  {
    using Base = SVGPropertyAnimator<TAnimationFunction>;
    using Base::_function;
    using Base::ApplyAnimatedStylePropertyChange;

  protected:
    using Base::_attributeName;
    using Base::ComputeCSSPropertyValue;

    RefPtr<TList> _list;

  public:
    template <typename... TArgs>
    SVGValuePropertyListAnimator(const dom::QualifiedName &attributeName, Ref<SVGProperty> &&property,
                                 TArgs &&...arguments) noexcept
        : Base(attributeName, std::forward<TArgs>(arguments)...),
          _list(ShareRef(static_cast<TList &>(*property)))
    {
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount) noexcept override
    {
      _function.Animate(targetElement, progress, repeatCount, _list);
    }

    void Apply(SVGElement &targetElement) noexcept override
    {
      ApplyAnimatedStylePropertyChange(targetElement, _list->ValueAsString());
    }
  };
}
