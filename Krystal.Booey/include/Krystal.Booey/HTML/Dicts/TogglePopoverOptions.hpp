#pragma once

#include "Krystal.Booey/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.Core/Types/BoolOr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#togglepopoveroptions
  struct TogglePopoverOptions : public ShowPopoverOptions
  {
    Maybe<bool> Force;
  };

  using TogglePopoverOptionsOrBool = BoolOr<TogglePopoverOptions>;
}