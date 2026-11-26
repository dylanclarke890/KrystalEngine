#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXRenderObject.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXImageMap: an image map's areas", "[Accessibility][AXImageMap]")
  {
    AXFixture page(u8"<img id=image usemap=#map width=100 height=100 alt=Shapes>"
                   u8"<map id=map name=map>"
                   u8"<area id=first href=#first shape=rect coords=0,0,50,50 alt=First>"
                   u8"<area id=plain shape=rect coords=50,0,100,50 alt=Plain>"
                   u8"</map>");

    auto &image = page.Object(u8"image");
    CHECK(image.Role() == AXRole::ImageMap);

    // The areas that are links are the image's children (not the map's).
    auto &first = page.Object(u8"first");
    REQUIRE(image.Children().size() == 1);
    CHECK(image.Children()[0].get() == &first);
    CHECK(first.IsImageMapLink());
    CHECK(first.Role() == AXRole::Link);
    CHECK(first.ParentObject() == &image);
    CHECK(first.AccessibleName() == u8"First");
    CHECK(first.IsEnabled());
    CHECK_FALSE(first.IsIgnored());

    auto rect = first.ElementRect();
    CHECK(rect.Width() == 50);
    CHECK(rect.Height() == 50);
    CHECK(image.ElementRect().Contains(rect));

    CHECK(page.Object(u8"plain").Role() == AXRole::Generic);
  }

  TEST_CASE("AXImageMap: an area's frame follows the image's transform", "[Accessibility][AXImageMap]")
  {
    AXFixture page(u8"<img id=image usemap=#map width=100 height=100 alt=Shapes"
                   u8" style='transform: translateX(200px) scale(2); transform-origin: 0 0'>"
                   u8"<map name=map><area id=first href=#first shape=rect coords=0,0,50,50 alt=First></map>");

    auto image = page.Object(u8"image").ElementRect();
    auto rect = page.Object(u8"first").ElementRect();
    CHECK(rect.X() == image.X());
    CHECK(rect.Y() == image.Y());
    CHECK(rect.Width() == 100);
    CHECK(rect.Height() == 100);
  }

  TEST_CASE("AXImageMap: a focused area is the image's child", "[Accessibility][AXImageMap]")
  {
    AXFixture page(u8"<img id=image usemap=#map width=100 height=100 alt=Shapes>"
                   u8"<map name=map><area id=area href=#a shape=rect coords=0,0,50,50 alt=A></map>");
    Downcast<HTMLElement>(page.ById(u8"area")).Focus();
    auto *focused = page.Cache().FocusedObject();
    REQUIRE(focused);
    CHECK(focused->GetNode() == &page.ById(u8"area"));
    CHECK(focused->ParentObject() == &page.Object(u8"image"));
  }
}
