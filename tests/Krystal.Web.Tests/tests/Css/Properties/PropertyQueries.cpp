#include "Krystal.Web/Css/Properties/PropertyQueries.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::web::css::tests
{
  TEST_CASE("Property queries", "[CSS][Properties][PropertyQueries]")
  {
    SECTION("IsLogicalPropertyGroupLogicalProperty")
    {
      CHECK(IsLogicalPropertyGroupLogicalProperty(PropertyId::MarginInlineStart));
      CHECK_FALSE(IsLogicalPropertyGroupLogicalProperty(PropertyId::MarginLeft));
      CHECK_FALSE(IsLogicalPropertyGroupLogicalProperty(PropertyId::Color));
    }

    SECTION("IsListValuedProperty")
    {
      CHECK(IsListValuedProperty(PropertyId::BackgroundImage));
      CHECK(IsListValuedProperty(PropertyId::Custom));
      CHECK_FALSE(IsListValuedProperty(PropertyId::Color));
    }

    SECTION("IsSizingProperty")
    {
      CHECK(IsSizingProperty(PropertyId::Width));
      CHECK(IsSizingProperty(PropertyId::MaxHeight));
      CHECK(IsSizingProperty(PropertyId::MinInlineSize));
      CHECK_FALSE(IsSizingProperty(PropertyId::Margin));
    }
  }
}
