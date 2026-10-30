#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;
  using GaugeRegion = html::HTMLMeterElement::GaugeRegion;

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMeterElement: values and gauge regions",
                   "[HTML][HTMLMeterElement]")
  {
    ResolverFixture fixture {
      AhemDocument(u8"meter{vertical-align:top}",
                   u8"<meter id=meter value=0.7></meter>"
                   u8"<meter id=low min=0 max=100 low=20 high=80 optimum=90 value=10></meter>")};

    // Adwaita doesn't draw meters, so the shadow tree's bar and value show, the value as wide as its ratio
    // and in the colour of its region.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x10
      RenderBlock {METER} at (0,0) size 50x10 id="meter"
        RenderBlock {DIV} at (0,0) size 50x10 id="inner"
          RenderBlock {DIV} at (0,0) size 50x10 id="bar"
            RenderBlock {DIV} at (0,0) size 35x10 id="value" class="optimum"
      RenderBlock {METER} at (50,0) size 50x10 id="low"
        RenderBlock {DIV} at (0,0) size 50x10 id="inner"
          RenderBlock {DIV} at (0,0) size 50x10 id="bar"
            RenderBlock {DIV} at (0,0) size 5x10 id="value" class="even-less-good"
)");

    auto &meter = Downcast<html::HTMLMeterElement>(fixture.ById(u8"meter"));
    CHECK(meter.Min() == 0);
    CHECK(meter.Max() == 1);
    CHECK(meter.Optimum() == 0.5);
    CHECK(meter.GetGaugeRegion() == GaugeRegion::Optimum);

    auto &low = Downcast<html::HTMLMeterElement>(fixture.ById(u8"low"));
    CHECK(low.GetGaugeRegion() == GaugeRegion::EvenLessGood);
    low.SetAttribute(u8"value", u8"50");
    CHECK(low.GetGaugeRegion() == GaugeRegion::Suboptimal);
    low.SetAttribute(u8"value", u8"85");
    CHECK(low.GetGaugeRegion() == GaugeRegion::Optimum);

    // The values clamp to the range, and low and high to each other.
    low.SetAttribute(u8"value", u8"150");
    CHECK(low.Value() == 100);
    low.SetAttribute(u8"high", u8"10");
    CHECK(low.High() == 20);
    CHECK(low.ValueRatio() == 1);
  }
}
