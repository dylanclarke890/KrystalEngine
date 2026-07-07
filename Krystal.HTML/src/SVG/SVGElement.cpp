#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  SVGElement::SVGElement(Document &document, SVGElementInterface interface, SVGElementFlags flags,
                         NodeFlags nodeFlags) noexcept
      : Element(document, NullQualifiedName(), NodeFlags::IsSVGElement | nodeFlags), _flags(flags),
        _interface(interface)
  {
  }

  SVGElement::SVGElement(Document &document) noexcept : SVGElement(document, SVGElementInterface::None)
  {
  }
}