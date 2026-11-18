#pragma once

#include "Krystal.Booey/Animation/Timelines/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class Element;

  using ViewTimelineIndividualInset = Variant<RefPtr<CSSNumericValue>, DOMString, RefPtr<CSSKeywordValue>>;
  using ViewTimelineInsetValue = Variant<DOMString, List<ViewTimelineIndividualInset>>;

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-viewtimelineoptions
  struct ViewTimelineOptions
  {
    RefPtr<boo::Element> Subject;
    ScrollAxis Axis {ScrollAxis::Block};
    ViewTimelineInsetValue Inset {DOMString(u8"auto")};
  };
}
