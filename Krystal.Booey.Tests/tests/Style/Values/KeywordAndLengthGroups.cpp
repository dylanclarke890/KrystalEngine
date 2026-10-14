#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Break/Orphans.hpp"
#include "Krystal.Booey/Style/Values/Break/Widows.hpp"
#include "Krystal.Booey/Style/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/Style/Values/Contain/Contain.hpp"
#include "Krystal.Booey/Style/Values/Contain/ContainerName.hpp"
#include "Krystal.Booey/Style/Values/CounterStyles/CounterStyle.hpp"
#include "Krystal.Booey/Style/Values/CSS2/ZIndex.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Display/Order.hpp"
#include "Krystal.Booey/Style/Values/FillStroke/StrokeMiterlimit.hpp"
#include "Krystal.Booey/Style/Values/FillStroke/StrokeWidth.hpp"
#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Booey/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Booey/Style/Values/Multicol/ColumnCount.hpp"
#include "Krystal.Booey/Style/Values/Multicol/ColumnWidth.hpp"
#include "Krystal.Booey/Style/Values/Overflow/BlockEllipsis.hpp"
#include "Krystal.Booey/Style/Values/Overflow/MaximumLines.hpp"
#include "Krystal.Booey/Style/Values/Overflow/ScrollbarGutter.hpp"
#include "Krystal.Booey/Style/Values/Overflow/ScrollBehavior.hpp"
#include "Krystal.Booey/Style/Values/Page/PageSize.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Rhythm/BlockStepSize.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarColor.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Booey/Style/Values/Speech/SpeakAs.hpp"
#include "Krystal.Booey/Style/Values/Viewport/Zoom.hpp"
#include "Krystal.Booey/Style/Values/ViewTransitions/ViewTransitionClass.hpp"
#include "Krystal.Booey/Style/Values/ViewTransitions/ViewTransitionName.hpp"
#include "Krystal.Booey/Style/Values/WillChange/WillChange.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using css::ValueId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style integers and their keywords",
                   "[Style][Values][KeywordAndLengthGroups]")
  {
    SECTION("z-index")
    {
      CHECK(Convert<ZIndex>(PropertyId::ZIndex, u8"auto").IsAuto());
      CHECK(Convert<ZIndex>(PropertyId::ZIndex, u8"-3").TryValue() == ZIndex::Value {-3});
      CHECK(Serialised(ZIndex {css::keywords::Auto {}}) == "auto");
      CHECK(Serialised(ZIndex {7}) == "7");
      CHECK_FALSE(CanBlend(ZIndex {css::keywords::Auto {}}, ZIndex {1}));
      CHECK(Blending<ZIndex> {}.Blend(ZIndex {0}, ZIndex {10}, BlendingContext {0.5}) == ZIndex {5});
    }

    SECTION("order, orphans, widows and column-count")
    {
      CHECK(Convert<Order>(PropertyId::Order, u8"-2").Value == -2);
      CHECK(Convert<Orphans>(PropertyId::Orphans, u8"3").TryInteger() == Orphans::Integer {3});
      CHECK(Convert<Widows>(PropertyId::Widows, u8"4").IsInteger());
      CHECK(Convert<ColumnCount>(PropertyId::ColumnCount, u8"auto").IsAuto());
      CHECK(Convert<ColumnCount>(PropertyId::ColumnCount, u8"3") == ColumnCount {ColumnCount::Value {3}});
      CHECK(Blending<ColumnCount> {}.Blend(ColumnCount {ColumnCount::Value {2}}, ColumnCount {ColumnCount::Value {4}}, BlendingContext {0.5})
            == ColumnCount {ColumnCount::Value {3}});
    }

    SECTION("math-depth")
    {
      CHECK(Convert<MathDepth>(PropertyId::MathDepth, u8"4") == MathDepth {4});

      ParentStyle.SetMathDepth(MathDepth {2});
      CHECK(Convert<MathDepth>(PropertyId::MathDepth, u8"auto-add") == MathDepth {2});
      ParentStyle.SetMathStyle(MathStyle::Compact);
      CHECK(Convert<MathDepth>(PropertyId::MathDepth, u8"auto-add") == MathDepth {3});
      CHECK(Convert<MathDepth>(PropertyId::MathDepth, u8"add(3)") == MathDepth {5});
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style lengths with keywords", "[Style][Values][KeywordAndLengthGroups]")
  {
    SECTION("column-width")
    {
      CHECK(Convert<ColumnWidth>(PropertyId::ColumnWidth, u8"auto").IsAuto());
      CHECK(Convert<ColumnWidth>(PropertyId::ColumnWidth, u8"2em").TryLength() == ColumnWidth::Length {32});
    }

    SECTION("block-step-size")
    {
      CHECK(Convert<BlockStepSize>(PropertyId::BlockStepSize, u8"none").IsNone());
      CHECK(Convert<BlockStepSize>(PropertyId::BlockStepSize, u8"10px").TryLength()
            == BlockStepSize::Length {10});
    }

    SECTION("stroke-width converts numbers to pixels")
    {
      CHECK(Serialised(Convert<StrokeWidth>(PropertyId::StrokeWidth, u8"2")) == "2px");
      CHECK(Serialised(Convert<StrokeWidth>(PropertyId::StrokeWidth, u8"5%")) == "5%");
      CHECK(Convert<StrokeMiterlimit>(PropertyId::StrokeMiterlimit, u8"6").Value.Value == 6.f);
    }

    SECTION("page size")
    {
      auto a4 = ConvertCSSValueToStyle<PageSize>(*State, *css::PrimitiveValue::Create(ValueId::A4));
      CHECK(Serialised(a4) == "793.700806px 1122.519653px");

      auto landscape = ConvertCSSValueToStyle<PageSize>(
        *State, *css::ValuePair::Create(css::PrimitiveValue::Create(ValueId::Letter),
                                        css::PrimitiveValue::Create(ValueId::Landscape)));
      CHECK(Serialised(landscape) == "1056px 816px");

      auto lengths = ConvertCSSValueToStyle<PageSize>(
        *State, *css::ValuePair::Create(css::PrimitiveValue::Create(1, css::UnitType::in),
                                        css::PrimitiveValue::Create(2, css::UnitType::in)));
      CHECK(Serialised(lengths) == "96px 192px");

      CHECK(
        Serialised(ConvertCSSValueToStyle<PageSize>(*State, *css::PrimitiveValue::Create(ValueId::Portrait)))
        == "portrait");
      CHECK_FALSE(IsInvalid());

      std::ignore = ConvertCSSValueToStyle<PageSize>(*State, *css::PrimitiveValue::Create(ValueId::Red));
      CHECK(IsInvalid());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style keyword sets", "[Style][Values][KeywordAndLengthGroups]")
  {
    SECTION("contain")
    {
      CHECK(Convert<Contain>(PropertyId::Contain, u8"none").IsNone());
      auto strict = Convert<Contain>(PropertyId::Contain, u8"strict");
      CHECK(strict.ContainsAll(
        {ContainValue::Size, ContainValue::Layout, ContainValue::Paint, ContainValue::Style}));
      CHECK(Serialised(strict) == "strict");
      CHECK(Serialised(Convert<Contain>(PropertyId::Contain, u8"content")) == "content");
      auto list = Convert<Contain>(PropertyId::Contain, u8"inline-size layout");
      CHECK(list.ContainsOnly({ContainValue::InlineSize, ContainValue::Layout}));
      CHECK(Serialised(list) == "inline-size layout");
    }

    SECTION("speak-as")
    {
      CHECK(Convert<SpeakAs>(PropertyId::SpeakAs, u8"normal").IsNormal());
      auto speakAs = Convert<SpeakAs>(PropertyId::SpeakAs, u8"spell-out no-punctuation");
      CHECK(speakAs.ContainsOnly({SpeakAsValue::SpellOut, SpeakAsValue::NoPunctuation}));
      CHECK(Serialised(speakAs) == "spell-out no-punctuation");
    }

    SECTION("touch-action")
    {
      CHECK(Convert<TouchAction>(PropertyId::TouchAction, u8"auto").IsAuto());
      CHECK(Convert<TouchAction>(PropertyId::TouchAction, u8"manipulation").IsManipulation());
      auto pans = Convert<TouchAction>(PropertyId::TouchAction, u8"pan-x pinch-zoom");
      REQUIRE(pans.IsEnumSet());
      CHECK(Serialised(pans) == "pan-x pinch-zoom");
      CHECK(ConvertToPlatform(pans)
            == EnumSet<boo::TouchAction> {boo::TouchAction::PanX, boo::TouchAction::PinchZoom});
      CHECK(ConvertToPlatform(TouchAction {css::keywords::None {}})
            == EnumSet<boo::TouchAction> {boo::TouchAction::None});
    }

    SECTION("will-change")
    {
      CHECK(Convert<WillChange>(PropertyId::WillChange, u8"auto").IsAuto());

      auto scroll = Convert<WillChange>(PropertyId::WillChange, u8"scroll-position");
      CHECK(scroll.ContainsScrollPosition());
      CHECK_FALSE(scroll.CanCreateStackingContext());

      auto properties = Convert<WillChange>(PropertyId::WillChange, u8"transform, opacity, contents");
      CHECK(properties.ContainsProperty(PropertyId::Transform));
      CHECK(properties.ContainsProperty(PropertyId::Opacity));
      CHECK(properties.ContainsContents());
      CHECK(properties.CanCreateStackingContext());
      CHECK(properties.CanTriggerCompositing());
      CHECK(properties.CanTriggerCompositingOnInline());
      CHECK(properties.CreatesContainingBlockForOutOfFlowPositioned(false));
      CHECK(Serialised(properties) == "transform, opacity, contents");

      CHECK(properties == Convert<WillChange>(PropertyId::WillChange, u8"transform, opacity, contents"));
      CHECK_FALSE(properties == scroll);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style identifiers and strings", "[Style][Values][KeywordAndLengthGroups]")
  {
    SECTION("list-style-type")
    {
      auto disc = Convert<ListStyleType>(PropertyId::ListStyleType, u8"disc");
      CHECK(disc.IsCounterStyle());
      CHECK(disc.IsDisc());
      CHECK(disc == ListStyleType {css::keywords::Disc {}});
      CHECK(Convert<ListStyleType>(PropertyId::ListStyleType, u8"none").IsNone());

      auto custom = Convert<ListStyleType>(PropertyId::ListStyleType, u8"my-style");
      REQUIRE(custom.TryCounterStyle());
      CHECK(*custom.TryCounterStyle() == css::CSSOMStringAtom {u8"my-style"});

      auto string = Convert<ListStyleType>(PropertyId::ListStyleType, u8"\"-\"");
      CHECK(string.TryString() == css::CSSOMStringAtom {u8"-"});
      CHECK(Created(string) == "\"-\"");
      CHECK(Created(disc) == "disc");
      CHECK(Created(ListStyleType {css::keywords::None {}}) == "none");
    }

    SECTION("counter styles")
    {
      auto counter =
        ConvertCSSValueToStyle<CounterStyle>(*State, *css::PrimitiveValue::Create(ValueId::Decimal));
      CHECK(counter == ValueId::Decimal);
    }

    SECTION("block-ellipsis")
    {
      CHECK(Convert<BlockEllipsis>(PropertyId::BlockEllipsis, u8"none").IsNone());
      CHECK(Convert<BlockEllipsis>(PropertyId::BlockEllipsis, u8"auto").IsAuto());
      auto string = Convert<BlockEllipsis>(PropertyId::BlockEllipsis, u8"\"...\"");
      CHECK(string.IsString());
      CHECK(Serialised(string) == "\"...\"");
      CHECK(Convert<MaximumLines>(PropertyId::MaxLines, u8"none").IsNone());
      CHECK(Convert<MaximumLines>(PropertyId::MaxLines, u8"3") == MaximumLines {MaximumLines::Value {3}});
    }

    SECTION("view transitions")
    {
      CurrentProperty.StyleScopeOrdinal = ScopeOrdinal::Shadow;
      CHECK(Convert<ViewTransitionName>(PropertyId::ViewTransitionName, u8"none").IsNone());
      auto named = Convert<ViewTransitionName>(PropertyId::ViewTransitionName, u8"hero");
      CHECK(named.IsCustomIdentifier());
      CHECK(named.GetScopeOrdinal() == ScopeOrdinal::Shadow);
      CHECK(Serialised(named) == "hero");
      CHECK(Convert<ViewTransitionName>(PropertyId::ViewTransitionName, u8"auto").IsAuto());
      CHECK(Convert<ViewTransitionName>(PropertyId::ViewTransitionName, u8"match-element").IsMatchElement());
    }

    SECTION("color-scheme")
    {
      CHECK(Convert<ColorScheme>(PropertyId::ColorScheme, u8"normal").IsNormal());
      auto scheme = Convert<ColorScheme>(PropertyId::ColorScheme, u8"light dark");
      CHECK(scheme.GetColorScheme()
            == EnumSet<boo::ColorScheme> {boo::ColorScheme::Light, boo::ColorScheme::Dark});
      CHECK(Serialised(scheme) == "light dark");
      auto only = Convert<ColorScheme>(PropertyId::ColorScheme, u8"dark only");
      CHECK(only.Only);
      CHECK(only.GetColorScheme() == EnumSet<boo::ColorScheme> {boo::ColorScheme::Dark});
      CHECK(Serialised(only) == "dark only");
      CHECK(Created(only) == "dark only");
      CHECK(Serialised(ColorScheme {css::keywords::Normal {}}) == "normal");
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style scrolling values", "[Style][Values][KeywordAndLengthGroups]")
  {
    CHECK(Convert<ScrollbarWidth>(PropertyId::ScrollbarWidth, u8"thin") == ScrollbarWidth::Thin);
    CHECK(Serialised(ScrollbarWidth::None) == "none");
    CHECK(Convert<ScrollBehavior>(PropertyId::ScrollBehavior, u8"smooth") == ScrollBehavior::Smooth);

    CHECK(Convert<ScrollbarGutter>(PropertyId::ScrollbarGutter, u8"auto").IsAuto());
    CHECK(Convert<ScrollbarGutter>(PropertyId::ScrollbarGutter, u8"stable").IsStable());
    auto bothEdges = Convert<ScrollbarGutter>(PropertyId::ScrollbarGutter, u8"stable both-edges");
    CHECK(bothEdges.IsStableBothEdges());
    CHECK(Serialised(bothEdges) == "stable both-edges");

    CHECK(Convert<ScrollbarColor>(PropertyId::ScrollbarColor, u8"auto").IsAuto());
    auto colors = Convert<ScrollbarColor>(PropertyId::ScrollbarColor, u8"red blue");
    REQUIRE(colors.IsParts());
    CHECK(colors.TryValue()->Thumb == Color {boo::Color::red});
    CHECK(colors.TryValue()->Track == Color {boo::Color::blue});
    CHECK(Serialised(colors) == "rgb(255, 0, 0) rgb(0, 0, 255)");
    CHECK_FALSE(IsInvalid());
  }

  TEST_CASE("Style Zoom", "[Style][Values][KeywordAndLengthGroups]")
  {
    CHECK(Zoom {1.5f} == 1.5f);
    CHECK(Zoom {0.f}.IsZero());
    CHECK(Zoom {50_css_percentage} == 0.5f);
  }
}
