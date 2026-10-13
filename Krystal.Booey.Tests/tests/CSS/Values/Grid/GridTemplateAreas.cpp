#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Maybe<GridNamedAreaMap> MapFromRows(std::initializer_list<GridNamedAreaMapRow> rows)
    {
      GridNamedAreaMap map;
      for (const auto &row : rows)
      {
        if (!AddRow(map, row))
        {
          return {};
        }
      }
      return map;
    }

    CSSOMString SerialiseForTest(const auto &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }
  }

  TEST_CASE("GridNamedAreaMap(AddRow)", "[CSS][Grid][GridNamedAreaMap]")
  {
    SECTION("areas")
    {
      auto map = MapFromRows({{u8"a", u8"a", u8"b"}, {u8"a", u8"a", u8"."}, {u8"c", u8"c", u8"c"}});
      REQUIRE(map);
      CHECK(map->RowCount == 3);
      CHECK(map->ColumnCount == 3);
      CHECK(map->Map.size() == 3);

      CHECK(
        map->Map.at(u8"a")
        == GridArea {GridSpan::TranslatedDefiniteGridSpan(0, 2), GridSpan::TranslatedDefiniteGridSpan(0, 2)});
      CHECK(
        map->Map.at(u8"b")
        == GridArea {GridSpan::TranslatedDefiniteGridSpan(0, 1), GridSpan::TranslatedDefiniteGridSpan(2, 3)});
      CHECK(
        map->Map.at(u8"c")
        == GridArea {GridSpan::TranslatedDefiniteGridSpan(2, 3), GridSpan::TranslatedDefiniteGridSpan(0, 3)});
    }

    SECTION("empty row")
    {
      GridNamedAreaMap map;
      CHECK_FALSE(AddRow(map, {}));
    }

    SECTION("rows of different lengths")
    {
      CHECK_FALSE(MapFromRows({{u8"a", u8"b"}, {u8"a"}}));
    }

    SECTION("non-rectangular areas")
    {
      // Not adjacent to the previous row.
      CHECK_FALSE(MapFromRows({{u8"a"}, {u8"b"}, {u8"a"}}));
      // Starts in a different column.
      CHECK_FALSE(MapFromRows({{u8"a", u8"a"}, {u8"b", u8"a"}}));
      // Ends in a different column.
      CHECK_FALSE(MapFromRows({{u8"a", u8"b"}, {u8"a", u8"a"}}));
      // Split within a row.
      CHECK_FALSE(MapFromRows({{u8"a", u8"b", u8"a"}}));
    }
  }

  TEST_CASE("GridTemplateAreas", "[CSS][Grid][GridTemplateAreas]")
  {
    GridTemplateAreas none {keywords::None {}};
    CHECK(none.IsNone());
    CHECK(SerialiseForTest(none) == u8"none");

    auto map = MapFromRows({{u8"head", u8"head"}, {u8"nav", u8"."}});
    REQUIRE(map);
    GridTemplateAreas areas {*map};
    CHECK_FALSE(areas.IsNone());
    CHECK(SerialiseForTest(areas) == u8"\"head head\" \"nav .\"");

    CHECK(areas == GridTemplateAreas {*map});
    CHECK(areas != none);

    auto isNone =
      areas.SwitchOn([](keywords::None) { return true; }, [](const GridNamedAreaMap &) { return false; });
    CHECK_FALSE(isNone);
  }
}
