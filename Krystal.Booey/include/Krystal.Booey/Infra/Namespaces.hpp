#pragma once

#include "Krystal.Core/Strings/String.hpp"

namespace krys::boo::infra
{
  /// @see https://infra.spec.whatwg.org/#namespaces
  namespace namespaces
  {
    /// @see https://infra.spec.whatwg.org/#xml-namespace
    constexpr const char8 *XML = u8"http://www.w3.org/XML/1998/namespace";
    /// @see https://infra.spec.whatwg.org/#xmlns-namespace
    constexpr const char8 *XMLNS = u8"http://www.w3.org/2000/xmlns/";
    /// @see https://infra.spec.whatwg.org/#html-namespace
    constexpr const char8 *HTML = u8"http://www.w3.org/1999/xhtml";
    /// @see https://infra.spec.whatwg.org/#svg-namespace
    constexpr const char8 *SVG = u8"http://www.w3.org/2000/svg";
    /// @see https://infra.spec.whatwg.org/#mathml-namespace
    constexpr const char8 *MathML = u8"http://www.w3.org/1998/Math/MathML";
    /// @see https://infra.spec.whatwg.org/#xlink-namespace
    constexpr const char8 *XLink = u8"http://www.w3.org/1999/xlink";
  }

  namespace namespace_prefixes
  {
    constexpr const char8 *XML = u8"xml";
    constexpr const char8 *XMLNS = u8"xmlns";
    constexpr const char8 *HTML = u8"html";
    constexpr const char8 *SVG = u8"svg";
    constexpr const char8 *MathML = u8"mathml";
    constexpr const char8 *XLink = u8"xlink";
  }
}
