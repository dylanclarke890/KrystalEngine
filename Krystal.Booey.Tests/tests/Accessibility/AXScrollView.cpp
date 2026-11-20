#include "Krystal.Booey/Accessibility/AXScrollView.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXScrollbar.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXScrollView: the view and its scroll bars", "[Accessibility][AXScrollView]")
  {
    AXFixture page(u8"<div style='height:2000px'>Tall</div>");
    auto &root = Downcast<AXScrollView>(*page.Cache().RootObject());
    CHECK(root.ParentObject() == nullptr);
    CHECK(root.ScrollView() == page.Document->View());
    auto *webArea = root.WebAreaObject();
    CHECK(webArea == page.Cache().Get(*page.Document));
    CHECK(root.ElementRect().Width() == 400);
    CHECK(root.ElementRect().Height() == 300);

    // The page scrolls down, not across.
    CHECK_FALSE(root.ScrollBar(AXOrientation::Horizontal));
    auto *scrollBar = root.ScrollBar(AXOrientation::Vertical);
    REQUIRE(scrollBar);
    REQUIRE(Is<AXScrollbar>(*scrollBar));
    CHECK(scrollBar->Role() == AXRole::ScrollBar);
    CHECK(scrollBar->ParentObject() == &root);
    CHECK(scrollBar->Orientation() == AXOrientation::Vertical);
    CHECK(scrollBar->ValueForRange() == 0);

    // The children are the web area and the scroll bar, which stays the same object.
    const auto &children = root.Children();
    REQUIRE(children.size() == 2);
    CHECK(children[0].get() == root.WebAreaObject());
    CHECK(children[1].get() == scrollBar);
    CHECK(root.ScrollBar(AXOrientation::Vertical) == scrollBar);

    // Setting the value scrolls.
    CHECK(Downcast<AXScrollbar>(*scrollBar).SetValue(1));
    CHECK(page.Document->View()->GetScrollPosition().Y() > 0);
    CHECK(scrollBar->ValueForRange() == 1);
  }

  TEST_CASE("AXScrollView: a page that doesn't scroll has no scroll bars", "[Accessibility][AXScrollView]")
  {
    AXFixture page(u8"<p>Short</p>");
    auto &root = *page.Cache().RootObject();
    CHECK(root.Children().size() == 1);
    CHECK_FALSE(Downcast<AXScrollView>(root).ScrollBar(AXOrientation::Vertical));
  }
}
