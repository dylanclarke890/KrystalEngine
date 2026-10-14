#pragma once

#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-view-transition-class> = <custom-ident excluding=none>
  // https://drafts.csswg.org/css-view-transitions-2/#valdef-view-transition-class-custom-ident
  using ViewTransitionClass = ScopedName;

  // <view-transition-class-list> = <single-view-transition-class>+
  using ViewTransitionClassList = css::SpaceSeparatedFixedList<ViewTransitionClass>;

  // <'view-transition-class'> = none | <view-transition-class-list>
  // https://drafts.csswg.org/css-view-transitions-2/#propdef-view-transition-class
  struct ViewTransitionClasses : css::ListOrNone<ViewTransitionClassList>
  {
    using css::ListOrNone<ViewTransitionClassList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ViewTransitionClasses)
