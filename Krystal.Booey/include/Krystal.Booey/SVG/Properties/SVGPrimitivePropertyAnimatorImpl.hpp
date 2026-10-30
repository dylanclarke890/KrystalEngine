#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimationDiscreteFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPrimitivePropertyAnimator.hpp"

namespace krys::boo::svg
{
  using SVGColorAnimator = SVGPrimitivePropertyAnimator<Color, SVGAnimationColorFunction>;
  using SVGNumberAnimator = SVGPrimitivePropertyAnimator<float, SVGAnimationNumberFunction>;
  using SVGStringAnimator = SVGPrimitivePropertyAnimator<dom::DOMString, SVGAnimationStringFunction>;
}
