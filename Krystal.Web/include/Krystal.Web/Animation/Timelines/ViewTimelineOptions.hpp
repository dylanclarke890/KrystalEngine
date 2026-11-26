#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Animation/Timelines/ScrollAxis.hpp"
#include "Krystal.Web/Css/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Web/Css/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class Element;

  using ViewTimelineIndividualInset = Variant<RefPtr<CSSNumericValue>, DOMString, RefPtr<CSSKeywordValue>>;
  using ViewTimelineInsetValue = Variant<DOMString, List<ViewTimelineIndividualInset>>;

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-viewtimelineoptions
  struct ViewTimelineOptions
  {
    RefPtr<web::Element> Subject;
    ScrollAxis Axis {ScrollAxis::Block};
    ViewTimelineInsetValue Inset {DOMString(u8"auto")};
  };
}
