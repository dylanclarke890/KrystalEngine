#pragma once

#include "Krystal.HTML/CSS/Values/Background/BoxShadow.hpp"

namespace Krys::HTML
{
  // <`box-shadow`> = none | <box-shadow>#
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-box-shadow
  using BoxShadowProperty = ListOrNone<CommaSeparatedList<BoxShadow>>;
}