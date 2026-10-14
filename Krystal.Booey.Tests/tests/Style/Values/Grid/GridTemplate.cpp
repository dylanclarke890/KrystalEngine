#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateList.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizes.hpp"
#include "Krystal.Booey/Style/Values/Grid/ItemTolerance.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  namespace
  {
    using Names = List<css::CSSOMString>;
  }

  TEST_CASE_METHOD(BuilderFixture, "Style GridAutoFlow", "[Style][Values][Grid]")
  {
    CHECK(Convert<GridAutoFlow>(PropertyId::GridAutoFlow, u8"normal").GetDirection()
          == GridAutoFlow::Direction::Normal);

    auto columnDense = Convert<GridAutoFlow>(PropertyId::GridAutoFlow, u8"dense column");
    CHECK(columnDense.IsColumn());
    CHECK(columnDense.IsDense());
    CHECK(Serialised(columnDense) == "column dense");

    CHECK(Serialised(Convert<GridAutoFlow>(PropertyId::GridAutoFlow, u8"row")) == "row");
    CHECK(Convert<GridAutoFlow>(PropertyId::GridAutoFlow, u8"dense").IsSparse() == false);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style GridTrackSizes", "[Style][Values][Grid]")
  {
    CHECK(Serialised(Convert<GridTrackSizes>(PropertyId::GridAutoRows, u8"auto")) == "auto");
    CHECK(
      Serialised(Convert<GridTrackSizes>(PropertyId::GridAutoRows, u8"minmax(10px, 1fr) fit-content(20%)"))
      == "minmax(10px, 1fr) fit-content(20%)");

    GridTrackSize breadth {GridTrackBreadth {2_css_fr}};
    CHECK(breadth.IsBreadth());
    CHECK(breadth.MaxTrackBreadth().IsFlex());
    CHECK(Blending<GridTrackSize> {}.Blend(GridTrackSize {GridTrackBreadth {1_css_fr}},
                                           GridTrackSize {GridTrackBreadth {3_css_fr}}, BlendingContext {0.5})
          == breadth);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style GridTemplateList", "[Style][Values][Grid]")
  {
    CHECK(Convert<GridTemplateList>(PropertyId::GridTemplateColumns, u8"none").IsNone());

    SECTION("track list")
    {
      auto list =
        Convert<GridTemplateList>(PropertyId::GridTemplateColumns, u8"[a] 10px repeat(2, [b] 1fr) [c]");

      // Line name sets alternate with the track sections, starting and ending with a (possibly empty) set.
      REQUIRE(list.Tracks.size() == 5);
      CHECK(std::get<Names>(list.Tracks[0]) == Names {u8"a"});
      CHECK(std::holds_alternative<GridTrackSize>(list.Tracks[1]));
      CHECK(std::get<Names>(list.Tracks[2]).empty());
      const auto &repeat = std::get<GridTrackEntryRepeat>(list.Tracks[3]);
      CHECK(repeat.Repeats == 2);
      CHECK(repeat.Tracks.size() == 3);
      CHECK(std::get<Names>(list.Tracks[4]) == Names {u8"c"});

      CHECK(list.Sizes.size() == 3);
      CHECK(list.NamedLines.Map.at(u8"a") == List<uint32> {0});
      CHECK(list.NamedLines.Map.at(u8"b") == List<uint32> {1, 2});
      CHECK(list.NamedLines.Map.at(u8"c") == List<uint32> {3});
      CHECK(list.OrderedNamedLines.Map.at(0) == Names {u8"a"});
      CHECK_FALSE(list.Subgrid);
    }

    SECTION("auto repeat")
    {
      auto list =
        Convert<GridTemplateList>(PropertyId::GridTemplateRows, u8"20px repeat(auto-fill, [x] 5px)");
      CHECK(list.AutoRepeatType == AutoRepeatType::Fill);
      CHECK(list.AutoRepeatSizes.size() == 1);
      CHECK(list.AutoRepeatInsertionPoint == 1);
      CHECK(list.AutoRepeatNamedLines.Map.at(u8"x") == List<uint32> {0});
    }

    SECTION("subgrid")
    {
      auto list = Convert<GridTemplateList>(PropertyId::GridTemplateRows, u8"subgrid [a] [b]");
      CHECK(list.Subgrid);
      CHECK(list.Sizes.empty());
      CHECK(list.NamedLines.Map.at(u8"b") == List<uint32> {1});
    }

    SECTION("blending")
    {
      auto from = Convert<GridTemplateList>(PropertyId::GridTemplateColumns, u8"[a] 10px");
      auto to = Convert<GridTemplateList>(PropertyId::GridTemplateColumns, u8"[b] 30px");
      CHECK(Blending<GridTemplateList> {}.CanBlend(from, to));
      auto blended = Blending<GridTemplateList> {}.Blend(from, to, BlendingContext {0.75});
      CHECK(std::get<Names>(blended.Tracks[0]) == Names {u8"b"});
      CHECK(std::get<GridTrackSize>(blended.Tracks[1])
            == GridTrackSize {GridTrackBreadth {GridTrackBreadth::Fixed {25.0f}}});

      auto autoRepeat =
        Convert<GridTemplateList>(PropertyId::GridTemplateColumns, u8"repeat(auto-fit, 10px)");
      CHECK_FALSE(Blending<GridTemplateList> {}.CanBlend(autoRepeat, autoRepeat));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style GridTemplateAreas", "[Style][Values][Grid]")
  {
    CHECK(Convert<GridTemplateAreas>(PropertyId::GridTemplateAreas, u8"none").IsNone());

    auto areas = Convert<GridTemplateAreas>(PropertyId::GridTemplateAreas, u8"\"a a b\" \"c c b\"");
    CHECK(areas.Count(GridTrackSizingDirection::Rows) == 2);
    CHECK(areas.Count(GridTrackSizingDirection::Columns) == 3);
    CHECK(Serialised(areas) == "\"a a b\" \"c c b\"");

    const auto &columns = areas.ImplicitNamedGridLines(GridTrackSizingDirection::Columns);
    CHECK(columns.Map.at(u8"a-start") == List<uint32> {0});
    CHECK(columns.Map.at(u8"a-end") == List<uint32> {2});
    CHECK(columns.Map.at(u8"b-start") == List<uint32> {2});
    const auto &rows = areas.ImplicitNamedGridLines(GridTrackSizingDirection::Rows);
    CHECK(rows.Map.at(u8"b-end") == List<uint32> {2});
    CHECK(rows.Map.at(u8"c-start") == List<uint32> {1});

    // The implicit lines are derived from the map, also when it's moved in.
    GridNamedAreaMap map = areas.Map;
    GridTemplateAreas moved {krys::move(map)};
    CHECK(moved.ImplicitNamedGridColumnLines == areas.ImplicitNamedGridColumnLines);
  }
}
