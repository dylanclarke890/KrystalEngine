#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Maybe<SRGBA<uint8>> Legacy(dom::DOMStringView value) noexcept
    {
      return HTMLElement::ParseLegacyColorValue(value);
    }
  }

  TEST_CASE("HTMLElement::ParseLegacyColorValue: named colours, #rgb, then thirds of the hex digits",
            "[HTML][PresentationalHints][html:rules-for-parsing-a-legacy-colour-value]")
  {
    CHECK_FALSE(Legacy(u8""));
    CHECK_FALSE(Legacy(u8"transparent"));
    CHECK_FALSE(Legacy(u8" TransParent "));
    CHECK(Legacy(u8"   ") == SRGBA<uint8> {0, 0, 0});
    CHECK(Legacy(u8"red") == SRGBA<uint8> {255, 0, 0});
    CHECK(Legacy(u8" ReD\t") == SRGBA<uint8> {255, 0, 0});
    CHECK(Legacy(u8"#abc") == SRGBA<uint8> {0xaa, 0xbb, 0xcc});
    CHECK(Legacy(u8"#00ff00") == SRGBA<uint8> {0, 255, 0});
    // Only named colours are keywords.
    CHECK(Legacy(u8"currentcolor") == SRGBA<uint8> {0xc0, 0xe0, 0});
    // The legacy rules: non-hex digits become 0, components are taken from thirds of the string.
    CHECK(Legacy(u8"chucknorris") == SRGBA<uint8> {0xc0, 0, 0});
    CHECK(Legacy(u8"sick") == SRGBA<uint8> {0, 0xc0, 0});
    CHECK(Legacy(u8"#1") == SRGBA<uint8> {0x01, 0, 0});
    CHECK(Legacy(u8"#") == SRGBA<uint8> {0, 0, 0});
    CHECK(Legacy(u8"abcd") == SRGBA<uint8> {0xab, 0xcd, 0});
    // Components longer than eight digits keep their last eight, then leading zeros all three share go.
    CHECK(Legacy(u8"1234567890ab1234567890ab1234567890ab") == SRGBA<uint8> {0x56, 0x56, 0x56});
    CHECK(Legacy(u8"000120003400056") == SRGBA<uint8> {0x12, 0x34, 0x56});
    // A non-ASCII code point is one digit, or two outside the BMP.
    CHECK(Legacy(u8"éff") == SRGBA<uint8> {0, 0x0f, 0x0f});
    CHECK(Legacy(u8"\U0001F600ff") == SRGBA<uint8> {0, 0xff, 0});
  }

  TEST_CASE("HTMLElement::ParseLegacyColorValue: 128 digits follow a leading #",
            "[HTML][PresentationalHints][html:rules-for-parsing-a-legacy-colour-value]")
  {
    // The specification truncates to 128 code points before removing the "#", which would leave out the "f".
    dom::DOMString value = u8"#";
    value.append(127, u8'0');
    value += u8'f';
    CHECK(Legacy(value) == SRGBA<uint8> {0, 0, 0xf0});
  }
}
