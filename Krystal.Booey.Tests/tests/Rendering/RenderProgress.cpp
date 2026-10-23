#include "Krystal.Booey/Rendering/RenderProgress.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderProgress: progress bars", "[Rendering][RenderProgress]")
  {
    // With its native appearance, a progress element is a bar the theme draws, Adwaita's 6 pixels tall;
    // without it, its shadow tree's bar and value show, the value as wide as the position.
    ResolverFixture fixture {AhemDocument(
      u8"progress{vertical-align:top}", u8"<progress id=half value=1 max=2></progress>"
                                        u8"<progress id=spinner></progress>"
                                        u8"<progress id=plain value=0.25 style=appearance:none></progress>")};
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x10
      RenderProgress {PROGRESS} at (0,0) size 100x6 id="half"
      RenderProgress {PROGRESS} at (100,0) size 100x6 id="spinner"
      RenderBlock {PROGRESS} at (200,0) size 100x10 id="plain"
        RenderProgress {DIV} at (0,0) size 100x6
          RenderBlock {DIV} at (0,0) size 100x10 [bgcolor=#808080]
            RenderBlock {DIV} at (0,0) size 25x10 [bgcolor=#008000]
)");

    auto &half = Downcast<html::HTMLProgressElement>(fixture.ById(u8"half"));
    auto &spinner = Downcast<html::HTMLProgressElement>(fixture.ById(u8"spinner"));
    CHECK(half.Position() == 0.5);
    CHECK(spinner.Position() == html::HTMLProgressElement::IndeterminatePosition);

    auto &halfRenderer = Downcast<RenderProgress>(*half.Renderer());
    auto &spinnerRenderer = Downcast<RenderProgress>(*spinner.Renderer());
    CHECK(halfRenderer.IsDeterminate());
    CHECK(halfRenderer.Position() == 0.5);
    CHECK_FALSE(spinnerRenderer.IsDeterminate());
    // The indeterminate bar animates from when it became indeterminate.
    CHECK(spinnerRenderer.AnimationStartTime() > MonotonicTime::duration::zero());

    // Giving it a value makes it determinate, and stops matching :indeterminate.

    CHECK(spinner.Matches(u8":indeterminate").ReleaseValue());
    spinner.SetAttribute(u8"value", u8"0.75");
    (void)fixture.Document->UpdateLayout();
    CHECK_FALSE(spinner.Matches(u8":indeterminate").ReleaseValue());
    CHECK(spinnerRenderer.Position() == 0.75);

    // The values clamp: value to [0, max], max to positive numbers.
    half.SetAttribute(u8"value", u8"5");
    CHECK(half.Value() == 2);
    half.SetAttribute(u8"max", u8"-1");
    CHECK(half.Max() == 1);
    CHECK(half.Value() == 1);
  }
}
