#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLScriptElement;
  class SVGScriptElement;

  /// @see https://html.spec.whatwg.org/#htmlorsvgscriptelement
  using HTMLOrSVGScriptElement = Variant<RefPtr<HTMLScriptElement>, RefPtr<SVGScriptElement>>;
}