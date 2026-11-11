#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include <algorithm>

namespace krys::boo::ax::tests
{
  namespace
  {
    KRYS_NODISCARD size_t
      CountPosted(const List<Pair<Ref<AccessibilityObject>, AXNotification>> &notifications,
                  const AccessibilityObject &object, AXNotification notification) noexcept
    {
      return static_cast<size_t>(
        std::ranges::count_if(notifications, [&](const auto &posted)
                              { return posted.first.get() == &object && posted.second == notification; }));
    }
  }

  TEST_CASE("AXLiveRegions: attributes", "[Accessibility][AXLiveRegions]")
  {
    AccessibilityFixture page(u8"<div id=polite aria-live=polite aria-relevant=all aria-busy=true>A</div>"
                              u8"<div id=alert role=alert>B</div>"
                              u8"<div id=timer role=timer>C</div>"
                              u8"<div id=status role=status aria-atomic=false>D</div>"
                              u8"<div id=plain>E</div>");
    auto &polite = page.Object(u8"polite");
    CHECK(polite.SupportsLiveRegion());
    CHECK(polite.LiveRegionStatus() == u8"polite");
    CHECK(polite.LiveRegionRelevant() == u8"all");
    CHECK_FALSE(polite.LiveRegionAtomic());
    CHECK(polite.IsBusy());

    auto &alert = page.Object(u8"alert");
    CHECK(alert.LiveRegionStatus() == u8"assertive");
    CHECK(alert.LiveRegionAtomic());
    CHECK(alert.LiveRegionRelevant() == u8"additions text");

    auto &timer = page.Object(u8"timer");
    CHECK_FALSE(timer.SupportsLiveRegion());
    CHECK(timer.SupportsLiveRegion(false));

    CHECK_FALSE(page.Object(u8"status").LiveRegionAtomic());
    CHECK_FALSE(page.Object(u8"plain").SupportsLiveRegion(false));
  }

  TEST_CASE("AXLiveRegions: a region's changes", "[Accessibility][AXLiveRegions]")
  {
    AccessibilityFixture page(u8"<div id=region aria-live=polite><p id=paragraph>Old</p></div>");
    auto &region = page.Object(u8"region");
    CHECK(page.Object(u8"paragraph").LiveRegionAncestor() == &region);
    (void)page.TakeNotifications();

    SECTION("Children added, once for all of them")
    {
      for (auto text : {u8"One", u8"Two"})
      {
        auto paragraph = page.Document->CreateElement(u8"p").ReleaseValue();
        REQUIRE_FALSE(paragraph->AppendChild(*page.Document->CreateTextNode(text)).HasException());
        REQUIRE_FALSE(page.ById(u8"region").AppendChild(*paragraph).HasException());
      }
      (void)page.Document->UpdateLayout();
      CHECK(CountPosted(page.TakeNotifications(), region, AXNotification::LiveRegionChanged) == 1);
    }

    SECTION("Text changed")
    {
      auto *text = DynamicDowncast<dom::Text>(page.ById(u8"paragraph").FirstChild());
      REQUIRE(text);
      REQUIRE_FALSE(text->AppendData(u8" news").HasException());
      (void)page.Document->UpdateLayout();
      CHECK(
        AccessibilityFixture::WasPosted(page.TakeNotifications(), region, AXNotification::LiveRegionChanged));
    }

    SECTION("Turned off")
    {
      REQUIRE_FALSE(page.ById(u8"region").SetAttribute(u8"aria-live", u8"off").HasException());
      auto notifications = page.TakeNotifications();
      CHECK(AccessibilityFixture::WasPosted(notifications, region, AXNotification::LiveRegionStatusChanged));
      CHECK_FALSE(region.SupportsLiveRegion());
    }
  }

  TEST_CASE("AXLiveRegions: a region being added", "[Accessibility][AXLiveRegions]")
  {
    AccessibilityFixture page(u8"<div id=container>Text</div>");
    (void)page.Object(u8"container");
    (void)page.TakeNotifications();

    auto alert = page.Document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(alert->SetAttribute(u8"role", u8"alert").HasException());
    REQUIRE_FALSE(alert->SetAttribute(u8"id", u8"alert").HasException());
    REQUIRE_FALSE(alert->AppendChild(*page.Document->CreateTextNode(u8"Saved")).HasException());
    REQUIRE_FALSE(page.ById(u8"container").AppendChild(*alert).HasException());
    (void)page.Document->UpdateLayout();
    auto notifications = page.TakeNotifications();
    CHECK(AccessibilityFixture::WasPosted(notifications, page.Object(u8"alert"),
                                          AXNotification::LiveRegionCreated));
  }

  TEST_CASE("AXLiveRegions: an editable region's value", "[Accessibility][AXLiveRegions]")
  {
    AccessibilityFixture page(u8"<div id=editor contenteditable><span id=span>Hi</span></div>");
    auto &editor = page.Object(u8"editor");
    CHECK(editor.IsNonNativeTextControl());
    (void)page.TakeNotifications();

    auto *text = DynamicDowncast<dom::Text>(page.ById(u8"span").FirstChild());
    REQUIRE(text);
    REQUIRE_FALSE(text->AppendData(u8" there").HasException());
    (void)page.Document->UpdateLayout();
    CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), editor, AXNotification::ValueChanged));
  }

  TEST_CASE("AXLiveRegions: a menu appearing", "[Accessibility][AXLiveRegions]")
  {
    AccessibilityFixture page(u8"<div id=container>Text</div>");
    (void)page.Object(u8"container");
    (void)page.TakeNotifications();

    auto menu = page.Document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(menu->SetAttribute(u8"role", u8"menu").HasException());
    REQUIRE_FALSE(menu->SetAttribute(u8"id", u8"menu").HasException());
    auto item = page.Document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(item->SetAttribute(u8"role", u8"menuitem").HasException());
    REQUIRE_FALSE(item->AppendChild(*page.Document->CreateTextNode(u8"Open")).HasException());
    REQUIRE_FALSE(menu->AppendChild(*item).HasException());
    REQUIRE_FALSE(page.ById(u8"container").AppendChild(*menu).HasException());
    (void)page.Document->UpdateLayout();
    CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), page.Object(u8"menu"),
                                          AXNotification::MenuOpened));
  }
}
