#pragma once

#include "Krystal.HTML/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.Lib/Types/BoolOr.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#togglepopoveroptions
  struct TogglePopoverOptions : public ShowPopoverOptions
  {
    bool Force;
  };

  using TogglePopoverOptionsOrBool = BoolOr<TogglePopoverOptions>;
}