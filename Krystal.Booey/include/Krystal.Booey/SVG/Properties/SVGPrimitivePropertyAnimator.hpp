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
