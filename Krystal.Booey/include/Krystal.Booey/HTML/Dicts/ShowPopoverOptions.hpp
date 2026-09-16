#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLElement;

  /// @see https://html.spec.whatwg.org/multipage/dom.html#showpopoveroptions
  struct ShowPopoverOptions
  {
    RefPtr<HTMLElement> Source;
  };
}