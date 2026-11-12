#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityImageMap: an image map's areas", "[Accessibility][AccessibilityImageMap]")
  {
    AccessibilityFixture page(u8"<img id=image usemap=#map width=100 height=100 alt=Shapes>"
                              u8"<map id=map name=map>"
                              u8"<area id=first href=#first shape=rect coords=0,0,50,50 alt=First>"
                              u8"<area id=plain shape=rect coords=50,0,100,50 alt=Plain>"
                              u8"</map>");

    auto &image = page.Object(u8"image");
    CHECK(image.Role() == AccessibilityRole::ImageMap);

    // The areas that are links are the image's children (not the map's).
    auto &first = page.Object(u8"first");
    REQUIRE(image.Children().size() == 1);
    CHECK(image.Children()[0].get() == &first);
    CHECK(first.IsImageMapLink());
    CHECK(first.Role() == AccessibilityRole::Link);
    CHECK(first.ParentObject() == &image);
    CHECK(first.AccessibleName() == u8"First");
    CHECK(first.IsEnabled());
    CHECK_FALSE(first.IsIgnored());

    auto rect = first.ElementRect();
    CHECK(rect.Width() == 50);
    CHECK(rect.Height() == 50);
    CHECK(image.ElementRect().Contains(rect));

    CHECK(page.Object(u8"plain").Role() == AccessibilityRole::Generic);
  }

  TEST_CASE("AccessibilityImageMap: a focused area is the image's child",
            "[Accessibility][AccessibilityImageMap]")
  {
    AccessibilityFixture page(
      u8"<img id=image usemap=#map width=100 height=100 alt=Shapes>"
      u8"<map name=map><area id=area href=#a shape=rect coords=0,0,50,50 alt=A></map>");
    Downcast<html::HTMLElement>(page.ById(u8"area")).Focus();
    auto *focused = page.Cache().FocusedObject();
    REQUIRE(focused);
    CHECK(focused->GetNode() == &page.ById(u8"area"));
    CHECK(focused->ParentObject() == &page.Object(u8"image"));
  }
}
