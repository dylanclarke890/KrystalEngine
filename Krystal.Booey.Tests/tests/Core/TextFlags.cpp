#include "Krystal.Booey/Core/TextFlags.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD Maybe<int> FeatureValue(const FeaturesMap &features, const char (&tag)[5])
    {
      auto found = features.find(FontFeatureTag(tag));
      if (found == features.end())
      {
        return null;
      }
      return found->second;
    }
  }

  TEST_CASE("TextFlags: font variant settings", "[Core][TextFlags]")
  {
    SECTION("All normal")
    {
      FontVariantSettings settings;
      CHECK(settings.IsAllNormal());
      CHECK(ComputeFeatureSettingsFromVariants(settings, nullptr).empty());

      settings.Caps = FontVariantCaps::Small;
      CHECK_FALSE(settings.IsAllNormal());
    }

    SECTION("Feature tags for the variants")
    {
      FontVariantSettings settings;
      settings.CommonLigatures = FontVariantLigatures::No;
      settings.DiscretionaryLigatures = FontVariantLigatures::Yes;
      settings.Position = FontVariantPosition::Superscript;
      settings.Caps = FontVariantCaps::AllPetite;
      settings.NumericFigure = FontVariantNumericFigure::OldStyleNumbers;
      settings.NumericSlashedZero = FontVariantNumericSlashedZero::Yes;
      settings.EastAsianVariant = FontVariantEastAsianVariant::Jis04;
      settings.EastAsianRuby = FontVariantEastAsianRuby::Yes;

      auto features = ComputeFeatureSettingsFromVariants(settings, nullptr);
      CHECK(FeatureValue(features, "liga") == 0);
      CHECK(FeatureValue(features, "clig") == 0);
      CHECK(FeatureValue(features, "dlig") == 1);
      CHECK(FeatureValue(features, "sups") == 1);
      CHECK(FeatureValue(features, "c2pc") == 1);
      CHECK(FeatureValue(features, "pcap") == 1);
      CHECK(FeatureValue(features, "onum") == 1);
      CHECK(FeatureValue(features, "zero") == 1);
      CHECK(FeatureValue(features, "jp04") == 1);
      CHECK(FeatureValue(features, "ruby") == 1);
      CHECK(features.size() == 10);
    }

    SECTION("Alternates use the font feature values")
    {
      auto featureValues = FontFeatureValues::Create();
      featureValues->Styleset().insert_or_assign(css::CSSOMString {u8"fancy"}, List<uint32> {3, 12, 100});
      featureValues->Swash().insert_or_assign(css::CSSOMString {u8"flowing"}, List<uint32> {2});
      featureValues->Stylistic().insert_or_assign(css::CSSOMString {u8"alt"}, List<uint32> {4});

      FontVariantSettings settings;
      auto &values = settings.Alternates.ValuesRef();
      values.HistoricalForms = true;
      values.Styleset.push_back(css::CSSOMString {u8"fancy"});
      values.Styleset.push_back(css::CSSOMString {u8"missing"});
      values.Swash = css::CSSOMString {u8"flowing"};
      values.Stylistic = css::CSSOMString {u8"alt"};

      auto features = ComputeFeatureSettingsFromVariants(settings, featureValues);
      CHECK(FeatureValue(features, "hist") == 1);
      CHECK(FeatureValue(features, "ss03") == 1);
      CHECK(FeatureValue(features, "ss12") == 1);
      CHECK(FeatureValue(features, "swsh") == 2);
      CHECK(FeatureValue(features, "cswh") == 2);
      CHECK(FeatureValue(features, "salt") == 4);
      CHECK(features.size() == 6);

      // Without feature values, only the historical forms apply.
      CHECK(ComputeFeatureSettingsFromVariants(settings, nullptr).size() == 1);
    }

    SECTION("Alternates")
    {
      auto normal = FontVariantAlternates::Normal();
      CHECK(normal.IsNormal());

      auto alternates = FontVariantAlternates::Normal();
      alternates.ValuesRef().Ornaments = css::CSSOMString {u8"o"};
      CHECK_FALSE(alternates.IsNormal());
      CHECK(alternates != normal);
      CHECK(DefaultHash<FontVariantAlternates>::Hash(alternates)
            != DefaultHash<FontVariantAlternates>::Hash(normal));

      auto copy = alternates;
      CHECK(copy == alternates);
      CHECK(DefaultHash<FontVariantAlternates>::Hash(copy)
            == DefaultHash<FontVariantAlternates>::Hash(alternates));
    }

    SECTION("Expansion behaviour")
    {
      CHECK(ExpansionBehavior::DefaultBehavior() == ExpansionBehavior::AllowRightOnly());
      CHECK(ExpansionBehavior::ForbidAll().Left == ExpansionBehavior::Behavior::Forbid);
      CHECK(ExpansionBehavior::ForceLeftOnly().Left == ExpansionBehavior::Behavior::Force);
    }
  }

  TEST_CASE("TextSpacing", "[Core][TextSpacing]")
  {
    using text_spacing::CharacterClass;

    SECTION("text-spacing-trim")
    {
      text_spacing::CharactersData data;
      data.CurrentCharacterClass = CharacterClass::FullWidthOpeningPunctuation;
      CHECK(TextSpacingTrim {TextSpacingTrim::TrimType::TrimAll}.ShouldTrimSpacing(data));
      CHECK_FALSE(TextSpacingTrim {TextSpacingTrim::TrimType::SpaceAll}.ShouldTrimSpacing(data));
      CHECK_FALSE(TextSpacingTrim {TextSpacingTrim::TrimType::Auto}.ShouldTrimSpacing(data));

      data.CurrentCharacterClass = CharacterClass::Ideograph;
      CHECK_FALSE(TextSpacingTrim {TextSpacingTrim::TrimType::TrimAll}.ShouldTrimSpacing(data));
      CHECK(TextSpacingTrim {}.IsSpaceAll());
    }

    SECTION("text-autospace")
    {
      TextAutospace none;
      CHECK(none.IsNoAutospace());
      CHECK_FALSE(none.ShouldApplySpacing(CharacterClass::Ideograph, CharacterClass::NonIdeographLetter));

      TextAutospace normal {TextAutospace::Type::Normal};
      CHECK(normal.HasIdeographAlpha());
      CHECK(normal.HasIdeographNumeric());
      CHECK(normal.ShouldApplySpacing(CharacterClass::Ideograph, CharacterClass::NonIdeographLetter));
      CHECK(normal.ShouldApplySpacing(CharacterClass::NonIdeographNumeral, CharacterClass::Ideograph));
      CHECK_FALSE(normal.ShouldApplySpacing(CharacterClass::Ideograph, CharacterClass::Ideograph));

      TextAutospace alpha {TextAutospace::Type::IdeographAlpha};
      CHECK(alpha.ShouldApplySpacing(CharacterClass::Ideograph, CharacterClass::NonIdeographLetter));
      CHECK_FALSE(alpha.ShouldApplySpacing(CharacterClass::Ideograph, CharacterClass::NonIdeographNumeral));
    }
  }
}
