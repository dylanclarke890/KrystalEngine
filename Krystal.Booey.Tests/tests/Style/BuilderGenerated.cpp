#include "Krystal.Booey/Style/BuilderGenerated.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/BuilderStateInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    /// @brief Applies property values through the generated builder.
    struct BuilderGeneratedFixture : BuilderFixture
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
    };
  }

  TEST_CASE_METHOD(BuilderGeneratedFixture, "BuilderGenerated: values, initial values and inheritance",
                   "[Style][BuilderGenerated]")
  {
    SECTION("A value is converted and set")
    {
      Apply(PropertyId::Opacity, u8"0.5");
      CHECK(Style.GetOpacity() == Opacity {0.5f});
      CHECK(Style.GetFontDescription() == ParentStyle.GetFontDescription());
    }

    SECTION("The initial value")
    {
      Apply(PropertyId::Opacity, u8"0.5");
      ApplyInitial(PropertyId::Opacity);
      CHECK(Style.GetOpacity() == ComputedStyle::InitialOpacity());
    }

    SECTION("The parent's value")
    {
      ParentStyle.SetOpacity(Opacity {0.25f});
      ApplyInherit(PropertyId::Opacity);
      CHECK(Style.GetOpacity() == Opacity {0.25f});
    }

    SECTION("Keywords")
    {
      Apply(PropertyId::Visibility, u8"hidden");
      CHECK(Style.GetVisibility() == Visibility::Hidden);
    }

    SECTION("Fast path inherited properties disallow fast path inheritance")
    {
      CHECK_FALSE(Style.DisallowsFastPathInheritance());
      Apply(PropertyId::Opacity, u8"0.5");
      CHECK_FALSE(Style.DisallowsFastPathInheritance());
      Apply(PropertyId::Visibility, u8"hidden");
      CHECK(Style.DisallowsFastPathInheritance());
    }

    SECTION("The invalid id is ignored")
    {
      auto value = css::PrimitiveValue::Create(css::ValueId::Initial);
      Apply(PropertyId::Invalid, *value, ApplyValueType::Value);
      CHECK(Style.GetOpacity() == ComputedStyle::InitialOpacity());
    }
  }

  TEST_CASE_METHOD(BuilderGeneratedFixture, "BuilderGenerated: visited link colours",
                   "[Style][BuilderGenerated]")
  {
    SECTION("Both the regular and the visited link style")
    {
      Apply(PropertyId::BackgroundColor, u8"red");
      CHECK(Serialised(Style.BackgroundColor()) == "rgb(255, 0, 0)");
      CHECK(Serialised(Style.VisitedLinkBackgroundColor()) == "rgb(255, 0, 0)");
    }

    SECTION("Initial and inherited values")
    {
      ParentStyle.SetBackgroundColor(Color {boo::Color {boo::Color::blue}});
      ApplyInherit(PropertyId::BackgroundColor);
      CHECK(Serialised(Style.BackgroundColor()) == "rgb(0, 0, 255)");
      CHECK(Serialised(Style.VisitedLinkBackgroundColor()) == "rgb(0, 0, 255)");

      ApplyInitial(PropertyId::BackgroundColor);
      CHECK(Style.BackgroundColor() == ComputedStyle::InitialBackgroundColor());
      CHECK(Style.VisitedLinkBackgroundColor() == ComputedStyle::InitialBackgroundColor());
    }
  }

  TEST_CASE_METHOD(BuilderGeneratedFixture, "BuilderGenerated: font properties", "[Style][BuilderGenerated]")
  {
    SECTION("Font properties set the font description")
    {
      Apply(PropertyId::FontWeight, u8"bold");
      CHECK(Style.GetFontWeight() == FontWeight {700});
      CHECK(State->FontDirty());
    }

    SECTION("Inherited")
    {
      Apply(PropertyId::FontStyle, u8"italic");
      ParentStyle.SetFontWeight(FontWeight {300});
      ApplyInherit(PropertyId::FontWeight);
      CHECK(Style.GetFontWeight() == FontWeight {300});
    }

    SECTION("A system font keyword applies the initial value")
    {
      Apply(PropertyId::FontWeight, u8"bold");
      auto caption = css::PrimitiveValue::Create(css::ValueId::Caption);
      Apply(PropertyId::FontWeight, *caption, ApplyValueType::Value);
      CHECK(Style.GetFontWeight() == ComputedStyle::InitialFontWeight());
    }
  }

  TEST_CASE_METHOD(BuilderGeneratedFixture, "BuilderGenerated: coordinated value lists",
                   "[Style][BuilderGenerated]")
  {
    SECTION("A list fills the layers")
    {
      Apply(PropertyId::AnimationDuration, u8"1s, 2s");
      const auto &animations = Style.GetAnimations();
      REQUIRE(animations.ComputedLength() == 2);
      CHECK(Serialised(animations[0].GetDuration()) == "1s");
      CHECK(Serialised(animations[1].GetDuration()) == "2s");
    }

    SECTION("A shorter list clears the remaining layers")
    {
      Apply(PropertyId::AnimationDuration, u8"1s, 2s");
      Apply(PropertyId::AnimationDuration, u8"3s");
      const auto &animations = Style.GetAnimations();
      REQUIRE(animations.ComputedLength() == 2);
      CHECK(Serialised(animations[0].GetDuration()) == "3s");
      CHECK_FALSE(animations[1].IsDurationSet());
    }

    SECTION("Initial")
    {
      Apply(PropertyId::AnimationDuration, u8"1s, 2s");
      ApplyInitial(PropertyId::AnimationDuration);
      const auto &animations = Style.GetAnimations();
      CHECK(animations[0].GetDuration() == Animation::InitialDuration());
      CHECK_FALSE(animations[1].IsDurationSet());
    }

    SECTION("Inherited")
    {
      // Build the parent's list with a state over the parent style.
      auto parentState =
        CreateUnique<BuilderState>(ParentStyle, BuilderContext {.Document = Document, .ParentStyle = &Style});
      auto parsed = Parse(PropertyId::AnimationDuration, u8"4s, 5s");
      BuilderGenerated::ApplyProperty(PropertyId::AnimationDuration, *parentState, *parsed,
                                      ApplyValueType::Value);

      ApplyInherit(PropertyId::AnimationDuration);
      const auto &animations = Style.GetAnimations();
      REQUIRE(animations.ComputedLength() == 2);
      CHECK(Serialised(animations[0].GetDuration()) == "4s");
      CHECK(Serialised(animations[1].GetDuration()) == "5s");
    }
  }
}
