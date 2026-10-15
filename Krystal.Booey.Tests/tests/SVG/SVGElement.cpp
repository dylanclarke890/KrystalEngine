#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD std::string Hints(const dom::Element &element) noexcept
    {
      const auto *style = Downcast<dom::StyledElement>(element).PresentationalHintStyle();
      return style ? css::tests::ToNarrow(style->AsText(css::DefaultSerialisationContext())) : std::string {};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGElement: presentation attributes",
                   "[SVG][PresentationalHints]")
  {
    using css::PropertyId;
    CHECK(
      SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"fill"), dom::DOMStringAtom::Null())
      == PropertyId::Fill);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"font-stretch"),
                                                       dom::DOMStringAtom::Null())
          == PropertyId::FontWidth);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"transform-origin"),
                                                       dom::DOMStringAtom::Null())
          == PropertyId::TransformOrigin);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"d"), dom::DOMStringAtom::Null())
          == PropertyId::Invalid);
    CHECK(
      SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"href"), dom::DOMStringAtom::Null())
      == PropertyId::Invalid);
    CHECK(SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom(u8"fill"),
                                                       dom::DOMStringAtom(infra::Namespaces::XLink))
          == PropertyId::Invalid);

    auto document =
      ResolverFixture::Parse(u8R"(<svg><rect id=r x=1 y=2 width=10 height=5 fill=red stroke-width=2
      opacity=0.5 bogus=1 fill-opacity=nonsense /><circle id=c cx=0 cy=0 r=4 /></svg>)");
    CHECK(Hints(*document->GetElementById(u8"r"))
          == "x: 1px; y: 2px; width: 10px; height: 5px; fill: red; stroke-width: 2px; opacity: 0.5;");
    CHECK(Hints(*document->GetElementById(u8"c")) == "cx: 0px; cy: 0px; r: 4px;");

    SECTION("Attribute changes")
    {
      auto &rect = *document->GetElementById(u8"r");
      REQUIRE_FALSE(rect.SetAttribute(u8"fill", u8"blue").HasException());
      CHECK(Hints(rect).find("fill: blue;") != std::string::npos);
    }

    SECTION("Resolution")
    {
      ResolverFixture fixture {u8"<!DOCTYPE html><svg><g id=g visibility=hidden display=none></g></svg>"};
      auto style = fixture.Resolve(fixture.ById(u8"g")).Style;
      CHECK(style->Display() == DisplayType::None);
    }
  }
}
