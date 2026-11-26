#pragma once

#include "Krystal.Booey.Tests/Dom/DOMTestUtils.hpp"
#include "Krystal.Booey/Svg/Elements/SVGElement.hpp"

namespace krys::boo::svg::tests
{
  /// @brief The SVG element of @p document with the id @p id, which must exist and be a @p T.
  template <typename T>
  KRYS_NODISCARD T &ById(Document &document, DOMStringView id) noexcept
  {
    auto &element = Downcast<SVGElement>(boo::tests::ById(document, id));
    if constexpr (SameType<T, SVGElement>)
    {
      return element;
    }
    else
    {
      return Downcast<T>(element);
    }
  }
}
