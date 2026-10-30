#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveListFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyListAnimator.hpp"

namespace krys::boo::svg
{
  class SVGLengthListAnimator final
      : public SVGValuePropertyListAnimator<SVGLengthList, SVGAnimationLengthListFunction>
  {
    using Base = SVGValuePropertyListAnimator<SVGLengthList, SVGAnimationLengthListFunction>;
    using Base::_attributeName;
    using Base::_list;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGLengthListAnimator> Create(const dom::QualifiedName &attributeName,
                                                            Ref<SVGProperty> &&property,
                                                            AnimationMode animationMode, CalcMode calcMode,
                                                            bool isAccumulated, bool isAdditive) noexcept
    {
      return AdoptRef(*new SVGLengthListAnimator(attributeName, krys::move(property), animationMode, calcMode,
                                                 isAccumulated, isAdditive, SVGLengthMode::Other));
    }

    void Start(SVGElement &targetElement) noexcept final
    {
      dom::DOMString baseValue =
        ComputeCSSPropertyValue(targetElement, css::PropertyIdFromName(_attributeName.LocalName().View()));
      if (!_list->Parse(baseValue))
      {
        _list->ClearItems();
      }
    }
  };
}
