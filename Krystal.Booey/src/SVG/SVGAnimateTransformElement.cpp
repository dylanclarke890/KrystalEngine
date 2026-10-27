#include "Krystal.Booey/SVG/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGTransformable.hpp"

namespace krys::boo::svg
{
  SVGAnimateTransformElement::SVGAnimateTransformElement(dom::Document &document) noexcept
      : SVGAnimateElementBase(document, SVGElementInterface::AnimateTransform)
  {
  }

  bool SVGAnimateTransformElement::HasValidAttributeType() const noexcept
  {
    if (!TargetElement())
    {
      return false;
    }

    if (GetAttributeType() == AttributeType::CSS)
    {
      return false;
    }

    return SVGAnimateElementBase::HasValidAttributeType();
  }

  void SVGAnimateTransformElement::AttributeChanged(const dom::QualifiedName &name,
                                                    Maybe<dom::DOMStringView> oldValue,
                                                    Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::TypeAttr())
    {
      _type = SVGTransformable::ParseTransformTypeFromString(value.value_or(dom::DOMStringView {}))
                .value_or(SVGTransformValue::SVG_TRANSFORM_UNKNOWN);
      if (_type == SVGTransformValue::SVG_TRANSFORM_MATRIX)
      {
        _type = SVGTransformValue::SVG_TRANSFORM_UNKNOWN;
      }
    }

    SVGAnimateElementBase::AttributeChanged(name, oldValue, value);
  }

  dom::DOMString SVGAnimateTransformElement::AnimateRangeString(dom::DOMStringView string) const noexcept
  {
    dom::DOMString result {SVGTransformValue::PrefixForTransformType(_type)};
    result += string;
    result += u8')';
    return result;
  }
}
