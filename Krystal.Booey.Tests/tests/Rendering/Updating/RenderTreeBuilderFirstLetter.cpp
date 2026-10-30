#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFirstLetter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::tests::ToNarrow;
  using style::tests::ResolverFixture;

  namespace
  {
    const Color Red {Color::red};
    const Color Blue {Color::blue};
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTreeBuilder::FirstLetter: first-letter renderers",
                   "[Rendering][RenderTreeBuilder]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      p::first-letter { color: red }
      #float::first-letter { float: left }
      #generated::before { content: "abc" }
    </style><p id=p>"Hello" world</p><p id=float>Float</p><p id=generated></p><div id=plain>plain</div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("The first letter, with its punctuation, is split into its own renderer")
    {
      auto &p = fixture.ById(u8"p");
      auto *firstLetter = DynamicDowncast<RenderInline>(p.Renderer()->FirstChild());
      REQUIRE(firstLetter);
      CHECK(firstLetter->IsFirstLetter());
      CHECK(firstLetter->IsAnonymous());
      CHECK(firstLetter->Style().GetPseudoElementType() == PseudoElementType::FirstLetter);
      CHECK(firstLetter->Style().GetColor() == Red);

      auto *letter = DynamicDowncast<RenderTextFragment>(firstLetter->FirstChild());
      REQUIRE(letter);
      CHECK(ToNarrow(letter->Text()) == "\"H");

      auto *remaining = DynamicDowncast<RenderTextFragment>(firstLetter->NextSibling());
      REQUIRE(remaining);
      CHECK(ToNarrow(remaining->Text()) == "ello\" world");
      CHECK(remaining->Start() == 2);
      CHECK(remaining->FirstLetter() == firstLetter);
      CHECK(firstLetter->FirstLetterRemainingText() == remaining);
      CHECK(p.FirstChild()->Renderer() == remaining);
      CHECK(remaining->Style().GetColor() != Red);
    }

    SECTION("A floating first letter is a block flow")
    {
      auto *firstLetter = fixture.ById(u8"float").Renderer()->FirstChild();
      REQUIRE(Is<RenderBlockFlow>(firstLetter));
      CHECK(firstLetter->IsFloating());
      CHECK(Downcast<RenderElement>(*firstLetter).IsFirstLetter());
    }

    SECTION("Generated content has a first letter")
    {
      auto *before = fixture.ById(u8"generated").BeforePseudoElement()->Renderer();
      auto *firstLetter = DynamicDowncast<RenderInline>(before->FirstChild());
      REQUIRE(firstLetter);
      CHECK(firstLetter->IsFirstLetter());
      CHECK(ToNarrow(Downcast<RenderText>(*firstLetter->FirstChild()).Text()) == "a");
      CHECK(ToNarrow(Downcast<RenderText>(*firstLetter->NextSibling()).Text()) == "bc");
    }

    SECTION("Blocks without the pseudo-element have no first letter")
    {
      CHECK(Is<RenderText>(fixture.ById(u8"plain").Renderer()->FirstChild()));
      CHECK_FALSE(Is<RenderTextFragment>(fixture.ById(u8"plain").Renderer()->FirstChild()));
    }

    SECTION("A style change restyles the first letter")
    {
      auto &p = fixture.ById(u8"p");
      auto *firstLetter = p.Renderer()->FirstChild();
      REQUIRE_FALSE(p.SetAttribute(u8"style", u8"color: blue").HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE(Is<RenderInline>(p.Renderer()->FirstChild()));
      CHECK(p.Renderer()->FirstChild() == firstLetter);
      CHECK(Downcast<RenderInline>(*p.Renderer()->FirstChild()).Style().GetColor() == Red);
      auto *remaining = p.FirstChild()->Renderer();
      REQUIRE(Is<RenderTextFragment>(remaining));
      CHECK(Downcast<RenderText>(*remaining).Style().GetColor() == Blue);
    }

    SECTION("A change to the text rebuilds the first letter")
    {
      auto &p = fixture.ById(u8"p");
      REQUIRE_FALSE(Downcast<dom::Text>(*p.FirstChild()).Data(u8"Bye").HasException());
      document.UpdateStyleIfNeeded();
      auto *firstLetter = DynamicDowncast<RenderInline>(p.Renderer()->FirstChild());
      REQUIRE(firstLetter);
      CHECK(firstLetter->IsFirstLetter());
      CHECK(ToNarrow(Downcast<RenderText>(*firstLetter->FirstChild()).Text()) == "B");
      CHECK(ToNarrow(Downcast<RenderText>(*firstLetter->NextSibling()).Text()) == "ye");
    }

    SECTION("Removing the text destroys the first letter")
    {
      auto &p = fixture.ById(u8"p");
      REQUIRE_FALSE(p.RemoveChild(*p.FirstChild()).HasException());
      CHECK_FALSE(p.Renderer()->FirstChild());
    }
  }
}
