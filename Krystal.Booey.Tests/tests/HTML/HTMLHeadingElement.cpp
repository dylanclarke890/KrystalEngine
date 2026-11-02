#include "Krystal.Booey/HTML/HTMLHeadingElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::Element> CreateWith(dom::Document &document, utf8_stringview localName,
                                                utf8_stringview attribute = {},
                                                utf8_stringview value = {}) noexcept
    {
      auto element = document.CreateElement(dom::DOMStringAtom(localName)).ReleaseValue();
      if (!attribute.empty())
      {
        REQUIRE_FALSE(
          element->SetAttribute(dom::DOMStringAtom(attribute), dom::DOMString(value)).HasException());
      }
      return element;
    }

    KRYS_NODISCARD uint32 LevelOf(dom::Element &element) noexcept
    {
      return Downcast<HTMLHeadingElement>(Downcast<HTMLElement>(element)).ComputedHeadingLevel();
    }
  }

  TEST_CASE("HTMLHeadingElement::ComputedHeadingLevel: the heading's number, offset, up to 9",
            "[HTML][HTMLHeadingElement][html:get-an-element's-computed-heading-level][html:heading-level]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    CHECK(LevelOf(*CreateWith(*document, u8"h1")) == 1);
    CHECK(LevelOf(*CreateWith(*document, u8"h6")) == 6);
    CHECK(LevelOf(*CreateWith(*document, u8"h2", u8"headingoffset", u8"3")) == 5);
    CHECK(LevelOf(*CreateWith(*document, u8"h4", u8"headingoffset", u8"100")) == 9);
    CHECK(LevelOf(*CreateWith(*document, u8"h4", u8"headingoffset", u8"99999999999999999999999")) == 9);
  }

  TEST_CASE(
    "HTMLHeadingElement::ComputedHeadingOffset: the ancestors' offsets, up to a headingreset",
    "[HTML][HTMLHeadingElement][html:get-an-element's-computed-heading-offset][html:attr-headingoffset]"
    "[html:attr-headingreset]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto outer = CreateWith(*document, u8"section", u8"headingoffset", u8"1");
    auto reset = CreateWith(*document, u8"div", u8"headingoffset", u8"2");
    REQUIRE_FALSE(reset->SetAttribute(u8"headingreset", u8"").HasException());
    auto svg = document->CreateElementNS(infra::Namespaces::SVG, u8"g").ReleaseValue();
    REQUIRE_FALSE(svg->SetAttribute(u8"headingoffset", u8"5").HasException());
    auto invalid = CreateWith(*document, u8"div", u8"headingoffset", u8"-1");
    auto heading = CreateWith(*document, u8"h1", u8"headingoffset", u8"1");

    REQUIRE_FALSE(outer->AppendChild(*reset).HasException());
    REQUIRE_FALSE(reset->AppendChild(*svg).HasException());
    REQUIRE_FALSE(svg->AppendChild(*invalid).HasException());
    REQUIRE_FALSE(invalid->AppendChild(*heading).HasException());

    // The heading's 1, the SVG element's and the invalid value don't count, and the reset's 2 is the last.
    CHECK(HTMLHeadingElement::ComputedHeadingOffset(*heading) == 3);
    CHECK(LevelOf(*heading) == 4);

    // Without the reset, the outer section's counts too.
    reset->RemoveAttribute(u8"headingreset");
    CHECK(HTMLHeadingElement::ComputedHeadingOffset(*heading) == 4);

    // A shadow tree's elements count their host's.
    auto host = CreateWith(*document, u8"div", u8"headingoffset", u8"2");
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto inShadow = CreateWith(*document, u8"h2");
    REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
    CHECK(LevelOf(*inShadow) == 4);
  }
}
