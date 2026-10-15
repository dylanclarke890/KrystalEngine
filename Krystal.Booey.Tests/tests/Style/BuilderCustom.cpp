#include "Krystal.Booey/Style/BuilderCustom.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/BuilderGenerated.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    /// @brief Applies property values through the generated builder, which calls the custom functions.
    struct BuilderCustomFixture : BuilderFixture
    {
      void Apply(PropertyId property, utf8_stringview value) noexcept
      {
        auto parsed = Parse(property, value);
        Apply(property, *parsed, ApplyValueType::Value);
      }

      void Apply(PropertyId property, css::Value &value, ApplyValueType valueType) noexcept
      {
        BuilderGenerated::ApplyProperty(property, *State, value, valueType);
      }

      void ApplyInitial(PropertyId property) noexcept
      {
        auto value = css::PrimitiveValue::Create(css::ValueId::Initial);
        Apply(property, *value, ApplyValueType::Initial);
      }

      void ApplyInherit(PropertyId property) noexcept
      {
        auto value = css::PrimitiveValue::Create(css::ValueId::Inherit);
        Apply(property, *value, ApplyValueType::Inherit);
      }

      /// @brief Builds the style from author declarations, so their origin is the author's.
      void BuildAuthor(utf8_stringview css) noexcept
      {
        auto matched = MatchResult::Create();
        matched->AuthorDeclarations.push_back(
          MatchedProperties {.Properties = css::tests::ImmutableDeclarations(css)});
        Builder builder {Style, BuilderContext {.Document = Document, .ParentStyle = &ParentStyle}, *matched};
        builder.ApplyAllProperties();
      }

      KRYS_NODISCARD const CounterDirectives &Counter(utf8_stringview name) const noexcept
      {
        static const CounterDirectives none {};
        const auto &map = Style.GetCounterDirectives().Map;
        auto it = map.find(css::CSSOMStringAtom {name});
        return it == map.end() ? none : it->second;
      }
    };
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: colours", "[Style][BuilderCustom]")
  {
    SECTION("currentcolor resolves to the parent's colour")
    {
      ParentStyle.SetColor(boo::Color {boo::Color::blue});
      ParentStyle.SetVisitedLinkColor(boo::Color {boo::Color::green});
      Apply(PropertyId::Color, u8"currentcolor");
      CHECK(Style.GetColor() == boo::Color {boo::Color::blue});
      CHECK(Style.VisitedLinkColor() == boo::Color {boo::Color::green});
      CHECK(Style.DisallowsFastPathInheritance());
    }

    SECTION("Author colours are explicitly set")
    {
      Apply(PropertyId::Color, u8"red");
      CHECK(Style.GetColor() == boo::Color {boo::Color::red});
      // Without a current property, the origin isn't the author's.
      CHECK_FALSE(Style.HasExplicitlySetColor());

      BuildAuthor(u8"color: initial");
      CHECK(Style.GetColor() == ComputedStyle::InitialColor());
      CHECK(Style.HasExplicitlySetColor());
    }

    SECTION("Inherited")
    {
      ParentStyle.SetColor(boo::Color {boo::Color::blue});
      ApplyInherit(PropertyId::Color);
      CHECK(Style.GetColor() == boo::Color {boo::Color::blue});
      CHECK(Style.VisitedLinkColor() == boo::Color {boo::Color::blue});
    }

    SECTION("caret-color: auto")
    {
      Apply(PropertyId::CaretColor, u8"red");
      CHECK_FALSE(Style.HasAutoCaretColor());
      CHECK(Serialised(Style.CaretColor()) == "rgb(255, 0, 0)");

      Apply(PropertyId::CaretColor, u8"auto");
      CHECK(Style.HasAutoCaretColor());
      CHECK(Style.HasVisitedLinkAutoCaretColor());
    }

    SECTION("stroke-color is explicitly set")
    {
      Apply(PropertyId::StrokeColor, u8"red");
      CHECK(Serialised(Style.StrokeColor()) == "rgb(255, 0, 0)");
      CHECK(Style.HasExplicitlySetStrokeColor());
    }

    SECTION("fill and stroke")
    {
      Apply(PropertyId::Fill, u8"red");
      CHECK(Serialised(Style.Fill()) == "rgb(255, 0, 0)");
      CHECK(Serialised(Style.VisitedLinkFill()) == "rgb(255, 0, 0)");

      ApplyInherit(PropertyId::Stroke);
      CHECK(Style.Stroke() == ParentStyle.Stroke());
    }
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: explicitly set flags", "[Style][BuilderCustom]")
  {
    SECTION("Padding by author origin")
    {
      Apply(PropertyId::PaddingTop, u8"5px");
      CHECK(Serialised(Style.PaddingTop()) == "5px");
      CHECK_FALSE(Style.HasExplicitlySetPaddingTop());

      BuildAuthor(u8"padding-top: 6px");
      CHECK(Style.HasExplicitlySetPaddingTop());
    }

    SECTION("Border radii")
    {
      Apply(PropertyId::BorderTopLeftRadius, u8"4px");
      CHECK(Style.HasExplicitlySetBorderTopLeftRadius());
      ApplyInitial(PropertyId::BorderTopLeftRadius);
      CHECK_FALSE(Style.HasExplicitlySetBorderTopLeftRadius());
    }

    SECTION("Direction and writing mode")
    {
      Apply(PropertyId::Direction, u8"rtl");
      CHECK(Style.HasExplicitlySetDirection());
      Apply(PropertyId::WritingMode, u8"vertical-rl");
      CHECK(Style.HasExplicitlySetWritingMode());
      CHECK(Style.GetWritingMode().IsVertical());
    }

    SECTION("stroke-width")
    {
      Apply(PropertyId::StrokeWidth, u8"2px");
      CHECK(Style.HasExplicitlySetStrokeWidth());
    }
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: line widths use the zoom", "[Style][BuilderCustom]")
  {
    // Widths are zero without a style.
    Apply(PropertyId::BorderTopStyle, u8"solid");
    Apply(PropertyId::OutlineStyle, u8"solid");
    std::ignore = Style.SetUsedZoom(2);
    ApplyInitial(PropertyId::BorderTopWidth);
    CHECK(Style.BorderTopWidth().Value.UnresolvedValue() == 6);
    ApplyInitial(PropertyId::OutlineWidth);
    CHECK(Style.OutlineWidth().Value.UnresolvedValue() == 6);
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: counters", "[Style][BuilderCustom]")
  {
    SECTION("Increments add up")
    {
      Apply(PropertyId::CounterIncrement, u8"a 2 a 3 b");
      CHECK(Counter(u8"a").IncrementValue == 5);
      CHECK(Counter(u8"b").IncrementValue == 1);
      CHECK_FALSE(Counter(u8"a").ResetValue);
    }

    SECTION("A new value replaces the previous one")
    {
      Apply(PropertyId::CounterReset, u8"a 2");
      Apply(PropertyId::CounterReset, u8"b");
      CHECK_FALSE(Counter(u8"a").ResetValue);
      CHECK(Counter(u8"b").ResetValue == 0);
    }

    SECTION("Each property keeps its own value")
    {
      Apply(PropertyId::CounterReset, u8"a 2");
      Apply(PropertyId::CounterSet, u8"a 7");
      Apply(PropertyId::CounterIncrement, u8"none");
      CHECK(Counter(u8"a").ResetValue == 2);
      CHECK(Counter(u8"a").SetValue == 7);
      CHECK_FALSE(Counter(u8"a").IncrementValue);
    }

    SECTION("Inherited")
    {
      auto &parentMap = ParentStyle.AccessCounterDirectives().Map;
      parentMap[css::CSSOMStringAtom {u8"c"}] = CounterDirectives {.ResetValue = 1, .SetValue = 4};
      ApplyInherit(PropertyId::CounterSet);
      CHECK(Counter(u8"c").SetValue == 4);
      CHECK_FALSE(Counter(u8"c").ResetValue);
    }
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: font size", "[Style][BuilderCustom]")
  {
    auto specifiedSize = [&]
    {
      return State->GetFontDescription().SpecifiedSize();
    };

    SECTION("Lengths")
    {
      Apply(PropertyId::FontSize, u8"20px");
      CHECK(specifiedSize() == 20);
      CHECK(State->GetFontDescription().IsAbsoluteSize());
    }

    SECTION("Relative to the parent")
    {
      Apply(PropertyId::FontSize, u8"150%");
      CHECK(specifiedSize() == 24);
      Apply(PropertyId::FontSize, u8"larger");
      CHECK(specifiedSize() == Catch::Approx(16 * 1.2f));
      Apply(PropertyId::FontSize, u8"smaller");
      CHECK(specifiedSize() == Catch::Approx(16 / 1.2f));
      Apply(PropertyId::FontSize, u8"calc(50% + 2px)");
      CHECK(specifiedSize() == 10);
    }

    SECTION("Keywords")
    {
      Apply(PropertyId::FontSize, u8"x-large");
      CHECK(specifiedSize() == 24);
      CHECK(State->GetFontDescription().KeywordSizeAsIdentifier() == css::ValueId::XLarge);

      ApplyInitial(PropertyId::FontSize);
      CHECK(specifiedSize() == 16);
      CHECK(State->GetFontDescription().KeywordSizeAsIdentifier() == css::ValueId::Medium);
    }

    SECTION("Inherited")
    {
      Apply(PropertyId::FontSize, u8"30px");
      ApplyInherit(PropertyId::FontSize);
      CHECK(specifiedSize() == 16);
    }

    SECTION("A monospace family changes the keyword size")
    {
      Apply(PropertyId::FontSize, u8"medium");
      Apply(PropertyId::FontFamily, u8"monospace");
      CHECK(specifiedSize() == 13);
      Apply(PropertyId::FontFamily, u8"serif");
      CHECK(specifiedSize() == 16);
    }
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: zoom", "[Style][BuilderCustom]")
  {
    std::ignore = ParentStyle.SetUsedZoom(1.5f);

    SECTION("The used zoom is the parent's times the zoom")
    {
      Apply(PropertyId::Zoom, u8"2");
      CHECK(Style.UsedZoom() == 3);
      CHECK(State->FontDirty());

      Apply(PropertyId::Zoom, u8"normal");
      CHECK(Style.UsedZoom() == 1.5f);
      CHECK(Style.GetZoom() == ComputedStyle::InitialZoom());
    }

    SECTION("Zero is ignored, but the used zoom is still reset")
    {
      Apply(PropertyId::Zoom, u8"2");
      Apply(PropertyId::Zoom, u8"0");
      CHECK(Evaluate<float>(Style.GetZoom()) == 2);
      CHECK(Style.UsedZoom() == 1.5f);
    }

    SECTION("Initial and inherited")
    {
      // Setting the zoom multiplies the used zoom.
      std::ignore = ParentStyle.SetZoom(Zoom {3.f});
      std::ignore = ParentStyle.SetUsedZoom(1.5f);
      ApplyInherit(PropertyId::Zoom);
      CHECK(Evaluate<float>(Style.GetZoom()) == 3);
      CHECK(Style.UsedZoom() == 4.5f);

      ApplyInitial(PropertyId::Zoom);
      CHECK(Style.UsedZoom() == 1.5f);
    }
  }

  TEST_CASE_METHOD(BuilderCustomFixture, "BuilderCustom: spacing dirties the font", "[Style][BuilderCustom]")
  {
    CHECK_FALSE(State->FontDirty());
    Apply(PropertyId::LetterSpacing, u8"2px");
    CHECK(Serialised(Style.ComputedLetterSpacing()) == "2px");
    CHECK(State->FontDirty());

    Apply(PropertyId::WordSpacing, u8"1em");
    CHECK(Serialised(Style.ComputedWordSpacing()) == "16px");
  }
}
