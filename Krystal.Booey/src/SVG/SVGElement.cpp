#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  SVGElement::SVGElement(dom::Document &document, SVGElementInterface interface, SVGElementFlags flags,
                         dom::NodeFlags nodeFlags) noexcept
      : Element(document, dom::NullQualifiedName(), dom::NodeFlags::IsSVGElement | nodeFlags), _flags(flags),
        _interface(interface)
  {
  }

  SVGElement::SVGElement(dom::Document &document) noexcept : SVGElement(document, SVGElementInterface::None)
  {
  }
}