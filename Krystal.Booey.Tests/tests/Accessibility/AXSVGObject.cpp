#include "Krystal.Booey/Accessibility/AXSVGObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/Accessibility/AXScrollView.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXSVGObject: roles and inclusion", "[Accessibility][AXSVGObject]")
  {
    AXFixture page(u8"<svg id=root width=200 height=100>"
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
    REQUIRE(Is<AXSVGObject>(root));
    CHECK(root.IsAccessibilitySVGRoot());
    CHECK(root.Role() == AXRole::Generic);

    // Shapes are left out unless something says to include them.
    CHECK(page.Object(u8"plain").IsIgnored());
    CHECK(page.Object(u8"titled").Role() == AXRole::Image);
    CHECK_FALSE(page.Object(u8"titled").IsIgnored());
    CHECK_FALSE(page.Object(u8"focusable").IsIgnored());
    CHECK_FALSE(page.Object(u8"grouped").IsIgnored());

    CHECK(page.Object(u8"group").Role() == AXRole::Group);
    CHECK(page.Object(u8"generic").Role() == AXRole::Generic);
    CHECK(page.Object(u8"text").Role() == AXRole::SVGText);
    CHECK_FALSE(page.Object(u8"text").IsIgnored());
    CHECK(page.Object(u8"link").Role() == AXRole::Link);

    CHECK(page.Object(u8"presentational").IsIgnored());
    // A desc child trumps a presentational role.
    CHECK(page.Object(u8"described").Role() == AXRole::Image);
    CHECK_FALSE(page.Object(u8"described").IsIgnored());
  }

  TEST_CASE("AXSVGObject: names and descriptions", "[Accessibility][AXSVGObject]")
  {
    AXFixture page(u8"<svg width=200 height=100>"
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

  namespace
  {
    /// @brief The first object under @p object (itself included) with @p role, if any.
    KRYS_NODISCARD AXObject *FindRole(AXObject &object, AXRole role) noexcept
    {
      if (object.Role() == role)
      {
        return &object;
      }
      for (const auto &child : object.Children())
      {
        if (auto *found = FindRole(*child, role))
        {
          return found;
        }
      }
      return nullptr;
    }
  }

  TEST_CASE("AXSVGObject: an SVG image's content is the image's", "[Accessibility][AXSVGObject]")
  {
    boo::tests::LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///chart.svg",
                            u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="80" height="40">)svg"
                            u8R"svg(<title>Chart</title><text x="2" y="20">Sales</text></svg>)svg",
                            u8"image/svg+xml");
    loader.Strategy.AddText(u8"file:///plain.svg",
                            u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="40" height="20">)svg"
                            u8R"svg(<rect width="40" height="20" fill="green"/></svg>)svg",
                            u8"image/svg+xml");
    AXFixture page(
      u8"<body style='margin: 0'><div style='height: 30px'></div>"
      u8"<img id=chart src='file:///chart.svg'><img id=plain alt=Plain src='file:///plain.svg'>");
    boo::tests::RunNetworkingTasks();
    (void)page.Document->UpdateLayout();

    // An image of an SVG document with something to expose is that document's root, whose objects are its
    // children (in the SVG document's own cache).
    auto &chart = page.Object(u8"chart");
    CHECK(chart.Role() == AXRole::SVGRoot);
    const auto &children = chart.Children();
    REQUIRE(children.size() == 1);
    auto &root = *children.front();
    CHECK(root.IsAccessibilitySVGRoot());
    CHECK(root.ParentObject() == &chart);
    CHECK(root.AxObjectCache() != &page.Cache());
    CHECK(root.AccessibleName() == u8"Chart");

    // Its objects are where the image is.
    auto *text = FindRole(root, AXRole::StaticText);
    REQUIRE(text);
    CHECK(text->ElementRect().Y() >= 30);
    auto &webArea = *Downcast<AXScrollView>(*page.Cache().RootObject()).WebAreaObject();
    auto rect = text->ElementRect();
    auto *hit = webArea.AccessibilityHitTest(
      IntPoint((rect.X() + rect.Width() / 2).ToInt(), (rect.Y() + rect.Height() / 2).ToInt()));
    REQUIRE(hit);
    CHECK(hit->AxObjectCache() == root.AxObjectCache());
    CHECK((hit == text || hit->ParentObjectUnignored() == text));

    // One without stays an image.
    CHECK(page.Object(u8"plain").Role() == AXRole::Image);
  }

  TEST_CASE("AXSVGObject: SVG elements' bounds", "[Accessibility][AXSVGObject]")
  {
    AXFixture page(u8"<body style='margin: 0'><svg width=200 height=100>"
                   u8"<text id=text x=10 y=50 font-size=16>Hello</text></svg>");

    // An SVG text's bounds are where its text is, and hit testing finds it there.
    auto &text = page.Object(u8"text");
    auto rect = text.ElementRect();
    CHECK(rect.X() >= 9);
    CHECK(rect.Y() >= 30);
    CHECK(rect.MaxY() <= 60);
    auto &webArea = *Downcast<AXScrollView>(*page.Cache().RootObject()).WebAreaObject();
    auto *hit = webArea.AccessibilityHitTest(
      IntPoint((rect.X() + rect.Width() / 2).ToInt(), (rect.Y() + rect.Height() / 2).ToInt()));
    REQUIRE(hit);
    CHECK((hit == &text || hit->ParentObjectUnignored() == &text));
  }
}
