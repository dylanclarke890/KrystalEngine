#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"

namespace Krys::HTML::MIMEType
{
  constexpr inline DOMStringView TextHTML = u8"text/html";
  constexpr inline DOMStringView ApplicationXHTMLXML = u8"application/xhtml+xml";
  constexpr inline DOMStringView ImageSVGXML = u8"image/svg+xml";
  constexpr inline DOMStringView ApplicationMathMLXML = u8"application/mathml+xml";
}