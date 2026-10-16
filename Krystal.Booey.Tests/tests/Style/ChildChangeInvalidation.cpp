#include "Krystal.Booey/Style/ChildChangeInvalidation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    const boo::Color Blue {boo::Color::blue};

    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    KRYS_NODISCARD bool IsBlue(const dom::Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return style->GetColor() == Blue;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ChildChangeInvalidation: structural pseudo-classes",
                   "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      div:empty { color: rgb(0, 0, 255) }
      li:first-child, li:last-child { color: rgb(0, 0, 255) }
      b:nth-child(2), i:nth-last-child(2) { color: rgb(0, 0, 255) }
      em + span { color: rgb(0, 0, 255) }
    </style><body>
      <div id=e></div>
      <ul id=u><li id=l1></li><li id=l2></li><li id=l3></li></ul>
      <p id=nth><b id=b1></b><b id=b2></b><i id=i1></i><i id=i2></i></p>
      <section id=s><em id=em></em><span id=sp></span></section>
    </body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION(":empty")
    {
      auto &e = fixture.ById(u8"e");
      REQUIRE(IsBlue(e));
      auto text = document.CreateTextNode(u8"");
      REQUIRE_FALSE(e.AppendChild(*text).HasException());
      document.UpdateStyleIfNeeded();
      // An empty text node doesn't stop :empty matching... but data does.
      REQUIRE_FALSE(text->AppendData(u8"x").HasException());
      CHECK(e.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(e));

      REQUIRE_FALSE(e.RemoveChild(*text).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(e));
    }

    SECTION(":first-child and :last-child")
    {
      auto &u = fixture.ById(u8"u");
      auto &l1 = fixture.ById(u8"l1");
      auto &l2 = fixture.ById(u8"l2");
      auto &l3 = fixture.ById(u8"l3");
      REQUIRE(IsBlue(l1));
      REQUIRE_FALSE(IsBlue(l2));
      REQUIRE(IsBlue(l3));

      auto first = Create(document, u8"li");
      REQUIRE_FALSE(u.InsertBefore(*first, &l1).HasException());
      auto last = Create(document, u8"li");
      REQUIRE_FALSE(u.AppendChild(*last).HasException());
      CHECK(l1.StyleValidity() != Validity::Valid);
      CHECK(l3.StyleValidity() != Validity::Valid);
      CHECK(l2.StyleValidity() == Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(l1));
      CHECK_FALSE(IsBlue(l3));
      CHECK(IsBlue(*first));
      CHECK(IsBlue(*last));

      REQUIRE_FALSE(u.RemoveChild(*first).HasException());
      REQUIRE_FALSE(u.RemoveChild(*last).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(l1));
      CHECK(IsBlue(l3));
    }

    SECTION("Positional pseudo-classes")
    {
      auto &nth = fixture.ById(u8"nth");
      auto &b1 = fixture.ById(u8"b1");
      auto &b2 = fixture.ById(u8"b2");
      auto &i1 = fixture.ById(u8"i1");
      REQUIRE(IsBlue(b2));
      REQUIRE(IsBlue(i1));

      auto b0 = Create(document, u8"b");
      REQUIRE_FALSE(nth.InsertBefore(*b0, &b1).HasException());
      auto i3 = Create(document, u8"i");
      REQUIRE_FALSE(nth.AppendChild(*i3).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(b1));
      CHECK_FALSE(IsBlue(b2));
      CHECK_FALSE(IsBlue(i1));
      CHECK(IsBlue(fixture.ById(u8"i2")));
    }

    SECTION("Sibling combinators")
    {
      auto &s = fixture.ById(u8"s");
      auto &sp = fixture.ById(u8"sp");
      REQUIRE(IsBlue(sp));
      auto between = Create(document, u8"b");
      REQUIRE_FALSE(s.InsertBefore(*between, &sp).HasException());
      CHECK(sp.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(sp));

      // Moving it out again.
      REQUIRE_FALSE(fixture.ById(u8"e").AppendChild(*between).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(sp));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ChildChangeInvalidation: :has()", "[Style][Invalidation]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      div:has(> .x) { color: rgb(0, 0, 255) }
      section:has(.y) { color: rgb(0, 0, 255) }
      p:has(+ .z) { color: rgb(0, 0, 255) }
    </style><body>
      <div id=d><span></span></div>
      <section id=s><p><b id=b></b></p></section>
      <article id=a><p id=p></p></article>
    </body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &s = fixture.ById(u8"s");
    REQUIRE(document.StyleScope().UsesHasPseudoClass());
    REQUIRE_FALSE(IsBlue(d));

    SECTION("Inserted and removed children")
    {
      auto x = Create(document, u8"i");
      REQUIRE_FALSE(x->SetAttribute(u8"class", u8"x").HasException());
      REQUIRE_FALSE(d.AppendChild(*x).HasException());
      CHECK(d.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(d));

      REQUIRE_FALSE(d.RemoveChild(*x).HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(IsBlue(d));
    }

    SECTION("Descendants")
    {
      auto y = Create(document, u8"i");
      REQUIRE_FALSE(y->SetAttribute(u8"class", u8"y").HasException());
      REQUIRE_FALSE(fixture.ById(u8"b").AppendChild(*y).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(s));
    }

    SECTION("Class changes in the argument")
    {
      REQUIRE_FALSE(fixture.ById(u8"b").SetAttribute(u8"class", u8"y").HasException());
      CHECK(s.StyleValidity() != Validity::Valid);
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(s));
    }

    SECTION("Siblings")
    {
      auto z = Create(document, u8"i");
      REQUIRE_FALSE(z->SetAttribute(u8"class", u8"z").HasException());
      REQUIRE_FALSE(fixture.ById(u8"a").AppendChild(*z).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(IsBlue(fixture.ById(u8"p")));
    }
  }
}
