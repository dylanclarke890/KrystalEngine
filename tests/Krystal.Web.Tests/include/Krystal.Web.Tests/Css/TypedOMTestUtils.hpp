#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include <string>

namespace krys::web::css::tests
{
  /// @brief @p value serialised, as a `std::string`.
  KRYS_NODISCARD inline std::string StyleValueText(const CSSStyleValue &value)
  {
    return ToNarrow(value.ToString());
  }

  /// @brief The value @p result holds, which it must.
  template <typename T>
  KRYS_NODISCARD Ref<T> RequireValue(ExceptionOr<Ref<T>> result)
  {
    REQUIRE(result.HasValue());
    return result.ReleaseValue();
  }
}
