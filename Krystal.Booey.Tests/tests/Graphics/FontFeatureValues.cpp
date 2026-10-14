#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FontFeatureValues", "[Graphics][FontFeatureValues]")
  {
    auto values = FontFeatureValues::Create();
    CHECK(values->IsEmpty());

    SECTION("UpdateOrInsertForType")
    {
      values->UpdateOrInsertForType(FontFeatureValuesType::Styleset,
                                    {{u8"nice", {1u, 3u}}, {u8"fancy", {2u}}});
      values->UpdateOrInsertForType(FontFeatureValuesType::Stylistic, {{u8"alt", {1u}}});
      values->UpdateOrInsertForType(FontFeatureValuesType::CharacterVariant, {{u8"cv", {1u, 2u}}});
      values->UpdateOrInsertForType(FontFeatureValuesType::Swash, {{u8"swishy", {1u}}});
      values->UpdateOrInsertForType(FontFeatureValuesType::Ornaments, {{u8"flowers", {4u}}});
      values->UpdateOrInsertForType(FontFeatureValuesType::Annotation, {{u8"circled", {5u}}});
      CHECK_FALSE(values->IsEmpty());
      CHECK(values->Styleset().size() == 2);
      CHECK(values->Styleset().at(u8"nice") == List<uint32> {1u, 3u});
      CHECK(values->Stylistic().at(u8"alt") == List<uint32> {1u});
      CHECK(values->CharacterVariant().at(u8"cv") == List<uint32> {1u, 2u});
      CHECK(values->Swash().at(u8"swishy") == List<uint32> {1u});
      CHECK(values->Ornaments().at(u8"flowers") == List<uint32> {4u});
      CHECK(values->Annotation().at(u8"circled") == List<uint32> {5u});

      // A later tag replaces an earlier one.
      values->UpdateOrInsertForType(FontFeatureValuesType::Styleset, {{u8"nice", {7u}}});
      CHECK(values->Styleset().at(u8"nice") == List<uint32> {7u});
      CHECK(values->Styleset().size() == 2);
    }

    SECTION("UpdateOrInsert")
    {
      values->UpdateOrInsertForType(FontFeatureValuesType::Swash, {{u8"a", {1u}}, {u8"b", {2u}}});
      auto other = FontFeatureValues::Create();
      other->UpdateOrInsertForType(FontFeatureValuesType::Swash, {{u8"b", {3u}}});
      other->UpdateOrInsertForType(FontFeatureValuesType::Annotation, {{u8"c", {4u}}});
      values->UpdateOrInsert(*other);
      CHECK(values->Swash().at(u8"a") == List<uint32> {1u});
      CHECK(values->Swash().at(u8"b") == List<uint32> {3u});
      CHECK(values->Annotation().at(u8"c") == List<uint32> {4u});

      values->UpdateOrInsert(*values);
      CHECK(values->Swash().size() == 2);

      values->Ornaments()[u8"d"] = {5u};
      CHECK(values->Ornaments().size() == 1);
    }

    SECTION("Equality and hashing")
    {
      auto a = FontFeatureValues::Create();
      auto b = FontFeatureValues::Create();
      CHECK(*a == *b);
      CHECK(DefaultHash<FontFeatureValues>::Hash(*a) == DefaultHash<FontFeatureValues>::Hash(*b));

      // Insertion order doesn't matter.
      auto tag = [](int i)
      {
        css::CSSOMString name = u8"tag";
        name += static_cast<char8>(u8'a' + i % 26);
        name += static_cast<char8>(u8'0' + i / 26);
        return name;
      };
      for (int i = 0; i < 32; ++i)
      {
        a->Styleset()[tag(i)] = {static_cast<uint32>(i)};
      }
      for (int i = 31; i >= 0; --i)
      {
        b->Styleset()[tag(i)] = {static_cast<uint32>(i)};
      }
      CHECK(*a == *b);
      CHECK(DefaultHash<FontFeatureValues>::Hash(*a) == DefaultHash<FontFeatureValues>::Hash(*b));

      b->Styleset()[u8"taga0"] = {99u};
      CHECK_FALSE(*a == *b);
      CHECK(DefaultHash<FontFeatureValues>::Hash(*a) != DefaultHash<FontFeatureValues>::Hash(*b));

      // The same tag in a different block differs.
      auto c = FontFeatureValues::Create();
      auto d = FontFeatureValues::Create();
      c->Swash()[u8"x"] = {1u};
      d->Ornaments()[u8"x"] = {1u};
      CHECK_FALSE(*c == *d);
      CHECK(DefaultHash<FontFeatureValues>::Hash(*c) != DefaultHash<FontFeatureValues>::Hash(*d));
    }
  }
}
