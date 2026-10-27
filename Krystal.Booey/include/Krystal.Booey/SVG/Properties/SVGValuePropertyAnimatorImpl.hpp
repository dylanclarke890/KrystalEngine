#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyAnimator.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"

namespace krys::boo::svg
{
  class SVGLengthAnimator final : public SVGValuePropertyAnimator<SVGLength, SVGAnimationLengthFunction>
  {
    using Base = SVGValuePropertyAnimator<SVGLength, SVGAnimationLengthFunction>;
    using Base::_attributeName;
    using Base::_property;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGLengthAnimator> Create(const dom::QualifiedName &attributeName,
                                                        Ref<SVGProperty> &&property,
                                                        AnimationMode animationMode, CalcMode calcMode,
                                                        bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGLengthAnimator(attributeName, krys::move(property), animationMode, calcMode,
                                             isAccumulated, isAdditive, SVGLengthMode::Other));
    }

    void Start(SVGElement &targetElement) noexcept override
    {
      dom::DOMString baseValue =
        ComputeCSSPropertyValue(targetElement, css::PropertyIdFromName(_attributeName.LocalName().View()));
      SVGLengthValue value(SVGLengthMode::Other);
      if (!value.SetValueAsString(baseValue).HasException())
      {
        _property->SetValue(value);
      }
    }
  };
}
