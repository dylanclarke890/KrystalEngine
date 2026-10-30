#pragma once

#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"

namespace krys::boo::css
{
  // <`text-shadow`> = none | <text-shadow>#
  // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
  using TextShadowProperty = ListOrNone<CommaSeparatedList<TextShadow>>;
}
