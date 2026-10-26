#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedDecoratedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPrimitiveProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyList.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedValueProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGDecoratedEnumeration.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"
#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatio.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"

namespace krys::boo::svg
{
  using SVGAnimatedBoolean = SVGAnimatedPrimitiveProperty<bool>;
  using SVGAnimatedInteger = SVGAnimatedPrimitiveProperty<int32>;
  using SVGAnimatedNumber = SVGAnimatedPrimitiveProperty<float>;
  using SVGAnimatedEnumeration = SVGAnimatedDecoratedProperty<SVGDecoratedEnumeration, uint32>;

  using SVGAnimatedAngle = SVGAnimatedValueProperty<SVGAngle>;
  using SVGAnimatedLength = SVGAnimatedValueProperty<SVGLength>;
  using SVGAnimatedRect = SVGAnimatedValueProperty<SVGRect>;
  using SVGAnimatedPreserveAspectRatio = SVGAnimatedValueProperty<SVGPreserveAspectRatio>;

  using SVGAnimatedLengthList = SVGAnimatedPropertyList<SVGLengthList>;
  using SVGAnimatedNumberList = SVGAnimatedPropertyList<SVGNumberList>;
  using SVGAnimatedPointList = SVGAnimatedPropertyList<SVGPointList>;
  using SVGAnimatedTransformList = SVGAnimatedPropertyList<SVGTransformList>;

  class SVGAnimatedOrientType : public SVGAnimatedEnumeration
  {
  public:
    using SVGAnimatedEnumeration::SVGAnimatedEnumeration;

    KRYS_NODISCARD static Ref<SVGAnimatedOrientType> Create(SVGElement *contextElement,
                                                            SVGMarkerOrientType baseValue) noexcept
    {
      return SVGAnimatedEnumeration::Create<SVGMarkerOrientType, SVGAnimatedOrientType>(contextElement,
                                                                                        baseValue);
    }
  };
}
