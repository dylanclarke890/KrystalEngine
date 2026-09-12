#pragma once

#include "Krystal.Booey/CSS/Values/Background/BoxShadow.hpp"

namespace krys::boo::css
{
  // <`box-shadow`> = none | <box-shadow>#
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-box-shadow
  using BoxShadowProperty = ListOrNone<CommaSeparatedList<BoxShadow>>;
}