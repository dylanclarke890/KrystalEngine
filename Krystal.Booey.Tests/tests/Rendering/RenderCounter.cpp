#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderQuote.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::tests::ToNarrow;
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The text of the generated content of @p element's @p type pseudo-element.
    KRYS_NODISCARD utf8_string GeneratedText(dom::Element &element, PseudoElementType type)
    {
      auto *pseudoElement =
        type == PseudoElementType::Before ? element.BeforePseudoElement() : element.AfterPseudoElement();
      if (!pseudoElement || !pseudoElement->Renderer())
      {
        return u8"(none)";
      }
      utf8_string text;
      for (auto &renderer : DescendantsOfType<RenderText>(*pseudoElement->Renderer()))
      {
        text += renderer.Text();
      }
      return text;
    }

    KRYS_NODISCARD std::string Before(ResolverFixture &fixture, dom::DOMStringView id)
    {
      return ToNarrow(GeneratedText(fixture.ById(id), PseudoElementType::Before));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderCounter: generated content", "[Rendering][RenderCounter]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      .n { counter-reset: s }
      .x::before { counter-increment: s; content: counters(s, ".") " " }
      #text::before { content: "hello" }
      #empty::before { content: "" }
      #roman::before { counter-reset: r 3; content: counter(r, upper-roman) }
    </style><div class=n id=root><div class=x id=a></div><div class=x id=b><div class=n><div class=x id=c></div><div class=x id=d></div></div></div></div><p id=text></p><p id=empty></p><p id=roman></p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Text content is a text fragment")
    {
      auto *before = fixture.ById(u8"text").BeforePseudoElement();
      REQUIRE(before);
      auto *fragment = DynamicDowncast<RenderTextFragment>(before->Renderer()->FirstChild());
      REQUIRE(fragment);
      CHECK(ToNarrow(fragment->Text()) == "hello");
      CHECK(ToNarrow(fragment->ContentString()) == "hello");
      CHECK(fragment->IsAnonymous());
      // Empty text gets no renderer.
      auto *emptyBefore = fixture.ById(u8"empty").BeforePseudoElement();
      REQUIRE(emptyBefore);
      CHECK_FALSE(emptyBefore->Renderer()->FirstChild());
    }

    SECTION("Counters count in tree order, nested by counter-reset")
    {
      CHECK(Before(fixture, u8"a") == "1 ");
      CHECK(Before(fixture, u8"b") == "2 ");
      CHECK(Before(fixture, u8"c") == "2.1 ");
      CHECK(Before(fixture, u8"d") == "2.2 ");
      CHECK(Before(fixture, u8"roman") == "III");

      auto *counter =
        DynamicDowncast<RenderCounter>(fixture.ById(u8"a").BeforePseudoElement()->Renderer()->FirstChild());
      REQUIRE(counter);
      CHECK(counter->RenderName() == u8"RenderCounter");
    }

    SECTION("Counters are recounted when the tree changes")
    {
      auto created = document.CreateElement(u8"div");
      REQUIRE_FALSE(created.HasException());
      auto inserted = created.ReleaseValue();
      REQUIRE_FALSE(inserted->SetAttribute(u8"class", u8"x").HasException());
      REQUIRE_FALSE(fixture.ById(u8"root").InsertBefore(*inserted, &fixture.ById(u8"a")).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(ToNarrow(GeneratedText(*inserted, PseudoElementType::Before)) == "1 ");
      CHECK(Before(fixture, u8"a") == "2 ");
      CHECK(Before(fixture, u8"b") == "3 ");
      CHECK(Before(fixture, u8"c") == "3.1 ");

      REQUIRE_FALSE(fixture.ById(u8"a").Remove().HasException());
      // Counters are recounted with the next render tree update.
      REQUIRE_FALSE(fixture.ById(u8"root").SetAttribute(u8"style", u8"color: green").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Before(fixture, u8"b") == "2 ");
      CHECK(Before(fixture, u8"d") == "2.2 ");
    }

    SECTION("Counters follow counter directive changes")
    {
      REQUIRE_FALSE(fixture.ById(u8"b").SetAttribute(u8"style", u8"counter-reset: s 10").HasException());
      document.UpdateStyleIfNeeded();
      // b resets a new s and its ::before increments it.
      CHECK(Before(fixture, u8"b") == "1.11 ");
      CHECK(Before(fixture, u8"a") == "1 ");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderQuote: quotes", "[Rendering][RenderQuote]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #custom { quotes: "<" ">" "{" "}" }
      #none::before { content: no-open-quote }
    </style><p><q id=q1>a<q id=q2>b</q></q></p><p lang=fr><q id=fr>c</q></p><p id=custom><q id=c1>d<q id=c2>e</q></q></p><p><q id=none>f</q></p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto quote = [&](dom::DOMStringView id, PseudoElementType type)
    {
      return GeneratedText(fixture.ById(id), type);
    };

    SECTION("Quotes nest, with the default quotes")
    {
      CHECK(quote(u8"q1", PseudoElementType::Before) == u8"\"");
      CHECK(quote(u8"q2", PseudoElementType::Before) == u8"'");
      CHECK(quote(u8"q2", PseudoElementType::After) == u8"'");
      CHECK(quote(u8"q1", PseudoElementType::After) == u8"\"");
      auto *renderer = fixture.ById(u8"q1").BeforePseudoElement()->Renderer();
      REQUIRE(Is<RenderQuote>(renderer->FirstChild()));
      CHECK(Downcast<RenderQuote>(*renderer->FirstChild()).RenderName() == u8"RenderQuote");
    }

    SECTION("Quotes for the language, and from the quotes property")
    {
      CHECK(quote(u8"fr", PseudoElementType::Before) == u8"«");
      CHECK(quote(u8"fr", PseudoElementType::After) == u8"»");
      CHECK(quote(u8"c1", PseudoElementType::Before) == u8"<");
      CHECK(quote(u8"c2", PseudoElementType::Before) == u8"{");
      CHECK(quote(u8"c2", PseudoElementType::After) == u8"}");
      CHECK(quote(u8"c1", PseudoElementType::After) == u8">");
    }

    SECTION("no-open-quote shows nothing but still nests")
    {
      CHECK(quote(u8"none", PseudoElementType::Before).empty());
      // The close quote closes the no-open-quote.
      CHECK(quote(u8"none", PseudoElementType::After) == u8"\"");
    }

    SECTION("Removing a quote updates the depth of the following ones")
    {
      auto &q1 = fixture.ById(u8"q1");
      auto q2 = ShareRef(fixture.ById(u8"q2"));
      REQUIRE_FALSE(q1.ParentNode()->InsertBefore(*q2, &q1).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(quote(u8"q2", PseudoElementType::Before) == u8"\"");
      CHECK(quote(u8"q1", PseudoElementType::Before) == u8"\"");
    }
  }
}
