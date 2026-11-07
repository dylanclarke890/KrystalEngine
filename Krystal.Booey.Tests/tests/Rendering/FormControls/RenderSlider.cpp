#include "Krystal.Booey/Rendering/FormControls/RenderSlider.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Controls/SliderTrackPart.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(
        point, point, type == PlatformEvent::Type::MouseMoved ? MouseButton::None : MouseButton::Left, type,
        clickCount, {}, MonotonicTime::Now());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSlider: the thumb sits at the value along the track",
                   "[Rendering][RenderSlider]")
  {
    // The default track is 129 pixels and Adwaita's thumb 20, so a quarter of the way along is at 27, and the
    // ends are at 0 and 109 (the dump rounds the quarter's thumb to 21 pixels).
    ResolverFixture fixture {AhemDocument(u8"input{margin:0}",
                                          u8"<input type=range id=quarter value=25><br>"
                                          u8"<input type=range id=low min=10 max=20 value=0><br>"
                                          u8"<input type=range id=high value=100>")};
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x66
  RenderBlock {HTML} at (0,0) size 800x66
    RenderBody {BODY} at (0,0) size 800x66
      RenderSlider {INPUT} at (0,0) size 129x20 [color=#909090] [bgcolor=#FFFFFF] id="quarter"
        RenderFlexibleBox {DIV} at (0,0) size 129x20
          RenderBlock {DIV} at (0,0) size 129x20
            RenderBlock {DIV} at (27,0) size 21x20
      RenderBR {BR} at (129,12) size 0x10
      RenderSlider {INPUT} at (0,22) size 129x20 [color=#909090] [bgcolor=#FFFFFF] id="low"
        RenderFlexibleBox {DIV} at (0,0) size 129x20
          RenderBlock {DIV} at (0,0) size 129x20
            RenderBlock {DIV} at (0,0) size 20x20
      RenderBR {BR} at (129,34) size 0x10
      RenderSlider {INPUT} at (0,44) size 129x20 [color=#909090] [bgcolor=#FFFFFF] id="high"
        RenderFlexibleBox {DIV} at (0,0) size 129x20
          RenderBlock {DIV} at (0,0) size 129x20
            RenderBlock {DIV} at (109,0) size 20x20
)");

    // The thumb is drawn by the theme, and the track knows where it is.
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"quarter"));
    CHECK(input.Renderer()->Style().UsedAppearance() == StyleAppearance::SliderHorizontal);
    CHECK(input.SliderThumbElement()->Renderer()->Style().UsedAppearance()
          == StyleAppearance::SliderThumbHorizontal);
    CHECK(Downcast<RenderSlider>(*input.Renderer()).ValueRatio() == 0.25);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSlider: vertical sliders", "[Rendering][RenderSlider]")
  {
    // `appearance: slider-vertical` lays the track out vertically, the default track length tall, with the
    // minimum at the bottom.
    ResolverFixture fixture {AhemDocument(u8"input{margin:0;appearance:slider-vertical;width:20px}",
                                          u8"<input type=range id=range value=100>")};
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"range"));
    CHECK(input.Renderer()->Style().UsedAppearance() == StyleAppearance::SliderVertical);
    auto *thumb = input.SliderThumbElement()->GetRenderBox();
    CHECK(thumb->Location().Y() == 0);
    input.SetValue(u8"0");
    (void)fixture.Document->UpdateLayout();
    CHECK(thumb->Location().Y() + thumb->Height() == input.SliderTrackElement()->GetRenderBox()->Height());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSlider: dragging the thumb and clicking the track",
                   "[Rendering][RenderSlider]")
  {
    boo::tests::EditorFixture fixture(
      u8"<style>body{margin:0}input{margin:0}</style><input type=range id=range "
      u8"value=50>");
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"range"));
    auto &handler = fixture.Frame->GetEventHandler();
    auto inputs = fixture.Listen(EventNames::Input);
    auto changes = fixture.Listen(EventNames::Change);

    // Pressing on the thumb and dragging it moves it with the mouse, firing input; releasing fires change.
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {64, 10}));
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {10, 10}));
    CHECK(input.Value() == u8"0");
    CHECK(inputs->Count >= 1);
    CHECK(changes->Count == 0);
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {300, 10}));
    CHECK(input.Value() == u8"100");
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {300, 10}));
    CHECK(changes->Count == 1);

    // Moving afterwards doesn't drag.
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {10, 10}));
    CHECK(input.Value() == u8"100");

    // Clicking the track jumps the thumb there and starts a drag.
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {37, 10}));
    CHECK(input.Value() == u8"25");
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {37, 10}));

    // A disabled range doesn't move.
    input.SetAttribute(u8"disabled", u8"");
    (void)fixture.Document->UpdateLayout();
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {100, 10}));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {100, 10}));
    CHECK(input.Value() == u8"25");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSlider: a datalist's ticks", "[Rendering][RenderSlider]")
  {
    boo::tests::EditorFixture fixture(
      u8"<style>body{margin:0}input{margin:0;width:200px;display:block}</style>"
      u8"<input type=range id=range list=d><input type=range id=plain>"
      u8"<datalist id=d><option value=50><option value=20><option value=x><option disabled "
      u8"value=70></datalist>");
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"range"));
    auto &plain = Downcast<html::HTMLInputElement>(fixture.ById(u8"plain"));
    auto &handler = fixture.Frame->GetEventHandler();

    SECTION("A slider with ticks is taller, to fit them")
    {
      auto height = Downcast<RenderBox>(*input.Renderer()).Height();
      CHECK(height > Downcast<RenderBox>(*plain.Renderer()).Height());
      CHECK(height == 32_lu);
    }

    SECTION("The thumb snaps to a tick within 5 pixels")
    {
      // The value 50 is at the track's middle.
      (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {100, 16}));
      CHECK(input.Value() == u8"50");
      (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {104, 16}));
      CHECK(input.Value() == u8"50");
      (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {110, 16}));
      CHECK(input.Value() != u8"50");
      (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {110, 16}));
    }

    SECTION("Changing the datalist changes the ticks")
    {
      REQUIRE_FALSE(fixture.ById(u8"d").InnerHTML(u8"").HasException());
      html::EventLoop::Current().RunUntilIdle();
      (void)fixture.Document->UpdateLayout();
      CHECK(Downcast<RenderBox>(*input.Renderer()).Height() == 32_lu);
      (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {100, 16}));
      (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {104, 16}));
      CHECK(input.Value() != u8"50");
      (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {104, 16}));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderSlider: a datalist's tick positions",
                   "[Rendering][RenderSlider]")
  {
    // The track draws a tick for each suggestion with a valid value, at its value's place along the range.
    boo::tests::EditorFixture fixture(u8"<input type=range id=range list=d min=0 max=200>"
                                      u8"<datalist id=d><option value=50><option value=20><option value=x>"
                                      u8"<option disabled value=70></datalist>");
    auto &renderer = *fixture.ById(u8"range").Renderer();
    auto part = RenderTheme::Singleton().CreateControlPart(renderer);
    REQUIRE(part);
    RenderTheme::Singleton().UpdateControlPartForRenderer(*part, renderer);
    CHECK(Downcast<SliderTrackPart>(*part).TickRatios() == List<double> {0.25, 0.1});
  }
}
