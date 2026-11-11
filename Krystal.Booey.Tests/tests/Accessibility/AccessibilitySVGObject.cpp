#include "Krystal.Booey/Accessibility/AccessibilitySVGObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilitySVGObject: roles and inclusion", "[Accessibility][AccessibilitySVGObject]")
  {
    AccessibilityFixture page(u8"<svg id=root width=200 height=100>"
                              u8"<rect id=plain width=10 height=10 />"
                              u8"<rect id=titled width=10 height=10><title>Box</title></rect>"
                              u8"<circle id=focusable tabindex=0 r=5 cx=50 cy=50 />"
                              u8"<g id=group aria-label=Shapes><rect id=grouped width=10 height=10 /></g>"
                              u8"<g id=generic><rect width=10 height=10 /></g>"
                              u8"<text id=text x=10 y=50>Hello</text>"
                              u8"<a id=link href=#x><text>Go</text></a>"
                              u8"<rect id=presentational role=presentation width=10 height=10 />"
                              u8"<rect id=described role=none width=10 height=10><desc>Kept</desc></rect>"
                              u8"</svg>");

    auto &root = page.Object(u8"root");
    REQUIRE(Is<AccessibilitySVGObject>(root));
    CHECK(root.IsAccessibilitySVGRoot());
    CHECK(root.Role() == AccessibilityRole::Generic);

    // Shapes are left out unless something says to include them.
    CHECK(page.Object(u8"plain").IsIgnored());
    CHECK(page.Object(u8"titled").Role() == AccessibilityRole::Image);
    CHECK_FALSE(page.Object(u8"titled").IsIgnored());
    CHECK_FALSE(page.Object(u8"focusable").IsIgnored());
    CHECK_FALSE(page.Object(u8"grouped").IsIgnored());

    CHECK(page.Object(u8"group").Role() == AccessibilityRole::Group);
    CHECK(page.Object(u8"generic").Role() == AccessibilityRole::Generic);
    CHECK(page.Object(u8"text").Role() == AccessibilityRole::SVGText);
    CHECK_FALSE(page.Object(u8"text").IsIgnored());
    CHECK(page.Object(u8"link").Role() == AccessibilityRole::Link);

    CHECK(page.Object(u8"presentational").IsIgnored());
    // A desc child trumps a presentational role.
    CHECK(page.Object(u8"described").Role() == AccessibilityRole::Image);
    CHECK_FALSE(page.Object(u8"described").IsIgnored());
  }

  TEST_CASE("AccessibilitySVGObject: names and descriptions", "[Accessibility][AccessibilitySVGObject]")
  {
    AccessibilityFixture page(
      u8"<svg width=200 height=100>"
      u8"<rect id=titled width=10 height=10><title>Box</title><desc>A square</desc></rect>"
      u8"<rect id=languages lang=fr width=10 height=10>"
      u8"<title lang=en>Box</title><title lang=fr-CA>Boîte</title><title>Any</title></rect>"
      u8"<rect id=labelled aria-label=Label width=10 height=10><title>Tip</title></rect>"
      u8"<a id=link href=#x xlink:title=Onward><text>Go</text></a>"
      u8"<defs><rect id=shape width=10 height=10><title>Shape</title></rect></defs>"
      u8"<use id=use href=#shape />"
      u8"</svg>");

    CHECK(page.Object(u8"titled").AccessibleName() == u8"Box");
    CHECK(page.Object(u8"titled").AccessibleDescription() == u8"A square");
    // The title in the element's language (by its primary subtag here).
    CHECK(page.Object(u8"languages").AccessibleName() == u8"Boîte");
    // A title that isn't the name is the description.
    CHECK(page.Object(u8"labelled").AccessibleName() == u8"Label");
    CHECK(page.Object(u8"labelled").AccessibleDescription() == u8"Tip");
    CHECK(page.Object(u8"link").AccessibleName() == u8"Onward");
    CHECK(page.Object(u8"use").AccessibleName() == u8"Shape");
  }
}
