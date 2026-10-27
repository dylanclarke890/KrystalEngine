#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"

namespace krys::boo::svg
{
  Maybe<float> SVGAnimationColorFunction::CalculateDistance(SVGElement &targetElement,
                                                            dom::DOMStringView from,
                                                            dom::DOMStringView to) const noexcept
  {
    auto simpleFrom =
      SVGPropertyTraits<Color>::FromString(targetElement, from).ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    auto simpleTo =
      SVGPropertyTraits<Color>::FromString(targetElement, to).ToColorTypeLossy<SRGBA<uint8>>().Resolved();

    float red = static_cast<float>(simpleFrom.Red - simpleTo.Red);
    float green = static_cast<float>(simpleFrom.Green - simpleTo.Green);
    float blue = static_cast<float>(simpleFrom.Blue - simpleTo.Blue);

    return std::hypot(red, green, blue);
  }

  Maybe<float> SVGAnimationIntegerFunction::CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                              dom::DOMStringView to) const noexcept
  {
    return static_cast<float>(
      std::abs(text::ParseInteger<int32>(to).value_or(0) - text::ParseInteger<int32>(from).value_or(0)));
  }

  Maybe<float> SVGAnimationNumberFunction::CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                             dom::DOMStringView to) const noexcept
  {
    return std::abs(ParseNumberFromString(to).value_or(0) - ParseNumberFromString(from).value_or(0));
  }
}
