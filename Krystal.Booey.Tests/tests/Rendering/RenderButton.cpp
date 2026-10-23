#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderButton: buttons and button inputs",
                   "[Rendering][RenderButton]")
  {
    // A button's children and a button input's value go in an anonymous block, centred in the button.
    ResolverFixture fixture {
      AhemDocument(u8"button,input{padding:2px 4px}",
                   u8"<button id=button>Go</button><input type=submit id=submit value=Send>"
                   u8"<button id=empty></button>")};
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x18
  RenderBlock {HTML} at (0,0) size 800x18
    RenderBody {BODY} at (0,0) size 800x18
      RenderButton {BUTTON} at (0,0) size 32x18 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)] id="button"
        RenderBlock (anonymous) at (6,4) size 20x10
          RenderText {#text} at (0,0) size 20x10
            text run at (0,0) width 20: "Go"
      RenderButton {INPUT} at (32,0) size 52x18 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)] id="submit"
        RenderBlock (anonymous) at (6,4) size 40x10
          RenderTextFragment at (0,0) size 40x10
            text run at (0,0) width 40: "Send"
      RenderButton {BUTTON} at (84,4) size 12x8 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)] id="empty"
)");

    // Changing the value changes the text.
    auto &submit = Downcast<html::HTMLInputElement>(fixture.ById(u8"submit"));
    submit.SetAttribute(u8"value", u8"Go");
    (void)fixture.Document->UpdateLayout();
    auto &button = Downcast<RenderButton>(*submit.Renderer());
    CHECK(button.Text() == u8"Go");
    CHECK(button.HasControlClip());
    CHECK_FALSE(button.CanHaveGeneratedChildren());
    CHECK_FALSE(Downcast<RenderButton>(*fixture.ById(u8"button").Renderer()).HasControlClip());
  }
}
