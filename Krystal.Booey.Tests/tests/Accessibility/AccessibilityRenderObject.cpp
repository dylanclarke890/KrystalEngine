#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
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

  TEST_CASE("AccessibilityRenderObject: which lists are lists", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<ul id=marked><li>A</li><li>B</li></ul>"
                              u8"<ul id=unmarked style='list-style:none'><li>A</li><li>B</li></ul>"
                              u8"<nav><ul id=nav style='list-style:none'><li>A</li><li>B</li></ul></nav>"
                              u8"<div id=aria role=list><div role=listitem>A</div></div>"
                              u8"<div id=empty role=list></div>"
                              u8"<dl id=dl><dt>A</dt><dd>B</dd></dl>");
    auto roleOf = [&](dom::DOMStringView id)
    {
      auto &object = page.Object(id);
      (void)object.Children();
      return object.Role();
    };
    CHECK(roleOf(u8"marked") == AccessibilityRole::List);
    CHECK(roleOf(u8"unmarked") == AccessibilityRole::Group);
    CHECK(roleOf(u8"nav") == AccessibilityRole::List);
    CHECK(roleOf(u8"aria") == AccessibilityRole::List);
    CHECK(roleOf(u8"empty") == AccessibilityRole::Group);
    CHECK(roleOf(u8"dl") == AccessibilityRole::DescriptionList);
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

  TEST_CASE("AccessibilityRenderObject: trees", "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<div id=tree role=tree aria-owns=stray>"
                              u8"<div id=item role=treeitem><span>Fruit</span>"
                              u8"<div role=group><div id=nested role=treeitem>Apple</div></div></div>"
                              u8"</div><button id=stray>Stray</button>"
                              u8"<div id=invalid role=tree><button>Not an item</button></div>"
                              u8"<div id=orphan role=treeitem>Orphan</div>");

    CHECK(page.Object(u8"tree").Role() == AccessibilityRole::Tree);
    CHECK(page.Object(u8"item").Role() == AccessibilityRole::TreeItem);
    CHECK(page.Object(u8"nested").Role() == AccessibilityRole::TreeItem);
    // A tree item's descendants are in the tree; anything else in a tree (here, owned) is left out.
    auto &item = page.Object(u8"item");
    CHECK(std::ranges::any_of(item.Children(), [](const auto &child) { return child->IsStaticText(); }));
    CHECK(page.Object(u8"stray").IsIgnored());

    // A tree whose children aren't all tree items (or groups of them) isn't one.
    CHECK(page.Object(u8"invalid").Role() == AccessibilityRole::Generic);
    // A tree item outside a tree loses its role.
    CHECK(page.Object(u8"orphan").Role() != AccessibilityRole::TreeItem);
  }

  TEST_CASE("AccessibilityRenderObject: an ancestor with a click handler",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<div id=clickable><p><span id=text>Click me</span></p><div id=empty></div></div>");
    CHECK(page.Object(u8"text").ActionElement() == nullptr);
    CHECK(page.Object(u8"empty").IsIgnored());

    auto listener =
      page.ById(u8"clickable").AddEventListener(dom::DOMStringAtom(EventNames::Click), [](dom::Event &) {});
    // The clickable element is what pressing its descendants presses, and keeps empty blocks in the tree.
    CHECK(page.Object(u8"text").ActionElement() == &page.ById(u8"clickable"));
    CHECK(page.Object(u8"text").ClickableSelfOrAncestor() == &page.Object(u8"clickable"));
    page.Object(u8"empty").RecomputeIsIgnored();
    CHECK_FALSE(page.Object(u8"empty").IsIgnored());
  }

  TEST_CASE("AccessibilityRenderObject: CSS alt text on generated content",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(u8"<style>#alt::before { content: '*' / 'Starred' }"
                              u8"#decorative::before { content: '*' / '' }</style>"
                              u8"<p id=alt>Text</p><p id=decorative>Text</p>");
    auto &alt = page.Object(u8"alt");
    REQUIRE_FALSE(alt.Children().empty());
    CHECK(alt.Children().front()->StringValue() == u8"Starred");
    // Empty alt text marks the content decorative.
    auto &decorative = page.Object(u8"decorative");
    CHECK(std::ranges::none_of(decorative.Children(),
                               [](const auto &child) { return child->StringValue() == u8"*"; }));
  }

  TEST_CASE("AccessibilityRenderObject: bounds of objects without boxes",
            "[Accessibility][AccessibilityRenderObject]")
  {
    AccessibilityFixture page(
      u8"<style>body { margin: 0 }</style>"
      u8"<div id=contents role=group style='display: contents'>"
      u8"<div style='height: 20px'>A</div><div style='height: 30px'>B</div></div>"
      u8"<div id=parent style='height: 40px'><span id=empty role=img aria-label=E></span></div>");
    // A display: contents element covers its children.
    auto contentsRect = page.Object(u8"contents").ElementRect();
    CHECK(contentsRect.Y() == 0);
    CHECK(contentsRect.Height() == 50);
    // An empty box is put at its nearest rendered ancestor's, at most 10 high.
    auto emptyRect = page.Object(u8"empty").ElementRect();
    CHECK_FALSE(emptyRect.IsEmpty());
    CHECK(emptyRect.Y() == 50);
    CHECK(emptyRect.Height() == 10);
  }
}
