#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::svg
{
  class SVGScriptElement;
}

namespace krys::boo::html
{
  class HTMLScriptElement;

  /// @see https://html.spec.whatwg.org/#htmlorsvgscriptelement
  using HTMLOrSVGScriptElement = Variant<RefPtr<HTMLScriptElement>, RefPtr<svg::SVGScriptElement>>;
}