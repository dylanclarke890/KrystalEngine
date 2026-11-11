#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXObjectCache: a document creates its cache once accessibility is enabled",
            "[Accessibility][AXObjectCache]")
  {
    boo::tests::PageInFrameFixture page(u8"<p>Hello</p>");
    CHECK_FALSE(page.Document->AxObjectCache());
    CHECK_FALSE(page.Document->ExistingAXObjectCache());

    AXObjectCache::EnableAccessibility();
    auto *cache = page.Document->AxObjectCache();
    CHECK(cache);
    CHECK(page.Document->ExistingAXObjectCache() == cache);
    CHECK(page.Document->AxObjectCache() == cache);
    AXObjectCache::DisableAccessibility();
  }

  TEST_CASE("AXObjectCache: a document outside a frame has no cache", "[Accessibility][AXObjectCache]")
  {
    css::tests::ValueFixture values;
    auto document = CreateRef<dom::HTMLDocument>();
    AXObjectCache::EnableAccessibility();
    CHECK_FALSE(document->AxObjectCache());
    AXObjectCache::DisableAccessibility();
  }

  TEST_CASE("AXObjectCache: the root is the document's view, above its web area",
            "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<p>Hello</p>");
    auto *root = page.Cache().RootObject();
    REQUIRE(root);
    CHECK(root->Role() == AccessibilityRole::ScrollArea);
    CHECK_FALSE(root->IsIgnored());
    CHECK(root->ParentObject() == nullptr);
    CHECK(page.Cache().RootObject() == root);

    auto *webArea = page.Cache().Get(*page.Document);
    REQUIRE(webArea);
    CHECK(webArea->Role() == AccessibilityRole::WebArea);
    CHECK(webArea->IsRootWebArea());
    CHECK(webArea->ParentObject() == root);
    REQUIRE_FALSE(root->Children().empty());
    CHECK(root->Children()[0].get() == webArea);
  }

  TEST_CASE("AXObjectCache: GetOrCreate creates one object per node", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<p id=x>Hello</p>");
    auto &object = page.Object(u8"x");
    CHECK(page.Cache().Get(page.ById(u8"x")) == &object);
    CHECK(page.Cache().GetOrCreate(page.ById(u8"x")) == &object);
    CHECK(page.Cache().ObjectForID(object.ObjectID()) == &object);
    CHECK(object.GetNode() == &page.ById(u8"x"));
    CHECK(object.Renderer() == page.ById(u8"x").Renderer());
  }

  TEST_CASE("AXObjectCache: unrendered nodes get objects only when needed", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<p id=hidden style='display: none'>Hidden</p>"
                              u8"<nav id=contents style='display: contents'>Links</nav>");

    SECTION("display: none")
    {
      CHECK_FALSE(page.Cache().GetOrCreate(page.ById(u8"hidden")));
    }

    SECTION("display: none, but part of a relation")
    {
      auto *object = page.Cache().GetOrCreate(page.ById(u8"hidden"), AXObjectCache::IsPartOfRelation::Yes);
      REQUIRE(object);
      CHECK(object->IsIgnored());
    }

    SECTION("display: contents")
    {
      auto *object = page.Cache().GetOrCreate(page.ById(u8"contents"));
      REQUIRE(object);
      CHECK_FALSE(object->Renderer());
      CHECK(object->Role() == AccessibilityRole::LandmarkNavigation);
      CHECK_FALSE(object->IsIgnored());
    }
  }

  TEST_CASE("AXObjectCache: removing a node detaches its object", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<p id=x>Hello</p>");
    auto object = ShareRef(page.Object(u8"x"));
    REQUIRE_FALSE(page.ById(u8"x").Remove().HasException());
    CHECK(object->IsDetached());
    CHECK_FALSE(object->AxObjectCache());
    CHECK_FALSE(page.Cache().ObjectForID(object->ObjectID()));
  }

  TEST_CASE("AXObjectCache: clearing the cache detaches its objects", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<p id=x>Hello</p>");
    auto object = ShareRef(page.Object(u8"x"));
    page.Document->ClearAXObjectCache();
    CHECK(object->IsDetached());
    CHECK_FALSE(page.Document->ExistingAXObjectCache());
  }

  TEST_CASE("AXObjectCache: a children change rebuilds the children", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<ul id=list><li>One</li></ul>");
    auto &list = page.Object(u8"list");
    CHECK(list.Children().size() == 1);

    auto item = page.Document->CreateElement(u8"li").ReleaseValue();
    REQUIRE_FALSE(page.ById(u8"list").AppendChild(*item).HasException());
    REQUIRE_FALSE(item->AppendChild(*page.Document->CreateTextNode(u8"Two")).HasException());
    (void)page.Document->UpdateLayout();
    page.Cache().ChildrenChanged(&list);
    page.Cache().PerformDeferredCacheUpdate();
    CHECK(list.Children().size() == 2);
  }

  TEST_CASE("AXObjectCache: the focused object", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<button id=button>Go</button>");

    SECTION("the web area while nothing is focused")
    {
      auto *focused = page.Cache().FocusedObject();
      CHECK(focused == page.Cache().Get(*page.Document));
      CHECK(focused->IsRootWebArea());
    }

    SECTION("the focused element's object")
    {
      DynamicDowncast<html::HTMLElement>(page.ById(u8"button"))->Focus();
      CHECK(page.Cache().FocusedObject() == &page.Object(u8"button"));
    }
  }
}
