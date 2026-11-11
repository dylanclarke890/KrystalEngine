#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityRenderObject: what's left out of the tree",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<div id=empty></div>"
                              u8"<span id=span>Text</span>"
                              u8"<span id=labelled-span aria-label=Named>Text</span>"
                              u8"<p id=invisible style='visibility: hidden'>Hidden</p>"
                              u8"<p id=aria-hidden aria-hidden=true>Hidden</p>"
                              u8"<div id=inert inert><button id=in-inert>B</button></div>"
                              u8"<div role=presentation id=presentational><p>P</p></div>"
                              u8"<img id=decorative alt='' src='data:,' width=20 height=20>"
                              u8"<img id=described alt='A cat' src='data:,' width=20 height=20>"
                              u8"<div id=focusable tabindex=0>F</div>"
                              u8"<button id=button><span id=in-button>Label</span></button>");

    CHECK(page.Object(u8"empty").IsIgnored());
    CHECK(page.Object(u8"span").IsIgnored());
    CHECK_FALSE(page.Object(u8"labelled-span").IsIgnored());
    CHECK(page.Object(u8"invisible").IsIgnored());
    CHECK(page.Object(u8"aria-hidden").IsIgnored());
    CHECK(page.Object(u8"in-inert").IsIgnored());
    CHECK(page.Object(u8"presentational").IsIgnored());
    CHECK(page.Object(u8"decorative").IsIgnored());
    CHECK_FALSE(page.Object(u8"described").IsIgnored());
    CHECK_FALSE(page.Object(u8"focusable").IsIgnored());
    // A button's descendants are presentational.
    CHECK_FALSE(page.Object(u8"button").IsIgnored());
    CHECK(page.Object(u8"button").Children().empty());
  }

  TEST_CASE("AccessibilityRenderObject: an ignored object's children take its place",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<div id=outer role=group><span><span><a id=link href='#'>Link</a></span></span></div>");
    auto &outer = page.Object(u8"outer");
    REQUIRE(outer.Children().size() == 1);
    CHECK(outer.Children()[0].get() == &page.Object(u8"link"));
    CHECK(page.Object(u8"link").ParentObjectUnignored() == &outer);
  }

  TEST_CASE("AccessibilityRenderObject: text", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<p id=p>Some <em id=em>very</em> text</p><p id=transformed style='text-transform: uppercase'>"
      u8"loud</p>");
    auto &paragraph = page.Object(u8"p");
    // The em is ignored (as a span would be), so its text is the paragraph's.
    REQUIRE(paragraph.Children().size() == 3);
    CHECK(paragraph.Children()[0]->Role() == AccessibilityRole::StaticText);
    CHECK(paragraph.Children()[0]->StringValue() == u8"Some ");
    CHECK(paragraph.Children()[1]->StringValue() == u8"very");
    CHECK(page.Object(u8"em").Role() == AccessibilityRole::Emphasis);
    CHECK(page.Object(u8"em").IsIgnored());

    auto &transformed = page.Object(u8"transformed");
    REQUIRE(transformed.Children().size() == 1);
    CHECK(transformed.Children()[0]->StringValue() == u8"LOUD");
  }

  TEST_CASE("AccessibilityRenderObject: list items' markers", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<ol><li id=item>One</li></ol>");
    auto &item = page.Object(u8"item");
    REQUIRE(item.Children().size() == 2);
    CHECK(item.Children()[0]->Role() == AccessibilityRole::ListMarker);
    CHECK(item.Children()[0]->StringValue() == u8"1. ");
    CHECK(item.Children()[0]->ParentObject() == &item);
    CHECK(item.Children()[1]->Role() == AccessibilityRole::StaticText);
  }

  TEST_CASE("AccessibilityRenderObject: generated content", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<style>#quote::before { content: 'Before' } #quote::after { content: 'After' }</style>"
      u8"<blockquote id=quote>Middle</blockquote>");
    auto &quote = page.Object(u8"quote");
    CHECK(quote.Role() == AccessibilityRole::Blockquote);
    REQUIRE(quote.Children().size() >= 3);
    CHECK(quote.Children().front()->ParentObjectUnignored() == &quote);
    CHECK(quote.Children().back()->ParentObjectUnignored() == &quote);
    CHECK(quote.AccessibleName().empty());
  }

  TEST_CASE("AccessibilityRenderObject: bounds", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<style>body { margin: 0 }</style>"
      u8"<button id=button style='position: absolute; left: 10px; top: 20px; width: 30px;"
      u8" height: 40px'>B</button>");
    CHECK(page.Object(u8"button").ElementRect() == LayoutRect(10, 20, 30, 40));
  }

  TEST_CASE("AccessibilityRenderObject: an object that loses its renderer keeps its node",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<p id=p>Text</p>");
    auto object = ShareRef(page.Object(u8"p"));
    REQUIRE_FALSE(page.ById(u8"p").SetAttribute(u8"style", u8"display: none").HasException());
    (void)page.Document->UpdateLayout();
    CHECK_FALSE(object->IsDetached());
    CHECK_FALSE(object->Renderer());
    CHECK(object->GetNode() == &page.ById(u8"p"));
  }
}
