#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FontFeatureTag", "[Graphics][FontTaggedSettings]")
  {
    CHECK(FontFeatureTag("liga") == FontTag {'l', 'i', 'g', 'a'});
  }

  TEST_CASE("FontTaggedSetting", "[Graphics][FontTaggedSettings]")
  {
    FontFeature on {FontFeatureTag("liga"), 1};
    FontFeature off {FontFeatureTag("liga"), 0};

    CHECK(on.Tag() == FontFeatureTag("liga"));
    CHECK(on.Value() == 1);
    CHECK(on.Enabled());
    CHECK_FALSE(off.Enabled());
    CHECK(on == FontFeature {FontFeatureTag("liga"), 1});
    CHECK_FALSE(on == off);

    CHECK(FontTaggedSetting<float> {FontFeatureTag("wght"), 0.5f}.Enabled());
    CHECK_FALSE(FontTaggedSetting<float> {FontFeatureTag("wght"), 0.0f}.Enabled());
  }

  TEST_CASE("FontTaggedSettings(insert)", "[Graphics][FontTaggedSettings]")
  {
    FontFeatureSettings settings;
    CHECK(settings.isEmpty());

    SECTION("keeps settings sorted by tag")
    {
      settings.insert({FontFeatureTag("smcp"), 1});
      settings.insert({FontFeatureTag("kern"), 1});
      settings.insert({FontFeatureTag("liga"), 0});

      REQUIRE(settings.size() == 3);
      CHECK(settings[0].Tag() == FontFeatureTag("kern"));
      CHECK(settings[1].Tag() == FontFeatureTag("liga"));
      CHECK(settings.at(2).Tag() == FontFeatureTag("smcp"));
    }

    SECTION("replaces a setting with the same tag")
    {
      settings.insert({FontFeatureTag("kern"), 1});
      settings.insert({FontFeatureTag("liga"), 1});
      settings.insert({FontFeatureTag("smcp"), 1});
      settings.insert({FontFeatureTag("liga"), 0});

      REQUIRE(settings.size() == 3);
      CHECK(settings[0] == FontFeature {FontFeatureTag("kern"), 1});
      CHECK(settings[1] == FontFeature {FontFeatureTag("liga"), 0});
      CHECK(settings[2] == FontFeature {FontFeatureTag("smcp"), 1});
    }

    SECTION("iterates in order")
    {
      settings.insert({FontFeatureTag("liga"), 1});
      settings.insert({FontFeatureTag("kern"), 2});

      int sum = 0;
      for (const auto &setting : settings)
      {
        sum = sum * 10 + setting.Value();
      }
      CHECK(sum == 21);
    }
  }

  TEST_CASE("FontTaggedSettings(equality)", "[Graphics][FontTaggedSettings]")
  {
    FontVariationSettings a;
    a.insert({FontFeatureTag("wght"), 700.0f});
    a.insert({FontFeatureTag("wdth"), 75.0f});

    FontVariationSettings b;
    b.insert({FontFeatureTag("wdth"), 75.0f});
    b.insert({FontFeatureTag("wght"), 700.0f});

    CHECK(a == b);
    b.insert({FontFeatureTag("wght"), 400.0f});
    CHECK_FALSE(a == b);
  }

  TEST_CASE("FontTag(hash)", "[Graphics][FontTaggedSettings]")
  {
    CHECK(FourCharacterTagHash::Hash(FontFeatureTag("liga"))
          == FourCharacterTagHash::Hash(FontFeatureTag("liga")));
    CHECK(FourCharacterTagHash::Hash(FontFeatureTag("liga"))
          != FourCharacterTagHash::Hash(FontFeatureTag("kern")));

    FontTag deleted {};
    CHECK(FourCharacterTagHashTraits::EmptyValueIsZero);
    CHECK_FALSE(FourCharacterTagHashTraits::IsDeletedValue(deleted));
    FourCharacterTagHashTraits::ConstructDeletedValue(deleted);
    CHECK(FourCharacterTagHashTraits::IsDeletedValue(deleted));

    using SettingHash = DefaultHash<FontFeature>;
    CHECK(SettingHash::Hash({FontFeatureTag("liga"), 1}) == SettingHash::Hash({FontFeatureTag("liga"), 1}));
    CHECK(SettingHash::Hash({FontFeatureTag("liga"), 1}) != SettingHash::Hash({FontFeatureTag("liga"), 0}));
    CHECK(SettingHash::Equal({FontFeatureTag("liga"), 1}, {FontFeatureTag("liga"), 1}));
  }
}
