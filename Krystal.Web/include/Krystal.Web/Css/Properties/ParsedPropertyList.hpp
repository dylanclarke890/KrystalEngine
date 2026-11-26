#pragma once

#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Web/Css/Properties/Property.hpp"

namespace krys::web::css
{
  using ParsedPropertyList = SmallList<Property, 63uz>;
}
