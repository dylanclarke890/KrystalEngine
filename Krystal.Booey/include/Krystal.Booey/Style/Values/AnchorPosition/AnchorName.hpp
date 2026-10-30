#pragma once

#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-anchor-name> = <custom-ident excluding=none>
  // https://drafts.csswg.org/css-anchor-position-1/#valdef-anchor-name-dashed-ident
  using AnchorName = ScopedName;

  // <anchor-name-list> = <single-anchor-name>#
  using AnchorNameList = css::CommaSeparatedFixedList<AnchorName>;

  // <'anchor-name'> = none | <anchor-name-list>
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-anchor-name
  struct AnchorNames : css::ListOrNone<AnchorNameList>
  {
    using css::ListOrNone<AnchorNameList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AnchorNames)
