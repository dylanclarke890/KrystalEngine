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
