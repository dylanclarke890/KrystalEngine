#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Align/AlignSelf.hpp"
#include "Krystal.Booey/Style/Values/Align/JustifySelf.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/AnchorName.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionAnchor.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionArea.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbacks.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionVisibility.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    /// @brief Recreates the fixture's builder state as if building the style for a fallback with `tactics`.
    void UseFallback(BuilderFixture &fixture, List<PositionTryFallbackTactic> tactics) noexcept
    {
      fixture.PositionTryFallback = BuilderPositionTryFallback {nullptr, krys::move(tactics)};
      fixture.ResetState();
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style anchor names", "[Style][Values][AnchorPosition]")
  {
    Build(PropertyId::AnchorName, u8"none");
    CHECK(Style.GetAnchorNames().IsNone());
    Build(PropertyId::AnchorName, u8"--a, --b");
    CHECK(Serialised(Style.GetAnchorNames()) == "--a, --b");

    Build(PropertyId::PositionAnchor, u8"auto");
    CHECK(Style.GetPositionAnchor().IsAuto());
    Build(PropertyId::PositionAnchor, u8"--a");
    auto anchor = Style.GetPositionAnchor();
    CHECK(anchor.IsName());
    CHECK(Serialised(anchor) == "--a");
    CHECK(Created(anchor) == "--a");

    CHECK(Convert<PositionVisibility>(PropertyId::PositionVisibility, u8"always").IsAlways());
    CHECK(
      Serialised(Convert<PositionVisibility>(PropertyId::PositionVisibility, u8"anchors-visible no-overflow"))
      == "anchors-visible no-overflow");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style PositionArea", "[Style][Values][AnchorPosition]")
  {
    CHECK(Convert<PositionArea>(PropertyId::PositionArea, u8"none").IsNone());

    auto area = Convert<PositionArea>(PropertyId::PositionArea, u8"top left");
    REQUIRE(area.IsValue());
    CHECK(area.TryValue()->BlockOrXAxis().Track() == PositionAreaTrack::Start);
    // The computed value puts the x axis first.
    CHECK(Serialised(area) == "left top");
    CHECK(Created(area) == "left top");

    CHECK(Serialised(Convert<PositionArea>(PropertyId::PositionArea, u8"span-all center"))
          == "span-all center");
    CHECK(Serialised(Convert<PositionArea>(PropertyId::PositionArea, u8"start")) == "start");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style PositionTryFallbacks", "[Style][Values][AnchorPosition]")
  {
    Build(PropertyId::PositionTryFallbacks, u8"none");
    CHECK(Style.GetPositionTryFallbacks().IsNone());

    Build(PropertyId::PositionTryFallbacks, u8"--foo flip-block, flip-x flip-y, left");
    auto fallbacks = Style.GetPositionTryFallbacks();
    CHECK(Serialised(fallbacks) == "--foo flip-block, flip-x flip-y, left");
    CHECK(Created(fallbacks) == "--foo flip-block, flip-x flip-y, left");

    REQUIRE(fallbacks.size() == 3);
    const auto &rule = fallbacks[0];
    REQUIRE(rule.IsRuleAndTactics());
    CHECK(rule.RuleAndTacticsFallback.Rule);
    REQUIRE(rule.RuleAndTacticsFallback.Tactics);
    CHECK(rule.RuleAndTacticsFallback.Tactics->Value[0] == PositionTryFallbackTactic::FlipBlock);

    CHECK_FALSE(fallbacks[1].RuleAndTacticsFallback.Rule);
    CHECK(fallbacks[2].IsPositionArea());
    CHECK((fallbacks[2] == fallbacks[2]));
    CHECK_FALSE((fallbacks[0] == fallbacks[2]));
  }

  TEST_CASE_METHOD(BuilderFixture, "Style self-alignment under position-try fallbacks",
                   "[Style][Values][AnchorPosition]")
  {
    auto alignSelf = [&](utf8_stringview value)
    {
      Build(PropertyId::AlignSelf, value);
      return Serialised(Style.GetAlignSelf());
    };
    auto justifySelf = [&](utf8_stringview value)
    {
      Build(PropertyId::JustifySelf, value);
      return Serialised(Style.GetJustifySelf());
    };

    CHECK(alignSelf(u8"start") == "start");

    UseFallback(*this, {PositionTryFallbackTactic::FlipBlock});
    CHECK(alignSelf(u8"start") == "end");
    CHECK(alignSelf(u8"safe self-end") == "safe self-start");
    CHECK(alignSelf(u8"center") == "center");
    // `flip-block` leaves the inline axis alone.
    CHECK(justifySelf(u8"left") == "left");

    // In a horizontal writing mode, `flip-x` flips the inline axis.
    UseFallback(*this, {PositionTryFallbackTactic::FlipX});
    CHECK(justifySelf(u8"left") == "right");
    CHECK(alignSelf(u8"start") == "start");

    // `flip-start` swaps the axes: `justify-self` applies to `align-self`, its physical positions resolved to
    // logical ones first.
    UseFallback(*this, {PositionTryFallbackTactic::FlipStart});
    Build(PropertyId::JustifySelf, u8"left");
    CHECK(Serialised(Style.GetAlignSelf()) == "self-start");
    CHECK(Serialised(Style.GetJustifySelf()) == "right");
  }
}
