#pragma once

#include "Krystal.Booey/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.Core/Types/BoolOr.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#togglepopoveroptions
  struct TogglePopoverOptions : public ShowPopoverOptions
  {
    bool Force;
  };

  using TogglePopoverOptionsOrBool = BoolOr<TogglePopoverOptions>;
}