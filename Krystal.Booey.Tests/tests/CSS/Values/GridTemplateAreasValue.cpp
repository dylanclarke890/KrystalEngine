#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD GridNamedAreaMap MapFromRows(std::initializer_list<GridNamedAreaMapRow> rows)
    {
      GridNamedAreaMap map;
      for (const auto &row : rows)
      {
        REQUIRE(AddRow(map, row));
      }
      return map;
    }

    KRYS_NODISCARD Ref<GridTemplateAreasValue> Areas(std::initializer_list<GridNamedAreaMapRow> rows)
    {
      return GridTemplateAreasValue::Create(GridTemplateAreas {MapFromRows(rows)});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridTemplateAreasValue::Create", "[CSS][Values][GridTemplateAreasValue]")
  {
    auto map = MapFromRows({{u8"head", u8"head"}, {u8"nav", u8"main"}});
    auto value = GridTemplateAreasValue::Create(GridTemplateAreas {map});
    CHECK(value->IsGridTemplateAreasValue());
    CHECK(value->Areas() == GridTemplateAreas {map});
    CHECK(value->Areas().Map.RowCount == 2);
    CHECK(value->Areas().Map.ColumnCount == 2);
  }

  TEST_CASE_METHOD(ValueFixture, "GridTemplateAreasValue serialisation",
                   "[CSS][Values][GridTemplateAreasValue]")
  {
    CHECK(CssText(*Areas({{u8"head", u8"head"}, {u8"nav", u8"."}})) == "\"head head\" \"nav .\"");
    CHECK(CssText(*Areas({{u8"a"}})) == "\"a\"");
  }

  TEST_CASE_METHOD(ValueFixture, "GridTemplateAreasValue::StringForRow",
                   "[CSS][Values][GridTemplateAreasValue]")
  {
    auto value = Areas({{u8"a", u8"a", u8"b"}, {u8"a", u8"a", u8"."}, {u8"c", u8"c", u8"c"}});
    CHECK(ToNarrow(value->StringForRow(0uz)) == "a a b");
    // Unnamed cells are written as `.`.
    CHECK(ToNarrow(value->StringForRow(1uz)) == "a a .");
    CHECK(ToNarrow(value->StringForRow(2uz)) == "c c c");
  }

  TEST_CASE_METHOD(ValueFixture, "GridTemplateAreasValue equality", "[CSS][Values][GridTemplateAreasValue]")
  {
    auto a = Areas({{u8"a", u8"b"}});
    CHECK(*a == *Areas({{u8"a", u8"b"}}));
    CHECK_FALSE(*a == *Areas({{u8"b", u8"a"}}));
    CHECK_FALSE(*a == *Areas({{u8"a", u8"b"}, {u8"a", u8"b"}}));
  }

  TEST_CASE_METHOD(ValueFixture, "GridTemplateAreasValue has no children",
                   "[CSS][Values][GridTemplateAreasValue]")
  {
    CHECK(Children(*Areas({{u8"a", u8"b"}})).empty());
  }
}
