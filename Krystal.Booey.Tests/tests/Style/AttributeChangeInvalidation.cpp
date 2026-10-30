#include "Krystal.Booey/Style/AttributeChangeInvalidation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/IdChangeInvalidation.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    const boo::Color Blue {boo::Color::blue};

    KRYS_NODISCARD const RenderStyle &Rendered(const dom::Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return *style;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ClassChangeInvalidation: class changes",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      .x .c { color: rgb(0, 0, 255) }
      .x + p { width: 5px }
      :not(.y) > b { height: 5px }
    </style><div id=d><p id=c class=c><b id=b></b></p><p id=o></p></div><p id=n></p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &c = fixture.ById(u8"c");
    auto &o = fixture.ById(u8"o");
    auto &n = fixture.ById(u8"n");
    auto &b = fixture.ById(u8"b");
    REQUIRE(Rendered(c).GetColor() != Blue);

    SECTION("Adding a class invalidates the descendants and siblings it may affect")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"class", u8"x").HasException());
      CHECK(c.StyleValidity() == Validity::ElementInvalid);
      CHECK(o.StyleValidity() == Validity::Valid);
      CHECK(n.StyleValidity() == Validity::ElementInvalid);
      CHECK(d.StyleValidity() == Validity::Valid);

      document.UpdateStyleIfNeeded();
      CHECK(Rendered(c).GetColor() == Blue);
      CHECK(ResolverFixture::Serialised(Rendered(n), Rendered(n).Width()) == "5px");

      SECTION("Removing it too")
      {
        d.RemoveAttribute(u8"class");
        CHECK(c.StyleValidity() == Validity::ElementInvalid);
        document.UpdateStyleIfNeeded();
        CHECK(Rendered(c).GetColor() != Blue);
        CHECK(ResolverFixture::Serialised(Rendered(n), Rendered(n).Width()) == "auto");
      }
    }

    SECTION("Classes no rule uses don't invalidate")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"class", u8"unused other").HasException());
      CHECK(c.StyleValidity() == Validity::Valid);
      CHECK(n.StyleValidity() == Validity::Valid);
    }

    SECTION("Negated classes invalidate when they're added")
    {
      REQUIRE(ResolverFixture::Serialised(Rendered(b), Rendered(b).Height()) == "5px");
      REQUIRE_FALSE(c.SetAttribute(u8"class", u8"c y").HasException());
      CHECK(b.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(ResolverFixture::Serialised(Rendered(b), Rendered(b).Height()) == "auto");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "IdChangeInvalidation: ID changes", "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { color: rgb(0, 0, 255) }
      #b p { width: 5px }
    </style><div id=d><p id=p></p></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &p = fixture.ById(u8"p");

    REQUIRE_FALSE(d.SetAttribute(u8"id", u8"a").HasException());
    CHECK(d.StyleValidity() == Validity::ElementInvalid);
    CHECK(p.StyleValidity() == Validity::Valid);
    document.UpdateStyleIfNeeded();
    CHECK(Rendered(d).GetColor() == Blue);

    // IDs matching ancestors invalidate the subtree.
    REQUIRE_FALSE(d.SetAttribute(u8"id", u8"b").HasException());
    CHECK(d.StyleValidity() == Validity::SubtreeInvalid);
    document.UpdateStyleIfNeeded();
    CHECK(Rendered(d).GetColor() != Blue);
    CHECK(ResolverFixture::Serialised(Rendered(p), Rendered(p).Width()) == "5px");

    // IDs no rule uses don't.
    REQUIRE_FALSE(p.SetAttribute(u8"id", u8"unused").HasException());
    CHECK(p.StyleValidity() == Validity::Valid);

    {
      IdChangeInvalidation invalidation(d, dom::DOMStringAtom(u8"b"), dom::DOMStringAtom(u8"b"));
    }
    CHECK(d.StyleValidity() == Validity::Valid);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AttributeChangeInvalidation: attribute changes",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      [data-v="1"] ~ p { color: rgb(0, 0, 255) }
      [style] + i { width: 5px }
    </style><div id=d></div><p id=p></p><b id=b></b><i id=i></i>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &p = fixture.ById(u8"p");

    SECTION("Changes that change what the attribute selectors match invalidate")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"data-v", u8"1").HasException());
      CHECK(p.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(p).GetColor() == Blue);

      REQUIRE_FALSE(d.SetAttribute(u8"data-v", u8"2").HasException());
      CHECK(p.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(p).GetColor() != Blue);

      // Neither value matches.
      REQUIRE_FALSE(d.SetAttribute(u8"data-v", u8"3").HasException());
      CHECK(p.StyleValidity() == Validity::Valid);
    }

    SECTION("Attributes no rule uses don't invalidate")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"title", u8"1").HasException());
      CHECK(p.StyleValidity() == Validity::Valid);
      CHECK(d.StyleValidity() == Validity::Valid);
    }

    SECTION("Inline style changes synchronise the style attribute for non-subject [style] selectors")
    {
      auto &b = Downcast<dom::StyledElement>(fixture.ById(u8"b"));
      auto &i = fixture.ById(u8"i");
      REQUIRE(b.SetInlineStyleProperty(css::PropertyId::Color, css::CSSOMString(u8"red")));
      CHECK(b.HasAttribute(u8"style"));
      CHECK(i.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK(ResolverFixture::Serialised(Rendered(i), Rendered(i).Width()) == "5px");
    }
  }
}
