#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::ValueId;
  using style::ColorOptions;

  namespace
  {
    // The colour's sRGB components, without its flags (the theme's colours are semantic).
    KRYS_NODISCARD SRGBA<uint8> Components(const Color &color) noexcept
    {
      return color.ToColorTypeLossy<SRGBA<uint8>>();
    }

    KRYS_NODISCARD SRGBA<uint8> SystemColor(ValueId keyword, ColorOptions options) noexcept
    {
      return Components(RenderTheme::Singleton().SystemColor(keyword, options));
    }
  }

  TEST_CASE("RenderTheme: system colours", "[Rendering][RenderTheme]")
  {
    SECTION("CSS Color 4 system colours, Adwaita's where it has its own")
    {
      CHECK(SystemColor(ValueId::Canvas, ColorOptions::None) == SRGBA<uint8> {255, 255, 255});
      CHECK(SystemColor(ValueId::Canvas, ColorOptions::UseDarkAppearance) == SRGBA<uint8> {30, 30, 30});
      CHECK(SystemColor(ValueId::Canvastext, ColorOptions::None) == SRGBA<uint8> {0, 0, 0});
      CHECK(SystemColor(ValueId::Canvastext, ColorOptions::UseDarkAppearance)
            == SRGBA<uint8> {255, 255, 255});
      CHECK(SystemColor(ValueId::Buttonface, ColorOptions::None) == Components(Color::lightGray));
      CHECK(SystemColor(ValueId::Buttontext, ColorOptions::None) == SRGBA<uint8> {0, 0, 0, 204});
      CHECK(SystemColor(ValueId::Highlight, ColorOptions::None) == SRGBA<uint8> {52, 132, 228});
      CHECK(SystemColor(ValueId::Mark, ColorOptions::None) == Components(Color::yellow));
      CHECK(SystemColor(ValueId::Graytext, ColorOptions::None) == SRGBA<uint8> {0, 0, 0, 102});
      CHECK(SystemColor(ValueId::Field, ColorOptions::UseDarkAppearance) == SRGBA<uint8> {45, 45, 45});
      CHECK(SystemColor(ValueId::Accentcolor, ColorOptions::None) == SRGBA<uint8> {0, 122, 255});
      CHECK(SystemColor(ValueId::Text, ColorOptions::None) == SRGBA<uint8> {0, 0, 0});
    }

    SECTION("Links depend on the appearance and visited state")
    {
      CHECK(SystemColor(ValueId::Linktext, ColorOptions::None) == SRGBA<uint8> {0, 0, 238});
      CHECK(SystemColor(ValueId::Linktext, ColorOptions::UseDarkAppearance) == SRGBA<uint8> {158, 158, 255});
      CHECK(SystemColor(ValueId::Visitedtext, ColorOptions::None) == SRGBA<uint8> {85, 26, 139});
      CHECK(SystemColor(ValueId::Activetext, ColorOptions::None) == Components(Color::red));
      CHECK(SystemColor(ValueId::WebkitActivelink, ColorOptions::UseDarkAppearance)
            == SRGBA<uint8> {255, 158, 158});

      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::None)
            == SystemColor(ValueId::Linktext, ColorOptions::None));
      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::ForVisitedLink)
            == SystemColor(ValueId::Visitedtext, ColorOptions::None));
      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::ForVisitedLink | ColorOptions::UseDarkAppearance)
            == SystemColor(ValueId::Visitedtext, ColorOptions::UseDarkAppearance));
    }

    SECTION("Deprecated system colours map to their replacements")
    {
      CHECK(SystemColor(ValueId::Activeborder, ColorOptions::None)
            == SystemColor(ValueId::Buttonborder, ColorOptions::None));
      CHECK(SystemColor(ValueId::Buttonshadow, ColorOptions::None)
            == SystemColor(ValueId::Buttonface, ColorOptions::None));
      CHECK(SystemColor(ValueId::Inactivecaptiontext, ColorOptions::None)
            == SystemColor(ValueId::Graytext, ColorOptions::None));
      CHECK(SystemColor(ValueId::Menu, ColorOptions::None)
            == SystemColor(ValueId::Canvas, ColorOptions::None));
      CHECK(SystemColor(ValueId::Windowtext, ColorOptions::None)
            == SystemColor(ValueId::Canvastext, ColorOptions::None));
    }

    SECTION("Anything else is the invalid colour")
    {
      auto &theme = RenderTheme::Singleton();
      CHECK_FALSE(theme.SystemColor(ValueId::Red, ColorOptions::None).IsValid());
      CHECK_FALSE(theme.SystemColor(ValueId::Currentcolor, ColorOptions::None).IsValid());
      CHECK_FALSE(theme.SystemColor(ValueId::InternalDocumentTextColor, ColorOptions::None).IsValid());
    }
  }

  TEST_CASE("RenderTheme: selection and focus ring colours", "[Rendering][RenderTheme]")
  {
    auto &theme = RenderTheme::Singleton();

    // The accent colour at 30% over white for the active selection; a translucent grey for the inactive one.
    CHECK(Components(theme.ActiveSelectionBackgroundColor(ColorOptions::None))
          == Components(theme.TransformSelectionBackgroundColor(
            Color(SRGBA<uint8> {52, 132, 228}).ColorWithAlphaMultipliedBy(0.3f), ColorOptions::None)));
    CHECK_FALSE(theme.ActiveSelectionForegroundColor(ColorOptions::None).IsValid());

    // The focus ring is the accent colour at 80%.
    CHECK(Components(theme.FocusRingColor(ColorOptions::None)) == SRGBA<uint8> {52, 132, 228, 204});

    // List boxes select in translucent black, or white when dark.
    CHECK(Components(theme.ActiveListBoxSelectionForegroundColor(ColorOptions::UseDarkAppearance))
          == SRGBA<uint8> {255, 255, 255});
  }

  namespace
  {
    KRYS_NODISCARD const RenderStyle &StyleOf(EditorFixture &fixture, dom::DOMStringView id) noexcept
    {
      return fixture.ById(id).Renderer()->Style();
    }

    KRYS_NODISCARD bool Contains(utf8_stringview haystack, utf8_stringview needle) noexcept
    {
      return haystack.find(needle) != utf8_stringview::npos;
    }
  }

  TEST_CASE("RenderTheme: appearance and the adjusted styles", "[Rendering][RenderTheme]")
  {
    EditorFixture fixture(u8"<input type=checkbox id=check style=padding:5px;border:3px solid red>"
                          u8"<input type=radio id=radio><button id=button>Go</button>"
                          u8"<input id=field><input type=range id=range><select id=select></select>"
                          u8"<select id=list size=3></select><textarea id=area></textarea>"
                          u8"<progress id=progress></progress><meter id=meter></meter>"
                          u8"<input type=hidden id=hidden><div id=div style=appearance:auto></div>");

    // Each control's own appearance.
    CHECK(StyleOf(fixture, u8"check").UsedAppearance() == StyleAppearance::Checkbox);
    CHECK(StyleOf(fixture, u8"radio").UsedAppearance() == StyleAppearance::Radio);
    CHECK(StyleOf(fixture, u8"button").UsedAppearance() == StyleAppearance::Button);
    CHECK(StyleOf(fixture, u8"field").UsedAppearance() == StyleAppearance::TextField);
    CHECK(StyleOf(fixture, u8"range").UsedAppearance() == StyleAppearance::SliderHorizontal);
    CHECK(StyleOf(fixture, u8"select").UsedAppearance() == StyleAppearance::Menulist);
    CHECK(StyleOf(fixture, u8"list").UsedAppearance() == StyleAppearance::Listbox);
    CHECK(StyleOf(fixture, u8"area").UsedAppearance() == StyleAppearance::TextArea);
    CHECK(StyleOf(fixture, u8"progress").UsedAppearance() == StyleAppearance::ProgressBar);
    CHECK(StyleOf(fixture, u8"meter").UsedAppearance() == StyleAppearance::Meter);
    // An element that isn't a control has none, whatever `appearance` says.
    CHECK(StyleOf(fixture, u8"div").UsedAppearance() == StyleAppearance::None);

    // Checkboxes and radios are the theme's size, without borders or padding; text fields have rounded
    // corners.
    auto *check = fixture.ById(u8"check").GetRenderBox();
    CHECK(check->Width() == 12);
    CHECK(check->Height() == 12);
    CHECK(check->BorderLeft() == 0);
    CHECK(check->PaddingLeft() == 0);
    CHECK(StyleOf(fixture, u8"field").HasBorderRadius());
  }

  TEST_CASE("RenderTheme: a page's styles turn native appearance off", "[Rendering][RenderTheme]")
  {
    EditorFixture fixture(
      u8"<button id=plain>a</button><button id=styled style=background:red>b</button>"
      u8"<select id=select style=border:1px solid blue></select>"
      u8"<input id=field style=border:1px solid blue><input id=none style=appearance:none>");

    CHECK(StyleOf(fixture, u8"plain").UsedAppearance() == StyleAppearance::Button);
    // A background or border of the page's turns a button's and a field's appearance off, and makes a
    // drop-down's a menu list button, whose arrow the theme still draws.
    CHECK(StyleOf(fixture, u8"styled").UsedAppearance() == StyleAppearance::None);
    CHECK(StyleOf(fixture, u8"select").UsedAppearance() == StyleAppearance::MenulistButton);
    CHECK(StyleOf(fixture, u8"field").UsedAppearance() == StyleAppearance::None);
    CHECK(StyleOf(fixture, u8"none").UsedAppearance() == StyleAppearance::None);
  }

  TEST_CASE("RenderTheme: control states", "[Rendering][RenderTheme]")
  {
    EditorFixture fixture(u8"<input type=checkbox id=check checked><input type=checkbox id=mixed>"
                          u8"<input type=checkbox id=off disabled><input id=field readonly>"
                          u8"<input type=checkbox id=dark style=color-scheme:dark>");
    auto &theme = RenderTheme::Singleton();
    auto states = [&](dom::DOMStringView id)
    {
      return theme.ExtractControlStyleStatesForRenderer(*fixture.ById(id).Renderer());
    };

    CHECK(states(u8"check").Contains(ControlStyle::State::Checked));
    CHECK(states(u8"check").Contains(ControlStyle::State::Enabled));
    CHECK(states(u8"check").Contains(ControlStyle::State::WindowActive));
    CHECK_FALSE(states(u8"check").Contains(ControlStyle::State::Hovered));

    Downcast<html::HTMLInputElement>(fixture.ById(u8"mixed")).SetIndeterminate(true);
    CHECK(states(u8"mixed").Contains(ControlStyle::State::Indeterminate));

    CHECK_FALSE(states(u8"off").Contains(ControlStyle::State::Enabled));
    CHECK(states(u8"field").Contains(ControlStyle::State::ReadOnly));
    CHECK(states(u8"dark").Contains(ControlStyle::State::DarkAppearance));

    fixture.ById(u8"check").SetHovered(true);
    CHECK(states(u8"check").Contains(ControlStyle::State::Hovered));

    fixture.Document->SetFocusedAndActive(false);
    CHECK_FALSE(states(u8"check").Contains(ControlStyle::State::WindowActive));
  }

  TEST_CASE("RenderTheme: controls are painted by the theme", "[Rendering][RenderTheme]")
  {
    // The theme draws checkboxes and buttons instead of their CSS backgrounds and borders, and text fields'
    // borders over their CSS backgrounds.
    auto painted = Painted(u8"<style>body{margin:0}</style><input type=checkbox checked><button>Go</button>"
                           u8"<input style=width:50px><button style=background:red>No</button>");
    CHECK(Contains(painted, u8"(type checkbox)"));
    CHECK(Contains(painted, u8"(states [enabled, checked, window-active, read-only])"));
    CHECK(Contains(painted, u8"(type button)"));
    CHECK(Contains(painted, u8"(type textfield)"));
    CHECK_FALSE(Contains(painted, u8"#C0C0C0"));
    // A button styled by the page has its own background and no control part.
    CHECK(Contains(painted, u8"#FF0000"));
    CHECK(painted.find(u8"(type button)") == painted.rfind(u8"(type button)"));
  }
}
