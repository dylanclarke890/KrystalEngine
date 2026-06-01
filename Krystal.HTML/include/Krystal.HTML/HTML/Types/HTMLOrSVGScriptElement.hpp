#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class HTMLScriptElement;
  class SVGScriptElement;

  /// @see https://html.spec.whatwg.org/#htmlorsvgscriptelement
  using HTMLOrSVGScriptElement = Variant<RefPtr<HTMLScriptElement>, RefPtr<SVGScriptElement>>;
}