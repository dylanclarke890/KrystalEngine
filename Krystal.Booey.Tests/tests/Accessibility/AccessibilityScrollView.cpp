#include "Krystal.Booey/Accessibility/AccessibilityScrollView.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityScrollbar.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityScrollView: the view and its scroll bars",
            "[Accessibility][AccessibilityScrollView]")
  {
    AccessibilityFixture page(u8"<div style='height:2000px'>Tall</div>");
    auto &root = Downcast<AccessibilityScrollView>(*page.Cache().RootObject());
    CHECK(root.ParentObject() == nullptr);
    CHECK(root.ScrollView() == page.Document->View());
    auto *webArea = root.WebAreaObject();
    CHECK(webArea == page.Cache().Get(*page.Document));
    CHECK(root.ElementRect().Width() == 400);
    CHECK(root.ElementRect().Height() == 300);

    // The page scrolls down, not across.
    CHECK_FALSE(root.ScrollBar(AccessibilityOrientation::Horizontal));
    auto *scrollBar = root.ScrollBar(AccessibilityOrientation::Vertical);
    REQUIRE(scrollBar);
    REQUIRE(Is<AccessibilityScrollbar>(*scrollBar));
    CHECK(scrollBar->Role() == AccessibilityRole::ScrollBar);
    CHECK(scrollBar->ParentObject() == &root);
    CHECK(scrollBar->Orientation() == AccessibilityOrientation::Vertical);
    CHECK(scrollBar->ValueForRange() == 0);

    // The children are the web area and the scroll bar, which stays the same object.
    const auto &children = root.Children();
    REQUIRE(children.size() == 2);
    CHECK(children[0].get() == root.WebAreaObject());
    CHECK(children[1].get() == scrollBar);
    CHECK(root.ScrollBar(AccessibilityOrientation::Vertical) == scrollBar);

    // Setting the value scrolls.
    CHECK(Downcast<AccessibilityScrollbar>(*scrollBar).SetValue(1));
    CHECK(page.Document->View()->GetScrollPosition().Y() > 0);
    CHECK(scrollBar->ValueForRange() == 1);
  }

  TEST_CASE("AccessibilityScrollView: a page that doesn't scroll has no scroll bars",
            "[Accessibility][AccessibilityScrollView]")
  {
    AccessibilityFixture page(u8"<p>Short</p>");
    auto &root = *page.Cache().RootObject();
    CHECK(root.Children().size() == 1);
    CHECK_FALSE(Downcast<AccessibilityScrollView>(root).ScrollBar(AccessibilityOrientation::Vertical));
  }
}
