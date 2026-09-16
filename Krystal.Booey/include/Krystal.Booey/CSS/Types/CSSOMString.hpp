#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/cssom/#cssomstring-type
  using CSSOMString = dom::DOMString;

  /// @see https://drafts.csswg.org/cssom/#cssomstring-type
  using CSSOMStringView = dom::DOMStringView;

  /// @see https://drafts.csswg.org/cssom/#cssomstring-type
  using CSSOMStringAtom = dom::DOMStringAtom;
}