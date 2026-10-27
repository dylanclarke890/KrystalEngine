#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    void Click(dom::Element &target, int16 button = 0) noexcept
    {
      dom::MouseEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      init.Composed = true;
      init.Button = button;
      auto type = button == 0 ? dom::DOMStringAtom(u8"click") : dom::DOMStringAtom(u8"auxclick");
      (void)target.DispatchEvent(*CreateRef<dom::MouseEvent>(type, init));
    }

    KRYS_NODISCARD SVGAElement &AById(NavigationFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<SVGAElement>(Downcast<SVGElement>(fixture.ById(id)));
    }
  }

  TEST_CASE("SVGAElement: a click follows the link", "[SVG][SVGAElement]")
  {
    NavigationFixture fixture(
      u8R"(<svg><a id=a href="next.html"><rect id=inner width=10 height=10 /></a><a id=plain><rect /></a>
      <a id=xlink xlink:href="x.html" xlink:show=new><rect /></a>
      <a id=blank href="https://other.test/" target=_blank rel=opener><rect /></a>
      <a id=noopener href="y.html" target=results rel="noreferrer"><rect /></a></svg>)");

    SECTION("the host is asked, with the action")
    {
      Click(fixture.ById(u8"inner"));
      REQUIRE(fixture.Navigations().size() == 1);
      auto &navigation = fixture.Navigations()[0];
      CHECK(navigation.URL == "https://example.com/dir/next.html");
      CHECK_FALSE(navigation.IsNewWindow);
      CHECK(navigation.Action.Type() == NavigationType::LinkClicked);
      CHECK(navigation.Action.SourceElement() == &fixture.ById(u8"a"));
    }

    SECTION("a link without href isn't followed")
    {
      Click(fixture.ById(u8"plain"));
      CHECK(fixture.Navigations().empty());
    }

    SECTION("xlink:href, and xlink:show=new opens a new window")
    {
      Click(fixture.ById(u8"xlink"));
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].URL == "https://example.com/dir/x.html");
      CHECK(fixture.Navigations()[0].IsNewWindow);
      CHECK(fixture.Navigations()[0].NewWindowName == "_blank");
      CHECK(fixture.Navigations()[0].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Suppress);
    }

    SECTION("targets and link types")
    {
      Click(fixture.ById(u8"blank"));
      REQUIRE(fixture.Navigations().size() == 1);
      CHECK(fixture.Navigations()[0].NewWindowName == "_blank");
      CHECK(fixture.Navigations()[0].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Allow);

      Click(fixture.ById(u8"noopener"));
      REQUIRE(fixture.Navigations().size() == 2);
      CHECK(fixture.Navigations()[1].NewWindowName == "results");
      CHECK(fixture.Navigations()[1].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Suppress);
    }

    SECTION("other buttons' clicks are followed too")
    {
      Click(fixture.ById(u8"inner"), 1);
      CHECK(fixture.Navigations().size() == 1);
    }
  }

  TEST_CASE("SVGAElement: the element", "[SVG][SVGAElement]")
  {
    NavigationFixture fixture(
      u8R"(<details><summary id=summary><svg><a id=a href="x.html" target=t rel="noopener other"><rect id=r width=10 height=10 />)"
      u8R"(<a id=nested href="y.html"><rect /></a></a></svg></summary></details>)");
    (void)fixture.Document->UpdateLayout();
    auto &a = AById(fixture, u8"a");

    CHECK(a.Target() == dom::DOMStringAtom(u8"t"));
    CHECK(css::IsLink(a));
    CHECK(a.RelList().Length() == 2);
    CHECK(a.RelList().Supports(u8"noopener").Value());
    CHECK_FALSE(a.RelList().Supports(u8"other").Value());

    // It renders as a group; an <a> in an <a> isn't rendered.
    CHECK(Is<LegacyRenderSVGTransformableContainer>(a.Renderer()));
    CHECK(fixture.ById(u8"nested").Renderer() == nullptr);

    // Clicking a link in a summary follows it rather than toggling the details.
    Click(fixture.ById(u8"r"));
    CHECK(fixture.Navigations().size() == 1);
    auto &details =
      Downcast<html::HTMLDetailsElement>(*fixture.Document->QuerySelector(u8"details").ReleaseValue());
    CHECK_FALSE(details.Open());
  }
}
