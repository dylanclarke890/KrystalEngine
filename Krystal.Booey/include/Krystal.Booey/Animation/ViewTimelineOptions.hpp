#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  using ViewTimelineIndividualInset =
    Variant<RefPtr<css::CSSNumericValue>, dom::DOMString, RefPtr<css::CSSKeywordValue>>;
  using ViewTimelineInsetValue = Variant<dom::DOMString, List<ViewTimelineIndividualInset>>;

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-viewtimelineoptions
  struct ViewTimelineOptions
  {
    RefPtr<dom::Element> Subject;
    ScrollAxis Axis {ScrollAxis::Block};
    ViewTimelineInsetValue Inset {dom::DOMString(u8"auto")};
  };
}
