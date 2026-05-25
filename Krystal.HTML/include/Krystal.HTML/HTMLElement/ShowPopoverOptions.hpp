#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class HTMLElement;

  /// @see https://html.spec.whatwg.org/multipage/dom.html#showpopoveroptions
  struct ShowPopoverOptions
  {
    RefPtr<HTMLElement> Source;
  };
}