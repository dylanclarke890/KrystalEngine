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
      fixture.State.reset();
      fixture.State = CreateUnique<BuilderState>(
        fixture.Style,
        BuilderContext {.Document = fixture.Document,
                        .ParentStyle = &fixture.ParentStyle,
                        .PositionTryFallback = BuilderPositionTryFallback {nullptr, krys::move(tactics)}});
      fixture.State->SetCurrentProperty(&fixture.CurrentProperty);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style anchor names", "[Style][Values][AnchorPosition]")
  {
    CHECK(Convert<AnchorNames>(PropertyId::AnchorName, u8"none").IsNone());
    CHECK(Serialised(Convert<AnchorNames>(PropertyId::AnchorName, u8"--a, --b")) == "--a, --b");

    CHECK(Convert<PositionAnchor>(PropertyId::PositionAnchor, u8"auto").IsAuto());
    auto anchor = Convert<PositionAnchor>(PropertyId::PositionAnchor, u8"--a");
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
    CHECK(Convert<PositionTryFallbacks>(PropertyId::PositionTryFallbacks, u8"none").IsNone());

    auto fallbacks = Convert<PositionTryFallbacks>(PropertyId::PositionTryFallbacks,
                                                   u8"--foo flip-block, flip-x flip-y, left");
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
    CHECK(Serialised(Convert<AlignSelf>(PropertyId::AlignSelf, u8"start")) == "start");

    UseFallback(*this, {PositionTryFallbackTactic::FlipBlock});
    CHECK(Serialised(Convert<AlignSelf>(PropertyId::AlignSelf, u8"start")) == "end");
    CHECK(Serialised(Convert<AlignSelf>(PropertyId::AlignSelf, u8"safe self-end")) == "safe self-start");
    CHECK(Serialised(Convert<AlignSelf>(PropertyId::AlignSelf, u8"center")) == "center");
    // `flip-block` leaves the inline axis alone.
    CHECK(Serialised(Convert<JustifySelf>(PropertyId::JustifySelf, u8"left")) == "left");

    // In a horizontal writing mode, `flip-x` flips the inline axis.
    UseFallback(*this, {PositionTryFallbackTactic::FlipX});
    CHECK(Serialised(Convert<JustifySelf>(PropertyId::JustifySelf, u8"left")) == "right");
    CHECK(Serialised(Convert<AlignSelf>(PropertyId::AlignSelf, u8"start")) == "start");

    // `flip-start` resolves physical positions to logical ones, ready to be swapped across axes.
    UseFallback(*this, {PositionTryFallbackTactic::FlipStart});
    CHECK(Serialised(Convert<JustifySelf>(PropertyId::JustifySelf, u8"left")) == "self-start");
  }
}
