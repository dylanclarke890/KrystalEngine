#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"

namespace krys::boo::svg
{
  // The one instantiation of each SVG animated property type (see SVGAnimatedPropertyImpl.hpp).
  template class SVGSharedPrimitiveProperty<bool>;
  template class SVGSharedPrimitiveProperty<int32>;
  template class SVGSharedPrimitiveProperty<float>;
  template class SVGSharedPrimitiveProperty<dom::DOMString>;
  template class SVGAnimatedPrimitiveProperty<bool>;
  template class SVGAnimatedPrimitiveProperty<int32>;
  template class SVGAnimatedPrimitiveProperty<float>;
  template class SVGAnimatedPrimitiveProperty<dom::DOMString>;
  template class SVGAnimatedValueProperty<SVGAngle>;
  template class SVGAnimatedValueProperty<SVGLength>;
  template class SVGAnimatedValueProperty<SVGRect>;
  template class SVGAnimatedValueProperty<SVGPreserveAspectRatio>;
  template class SVGAnimatedPropertyList<SVGLengthList>;
  template class SVGAnimatedPropertyList<SVGNumberList>;
  template class SVGAnimatedPropertyList<SVGPointList>;
  template class SVGAnimatedPropertyList<SVGTransformList>;
  template class SVGAnimatedPropertyList<SVGPathSegList>;
}
