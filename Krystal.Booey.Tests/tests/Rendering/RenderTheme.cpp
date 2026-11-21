#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/StringTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::ValueId;
  using style::ColorOption;
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
      CHECK(SystemColor(ValueId::Canvas, {}) == SRGBA<uint8> {255, 255, 255});
      CHECK(SystemColor(ValueId::Canvas, ColorOption::UseDarkAppearance) == SRGBA<uint8> {30, 30, 30});
      CHECK(SystemColor(ValueId::Canvastext, {}) == SRGBA<uint8> {0, 0, 0});
      CHECK(SystemColor(ValueId::Canvastext, ColorOption::UseDarkAppearance) == SRGBA<uint8> {255, 255, 255});
      CHECK(SystemColor(ValueId::Buttonface, {}) == Components(Color::lightGray));
      CHECK(SystemColor(ValueId::Buttontext, {}) == SRGBA<uint8> {0, 0, 0, 204});
      CHECK(SystemColor(ValueId::Highlight, {}) == SRGBA<uint8> {52, 132, 228});
      CHECK(SystemColor(ValueId::Mark, {}) == Components(Color::yellow));
      CHECK(SystemColor(ValueId::Graytext, {}) == SRGBA<uint8> {0, 0, 0, 102});
      CHECK(SystemColor(ValueId::Field, ColorOption::UseDarkAppearance) == SRGBA<uint8> {45, 45, 45});
      CHECK(SystemColor(ValueId::Accentcolor, {}) == SRGBA<uint8> {0, 122, 255});
      CHECK(SystemColor(ValueId::Text, {}) == SRGBA<uint8> {0, 0, 0});
    }

    SECTION("Links depend on the appearance and visited state")
    {
      CHECK(SystemColor(ValueId::Linktext, {}) == SRGBA<uint8> {0, 0, 238});
      CHECK(SystemColor(ValueId::Linktext, ColorOption::UseDarkAppearance) == SRGBA<uint8> {158, 158, 255});
      CHECK(SystemColor(ValueId::Visitedtext, {}) == SRGBA<uint8> {85, 26, 139});
      CHECK(SystemColor(ValueId::Activetext, {}) == Components(Color::red));
      CHECK(SystemColor(ValueId::WebkitActivelink, ColorOption::UseDarkAppearance)
            == SRGBA<uint8> {255, 158, 158});

      CHECK(SystemColor(ValueId::WebkitLink, {}) == SystemColor(ValueId::Linktext, {}));
    }

    SECTION("Deprecated system colours map to their replacements")
    {
      CHECK(SystemColor(ValueId::Activeborder, {}) == SystemColor(ValueId::Buttonborder, {}));
      CHECK(SystemColor(ValueId::Buttonshadow, {}) == SystemColor(ValueId::Buttonface, {}));
      CHECK(SystemColor(ValueId::Inactivecaptiontext, {}) == SystemColor(ValueId::Graytext, {}));
      CHECK(SystemColor(ValueId::Menu, {}) == SystemColor(ValueId::Canvas, {}));
      CHECK(SystemColor(ValueId::Windowtext, {}) == SystemColor(ValueId::Canvastext, {}));
    }

    SECTION("Anything else is the invalid colour")
    {
      auto &theme = RenderTheme::Singleton();
      CHECK_FALSE(theme.SystemColor(ValueId::Red, {}).IsValid());
      CHECK_FALSE(theme.SystemColor(ValueId::Currentcolor, {}).IsValid());
      CHECK_FALSE(theme.SystemColor(ValueId::InternalDocumentTextColor, {}).IsValid());
    }
  }

  TEST_CASE("RenderTheme: selection and focus ring colours", "[Rendering][RenderTheme]")
  {
    auto &theme = RenderTheme::Singleton();

    // The accent colour at 30% over white for the active selection; a translucent grey for the inactive one.
    CHECK(Components(theme.ActiveSelectionBackgroundColor({}))
          == Components(theme.TransformSelectionBackgroundColor(
            Color(SRGBA<uint8> {52, 132, 228}).ColorWithAlphaMultipliedBy(0.3f), {})));
    CHECK_FALSE(theme.ActiveSelectionForegroundColor({}).IsValid());

    // The focus ring is the accent colour at 80%.
    CHECK(Components(theme.FocusRingColor({})) == SRGBA<uint8> {52, 132, 228, 204});

    // List boxes select in translucent black, or white when dark.
    CHECK(Components(theme.ActiveListBoxSelectionForegroundColor(ColorOption::UseDarkAppearance))
          == SRGBA<uint8> {255, 255, 255});
  }

  namespace
  {
    KRYS_NODISCARD const RenderStyle &StyleOf(EditorFixture &fixture, DOMStringView id) noexcept
    {
      return fixture.ById(id).Renderer()->Style();
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
    auto states = [&](DOMStringView id)
    {
      return theme.ExtractControlStyleStatesForRenderer(*fixture.ById(id).Renderer());
    };

    CHECK(states(u8"check").contains(ControlStyle::State::Checked));
    CHECK(states(u8"check").contains(ControlStyle::State::Enabled));
    CHECK(states(u8"check").contains(ControlStyle::State::WindowActive));
    CHECK_FALSE(states(u8"check").contains(ControlStyle::State::Hovered));

    Downcast<boo::HTMLInputElement>(fixture.ById(u8"mixed")).SetIndeterminate(true);
    CHECK(states(u8"mixed").contains(ControlStyle::State::Indeterminate));

    CHECK_FALSE(states(u8"off").contains(ControlStyle::State::Enabled));
    CHECK(states(u8"field").contains(ControlStyle::State::ReadOnly));
    CHECK(states(u8"dark").contains(ControlStyle::State::DarkAppearance));

    fixture.ById(u8"check").SetHovered(true);
    CHECK(states(u8"check").contains(ControlStyle::State::Hovered));

    fixture.Document->SetFocusedAndActive(false);
    CHECK_FALSE(states(u8"check").contains(ControlStyle::State::WindowActive));
  }

  TEST_CASE("RenderTheme: controls repaint when they're hovered or pressed", "[Rendering][RenderTheme]")
  {
    // The theme draws hover and press states without a style change, so the control repaints itself.
    PaintFixture fixture(u8"<!DOCTYPE html><style>body{margin:0}</style><button id=button>Go</button>"
                         u8"<button id=off disabled>No</button><div id=plain>x</div>");
    (void)fixture.PaintedDisplayList();
    auto &view = *fixture.Frame->View();
    (void)view.TakeDamage();

    SECTION("Hovering")
    {
      fixture.ById(u8"button").SetHovered(true);
      CHECK(view.HasDamage());
    }

    SECTION("Pressing")
    {
      fixture.ById(u8"button").SetActive(true);
      CHECK(view.HasDamage());
    }

    SECTION("A disabled control doesn't repaint for a press")
    {
      fixture.ById(u8"off").SetActive(true);
      CHECK_FALSE(view.HasDamage());
    }

    SECTION("An element without an appearance waits for its style")
    {
      fixture.ById(u8"plain").SetHovered(true);
      fixture.ById(u8"plain").SetActive(true);
      CHECK_FALSE(view.HasDamage());
    }
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTheme::PlatformColorsDidChange: every page restyles",
                   "[Rendering][RenderTheme]")
  {
    PaintFixture first(u8"<!DOCTYPE html><p>One");
    PaintFixture second(u8"<!DOCTYPE html><p>Two");
    (void)first.Painted();
    (void)second.Painted();
    REQUIRE_FALSE(first.Document->HasPendingFullStyleRebuild());
    REQUIRE_FALSE(second.Document->HasPendingFullStyleRebuild());

    // The theme's colours changing (its accent colour, say) restyles what's drawn with them.
    RenderTheme::Singleton().PlatformColorsDidChange();
    CHECK(first.Document->HasPendingFullStyleRebuild());
    CHECK(second.Document->HasPendingFullStyleRebuild());
    (void)first.Painted();
    CHECK_FALSE(first.Document->HasPendingFullStyleRebuild());
  }
}
