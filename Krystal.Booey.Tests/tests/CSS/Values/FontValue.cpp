#include "Krystal.Booey/CSS/Values/FontValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<FontValue> FullFont()
    {
      auto font = FontValue::Create();
      font->Style = PrimitiveValue::Create(ValueId::Italic);
      font->Variant = PrimitiveValue::Create(ValueId::SmallCaps);
      font->Weight = PrimitiveValue::Create(ValueId::Bold);
      font->Width = PrimitiveValue::Create(ValueId::Condensed);
      font->Size = PrimitiveValue::Create(12.0, UnitType::px);
      font->LineHeight = PrimitiveValue::Create(1.5, UnitType::Number);
      font->Family = PrimitiveValue::Create(ValueId::SansSerif);
      return font;
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontValue::Create", "[CSS][Values][FontValue]")
  {
    auto font = FontValue::Create();
    CHECK(font->IsFontValue());
    CHECK_FALSE(font->Style);
    CHECK_FALSE(font->Variant);
    CHECK_FALSE(font->Weight);
    CHECK_FALSE(font->Width);
    CHECK_FALSE(font->Size);
    CHECK_FALSE(font->LineHeight);
    CHECK_FALSE(font->Family);
    CHECK(CssText(*font).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "FontValue serialisation", "[CSS][Values][FontValue]")
  {
    SECTION("Every component")
    {
      CHECK(CssText(*FullFont()) == "italic small-caps bold condensed 12px / 1.5 sans-serif");
    }

    SECTION("Size and family")
    {
      auto font = FontValue::Create();
      font->Size = PrimitiveValue::Create(12.0, UnitType::px);
      font->Family = PrimitiveValue::Create(ValueId::SansSerif);
      CHECK(CssText(*font) == "12px sans-serif");
    }

    SECTION("Line height without a size isn't slash-separated")
    {
      auto font = FontValue::Create();
      font->Weight = PrimitiveValue::Create(ValueId::Bold);
      font->LineHeight = PrimitiveValue::Create(1.5, UnitType::Number);
      CHECK(CssText(*font) == "bold 1.5");

      auto lineHeightOnly = FontValue::Create();
      lineHeightOnly->LineHeight = PrimitiveValue::Create(1.5, UnitType::Number);
      CHECK(CssText(*lineHeightOnly) == "1.5");
    }

    SECTION("The first component has no leading space")
    {
      auto font = FontValue::Create();
      font->Variant = PrimitiveValue::Create(ValueId::SmallCaps);
      CHECK(CssText(*font) == "small-caps");

      auto family = FontValue::Create();
      family->Family = PrimitiveValue::Create(ValueId::SansSerif);
      CHECK(CssText(*family) == "sans-serif");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontValue equality", "[CSS][Values][FontValue]")
  {
    CHECK(*FullFont() == *FullFont());
    CHECK(*FontValue::Create() == *FontValue::Create());

    auto style = FullFont();
    style->Style = PrimitiveValue::Create(ValueId::Normal);
    CHECK_FALSE(*FullFont() == *style);

    auto variant = FullFont();
    variant->Variant = nullptr;
    CHECK_FALSE(*FullFont() == *variant);

    auto weight = FullFont();
    weight->Weight = PrimitiveValue::Create(ValueId::Normal);
    CHECK_FALSE(*FullFont() == *weight);

    auto width = FullFont();
    width->Width = PrimitiveValue::Create(ValueId::Normal);
    CHECK_FALSE(*FullFont() == *width);

    auto size = FullFont();
    size->Size = PrimitiveValue::Create(13.0, UnitType::px);
    CHECK_FALSE(*FullFont() == *size);

    auto lineHeight = FullFont();
    lineHeight->LineHeight = PrimitiveValue::Create(2.0, UnitType::Number);
    CHECK_FALSE(*FullFont() == *lineHeight);

    auto family = FullFont();
    family->Family = PrimitiveValue::Create(ValueId::Serif);
    CHECK_FALSE(*FullFont() == *family);
  }

  TEST_CASE_METHOD(ValueFixture, "FontValue visits its components in order", "[CSS][Values][FontValue]")
  {
    SECTION("Every component")
    {
      auto font = FullFont();
      auto children = Children(*font);
      REQUIRE(children.size() == 7uz);
      CHECK(children[0uz] == font->Style.get());
      CHECK(children[1uz] == font->Variant.get());
      CHECK(children[2uz] == font->Weight.get());
      CHECK(children[3uz] == font->Width.get());
      CHECK(children[4uz] == font->Size.get());
      CHECK(children[5uz] == font->LineHeight.get());
      CHECK(children[6uz] == font->Family.get());
    }

    SECTION("Missing components are skipped")
    {
      auto font = FontValue::Create();
      font->Size = PrimitiveValue::Create(12.0, UnitType::px);
      font->Family = PrimitiveValue::Create(ValueId::SansSerif);
      auto children = Children(*font);
      REQUIRE(children.size() == 2uz);
      CHECK(children[0uz] == font->Size.get());
      CHECK(children[1uz] == font->Family.get());
    }
  }
}
