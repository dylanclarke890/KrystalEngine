#pragma once

#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-container-name> = <custom-ident excluding=none,and,or,not>
  // https://www.w3.org/TR/css-contain-3/#valdef-container-name-custom-ident
  using ContainerName = ScopedName;

  // <container-name-list> = <single-container-name>+
  using ContainerNameList = css::SpaceSeparatedFixedList<ContainerName>;

  // <'container-name'> = none | <container-name-list>
  // https://www.w3.org/TR/css-contain-3/#propdef-container-name
  struct ContainerNames : css::ListOrNone<ContainerNameList>
  {
    using css::ListOrNone<ContainerNameList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ContainerNames)
