#include "Krystal.Booey/Rendering/RenderTextControl.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTextControl: fields and text areas",
                   "[Rendering][RenderTextControl]")
  {
    // A field is as wide as its size in average characters (and a pixel for the caret), and its baseline is
    // its text's; a text area is its cols wide (and a scrollbar) and its rows tall, and its baseline is its
    // bottom. The placeholder is drawn in the inner text's place.
    ResolverFixture fixture {AhemDocument(u8"", u8"<input id=field value=abc>x<textarea id=area>xy</textarea>"
                                                u8"<input id=empty placeholder=Name size=5>")};
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x31
  RenderBlock {HTML} at (0,0) size 800x31
    RenderBody {BODY} at (0,0) size 800x31
      RenderTextControl {INPUT} at (0,15) size 207x16 [bgcolor=#FFFFFF] [border: (2px inset #808080)] id="field"
      RenderText {#text} at (207,18) size 10x10
        text run at (207,18) width 10: "x"
      RenderTextControl {INPUT} at (444,15) size 57x16 [bgcolor=#FFFFFF] [border: (2px inset #808080)] id="empty"
layer at (3,18) size 201x10
  RenderBlock at (3,3) size 201x10
    RenderText {#text} at (0,0) size 30x10
      text run at (0,0) width 30: "abc"
layer at (217,0) size 227x26 clip at (218,1) size 225x24
  RenderTextControl {TEXTAREA} at (217,0) size 227x26 [bgcolor=#FFFFFF] [border: (1px solid #000000)] id="area"
    RenderBlock at (3,3) size 221x10
      RenderText {#text} at (0,0) size 20x10
        text run at (0,0) width 20: "xy"
layer at (447,18) size 51x10
  RenderBlock at (3,3) size 51x10 [color=#A9A9A9]
    RenderText {#text} at (0,0) size 40x10
      text run at (0,0) width 40: "Name"
layer at (447,18) size 51x10
  RenderBlock at (3,3) size 51x10
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTextControl: clicking in a field puts the caret there",
                   "[Rendering][RenderTextControl]")
  {
    RegisterAhem();
    boo::tests::EditorFixture fixture(u8"<style>input{font:10px Ahem}</style><input id=field value=abcdef>");
    auto &field = Downcast<html::HTMLInputElement>(fixture.ById(u8"field"));
    auto &handler = fixture.Frame->GetEventHandler();

    // The text starts 3 pixels in (the border and padding); a click on the border goes to the inner text.
    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {24, 8}));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {24, 8}));
    CHECK(fixture.Document->FocusedElement() == &field);
    CHECK(field.SelectionStart() == 2);
    CHECK(field.SelectionEnd() == 2);

    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {1, 8}));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {1, 8}));
    CHECK(field.SelectionStart() == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTextControl: dragging a resizer resizes the box",
                   "[Rendering][RenderTextControl]")
  {
    boo::tests::EditorFixture fixture(
      u8"<div id=box style=overflow:auto;resize:both;width:100px;height:50px;min-height:40px></div>");
    auto &box = fixture.ById(u8"box");
    auto &handler = fixture.Frame->GetEventHandler();
    auto *layer = Downcast<RenderBox>(box.Renderer())->Layer();
    REQUIRE(layer->CanResize());

    // The resizer is the bottom right corner, a scrollbar's thickness square.
    CHECK(layer->IsPointInResizeControl({95, 45}));
    CHECK_FALSE(layer->IsPointInResizeControl({50, 25}));
    CHECK(handler.CurrentMouseCursor().GetType() != Cursor::Type::SouthEastResize);
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {95, 45}));
    CHECK(handler.CurrentMouseCursor().GetType() == Cursor::Type::SouthEastResize);

    (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, {95, 45}));
    CHECK(layer->InResizeMode());
    (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, {125, 35}));
    (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, {125, 35}));
    CHECK_FALSE(layer->InResizeMode());

    // It's wider, and no shorter than its minimum height.
    CHECK(boo::tests::Narrow(box.GetAttribute(u8"style").value_or(u8"")).find("width: 130px")
          != std::string::npos);
    CHECK(boo::tests::Narrow(box.GetAttribute(u8"style").value_or(u8"")).find("height: 40px")
          != std::string::npos);
    CHECK(Downcast<RenderBox>(box.Renderer())->Width() == 130);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTextControl: text areas resize when the settings say so",
                   "[Rendering][RenderTextControl]")
  {
    boo::tests::EditorFixture fixture(u8"<textarea id=area></textarea>");
    auto &area = fixture.ById(u8"area");
    CHECK(area.Renderer()->Style().GetResize() == style::Resize::None);

    auto settings = Settings::Create();
    settings->TextAreasAreResizable = true;
    fixture.Document->SetSettings(krys::move(settings));
    fixture.Document->ScheduleFullStyleRebuild();
    (void)fixture.Document->UpdateLayout();
    CHECK(area.Renderer()->Style().GetResize() == style::Resize::Both);
    CHECK(Downcast<RenderBox>(area.Renderer())->Layer()->CanResize());
  }
}
