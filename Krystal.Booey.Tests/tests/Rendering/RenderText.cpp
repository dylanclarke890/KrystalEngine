#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLWBRElement.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::tests::ToNarrow;
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The text node child of the element with @p id at @p index (among all its children).
    KRYS_NODISCARD dom::Text &TextChild(ResolverFixture &fixture, dom::DOMStringView id, size_t index = 0)
    {
      auto *child = fixture.ById(id).FirstChild();
      for (; index; --index)
      {
        child = child->NextSibling();
      }
      return Downcast<dom::Text>(*child);
    }

    /// @brief The rendered text of the text node child of the element with @p id at @p index.
    KRYS_NODISCARD utf8_string RenderedText(ResolverFixture &fixture, dom::DOMStringView id, size_t index = 0)
    {
      auto *renderer = TextChild(fixture, id, index).Renderer();
      return renderer ? renderer->Text() : u8"(none)";
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderText: text renderers", "[Rendering][RenderText]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><div id=blocks><div></div> <div></div></div><span id=inlines><b>x</b> <b>y</b></span><div id=lead> <span>x</span></div><div id=breaks><br> <br></div><pre id=pre>  </pre><p id=p>text</p><table id=table> </table><div id=wrap><div style="display: contents; color: red">red</div></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Text gets a renderer, unless it's collapsible whitespace that wouldn't render")
    {
      CHECK(ToNarrow(RenderedText(fixture, u8"p")) == "text");
      CHECK(TextChild(fixture, u8"p").Renderer()->TextNode() == &TextChild(fixture, u8"p"));
      CHECK(TextChild(fixture, u8"p").Renderer()->RenderName() == u8"RenderText");
      // Between blocks.
      CHECK_FALSE(TextChild(fixture, u8"blocks", 1).Renderer());
      // Between inlines.
      CHECK(ToNarrow(RenderedText(fixture, u8"inlines", 1)) == " ");
      // At the start of a block.
      CHECK_FALSE(TextChild(fixture, u8"lead").Renderer());
      // After a <br>.
      CHECK_FALSE(TextChild(fixture, u8"breaks", 1).Renderer());
      // Preserved whitespace.
      CHECK(ToNarrow(RenderedText(fixture, u8"pre")) == "  ");
      // In a table.
      CHECK_FALSE(TextChild(fixture, u8"table").Renderer());
    }

    SECTION("Text in a display: contents element is wrapped in an anonymous inline with its style")
    {
      auto &text = Downcast<dom::Text>(*fixture.ById(u8"wrap").FirstElementChild()->FirstChild());
      auto *renderer = text.Renderer();
      REQUIRE(renderer);
      auto *wrapper = renderer->InlineWrapperForDisplayContents();
      REQUIRE(wrapper);
      CHECK(wrapper->IsAnonymous());
      CHECK(renderer->Parent() == wrapper);
      CHECK(wrapper->Parent() == fixture.ById(u8"wrap").Renderer());
      CHECK(renderer->Style().GetColor() == Color {Color::red});
    }

    SECTION("A change to the text updates its renderer when style is resolved")
    {
      auto &text = TextChild(fixture, u8"p");
      auto *renderer = text.Renderer();
      REQUIRE_FALSE(text.AppendData(u8" more").HasException());
      CHECK(document.NeedsStyleRecalc());
      CHECK(ToNarrow(renderer->Text()) == "text");
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(document.NeedsStyleRecalc());
      CHECK(text.Renderer() == renderer);
      CHECK(ToNarrow(renderer->Text()) == "text more");
    }

    SECTION("Text that becomes collapsible whitespace loses its renderer, and gets it back")
    {
      auto &text = TextChild(fixture, u8"p");
      REQUIRE_FALSE(text.Data(u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(text.Renderer());
      REQUIRE_FALSE(text.Data(u8"back").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(ToNarrow(RenderedText(fixture, u8"p")) == "back");
    }

    SECTION("Removing text destroys its renderer")
    {
      auto &p = fixture.ById(u8"p");
      auto text = ShareRef(TextChild(fixture, u8"p"));
      REQUIRE(text->Renderer());
      REQUIRE_FALSE(p.RemoveChild(*text).HasException());
      CHECK_FALSE(text->Renderer());
      CHECK_FALSE(p.Renderer()->FirstChild());
    }

    SECTION("Moving text gives it a renderer in its new place")
    {
      auto text = ShareRef(TextChild(fixture, u8"p"));
      auto &inlines = fixture.ById(u8"inlines");
      REQUIRE_FALSE(inlines.AppendChild(*text).HasException());
      CHECK_FALSE(text->Renderer());
      document.UpdateStyleIfNeeded();
      REQUIRE(text->Renderer());
      CHECK(text->Renderer()->Parent() == inlines.Renderer());
    }

    SECTION("Without a view, text has no renderer")
    {
      ResolverFixture noView {u8"<p id=p>text</p>", style::tests::WithView::No};
      noView.Document->UpdateStyleIfNeeded();
      CHECK_FALSE(TextChild(noView, u8"p").Renderer());
      REQUIRE_FALSE(TextChild(noView, u8"p").AppendData(u8"!").HasException());
      CHECK_FALSE(noView.Document->NeedsStyleRecalc());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderText: text transform and security",
                   "[Rendering][RenderText]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #upper { text-transform: uppercase }
      #lower { text-transform: lowercase }
      #capitalize { text-transform: capitalize }
      #kana { text-transform: full-size-kana }
      #disc { -webkit-text-security: disc }
      #circle { -webkit-text-security: circle }
      #square { -webkit-text-security: square }
    </style><p id=upper>abc straße</p><p id=lower>ABC</p><p id=capitalize>hello world<span id=continued>wide</span> <span id=next>new</span></p><p id=kana>ぁァ</p><p id=disc>ab€</p><p id=circle>ab</p><p id=square>ab</p><p id=none>Plain</p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Case transforms")
    {
      CHECK(RenderedText(fixture, u8"upper") == u8"ABC STRASSE");
      CHECK(TextChild(fixture, u8"upper").Renderer()->OriginalText() == u8"abc straße");
      CHECK(ToNarrow(RenderedText(fixture, u8"lower")) == "abc");
      CHECK(ToNarrow(RenderedText(fixture, u8"none")) == "Plain");
      CHECK(TextChild(fixture, u8"none").Renderer()->OriginalText() == u8"Plain");
    }

    SECTION("Capitalization continues from the previous text")
    {
      CHECK(ToNarrow(RenderedText(fixture, u8"capitalize")) == "Hello World");
      CHECK(ToNarrow(RenderedText(fixture, u8"continued")) == "wide");
      CHECK(ToNarrow(TextChild(fixture, u8"continued").Renderer()->PreviousCharacter()) == "d");
      CHECK(ToNarrow(RenderedText(fixture, u8"next")) == "New");
    }

    SECTION("Small kana")
    {
      CHECK(RenderedText(fixture, u8"kana") == u8"あア");
    }

    SECTION("Text security masks each code point")
    {
      CHECK(RenderedText(fixture, u8"disc") == u8"•••");
      CHECK(RenderedText(fixture, u8"circle") == u8"◦◦");
      CHECK(RenderedText(fixture, u8"square") == u8"■■");
      CHECK(TextChild(fixture, u8"disc").Renderer()->OriginalText() == u8"ab€");
    }

    SECTION("A style change reapplies the transform")
    {
      auto &upper = fixture.ById(u8"upper");
      REQUIRE_FALSE(upper.SetAttribute(u8"style", u8"text-transform: none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(RenderedText(fixture, u8"upper") == u8"abc straße");
      REQUIRE_FALSE(upper.SetAttribute(u8"style", u8"text-transform: capitalize").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(RenderedText(fixture, u8"upper") == u8"Abc Straße");
    }

    SECTION("A change to the text is transformed")
    {
      REQUIRE_FALSE(TextChild(fixture, u8"upper").Data(u8"new").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(ToNarrow(RenderedText(fixture, u8"upper")) == "NEW");
    }
  }

  TEST_CASE("RenderText: Capitalize", "[Rendering][RenderText]")
  {
    CHECK(Capitalize(u8"hello world") == u8"Hello World");
    CHECK(Capitalize(u8"world", u8"o") == u8"world");
    CHECK(Capitalize(u8" world", u8"o") == u8" World");
    CHECK(Capitalize(u8"élan vital") == u8"Élan Vital");
    // A title-case letter that isn't the upper-case one.
    CHECK(Capitalize(u8"ǆemal") == u8"ǅemal");
    // No-break spaces separate words, and are kept.
    CHECK(Capitalize(u8"a b") == u8"A B");
    CHECK(Capitalize(u8"b", u8" ") == u8"B");
    CHECK(Capitalize(u8"") == u8"");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderLineBreak: <br> and <wbr>",
                   "[Rendering][RenderLineBreak]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=p>a<br id=br>b<wbr id=wbr>c</p>)"};
    fixture.Document->UpdateStyleIfNeeded();

    auto *br = DynamicDowncast<RenderLineBreak>(fixture.ById(u8"br").Renderer());
    REQUIRE(br);
    CHECK(br->RenderName() == u8"RenderBR");
    CHECK(br->IsBR());
    CHECK_FALSE(br->IsWBR());
    CHECK(static_cast<RenderObject *>(br)->IsBR());
    CHECK_FALSE(br->IsLineBreakOpportunity());
    CHECK(br->IsInline());
    CHECK_FALSE(static_cast<RenderElement *>(br)->CanHaveChildren());

    auto *wbr = DynamicDowncast<RenderLineBreak>(fixture.ById(u8"wbr").Renderer());
    REQUIRE(wbr);
    CHECK(wbr->RenderName() == u8"RenderWordBreak");
    CHECK(wbr->IsWBR());
    CHECK(static_cast<RenderObject *>(wbr)->IsWBR());
    CHECK(wbr->IsLineBreakOpportunity());
    CHECK(Is<html::HTMLWBRElement>(Downcast<html::HTMLElement>(fixture.ById(u8"wbr"))));
  }
}
