#include "Krystal.Booey/HTML/InputTypes/ColorInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/Choosers/ColorChooser.hpp"
#include "Krystal.Booey/Platform/Choosers/ColorChooserClient.hpp"

namespace krys::boo::tests
{
  using html::HTMLInputElement;

  namespace
  {
    /// @brief Clicks @p input as a user does, with the window's transient activation that opening a picker
    /// needs.
    void UserClick(HTMLInputElement &input) noexcept
    {
      input.NodeDocument().Window()->NotifyActivated();
      input.Click();
    }

    /// @brief Calls `showPicker()` on @p input during a user gesture, which it needs.
    KRYS_NODISCARD dom::ExceptionOr<void> ShowPicker(HTMLInputElement &input) noexcept
    {
      input.NodeDocument().Window()->NotifyActivated();
      return input.ShowPicker();
    }

    /// @brief A colour chooser that records what it's told.
    struct RecordingColorChooser final : ColorChooser
    {
      ColorChooserClient *Client {nullptr};
      Color Selected;
      int32 Reattached {0};
      bool Ended {false};

      RecordingColorChooser(ColorChooserClient &client, const Color &initialColor) noexcept
          : Client(&client), Selected(initialColor)
      {
      }

      void ReattachColorChooser(const Color &color) noexcept final
      {
        Selected = color;
        ++Reattached;
      }

      void SetSelectedColor(const Color &color) noexcept final
      {
        Selected = color;
      }

      void EndChooser() noexcept final
      {
        if (Ended)
        {
          return;
        }
        Ended = true;
        if (auto *client = std::exchange(Client, nullptr))
        {
          client->DidEndChooser();
        }
      }
    };

    /// @brief A chrome client whose colour choosers record what they're told.
    struct ColorChromeClient final : ChromeClient
    {
      RefPtr<RecordingColorChooser> Chooser;

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      KRYS_NODISCARD RefPtr<ColorChooser> CreateColorChooser(ColorChooserClient &client,
                                                             const Color &initialColor) noexcept final
      {
        Chooser = CreateRef<RecordingColorChooser>(client, initialColor);
        return Chooser;
      }
    };

    KRYS_NODISCARD HTMLInputElement &InputById(EditorFixture &fixture, utf8_stringview id) noexcept
    {
      return Downcast<HTMLInputElement>(fixture.ById(id));
    }

    /// @brief The swatch (the `::-webkit-color-swatch` part) of a colour input.
    KRYS_NODISCARD dom::Element &Swatch(HTMLInputElement &input) noexcept
    {
      auto &wrapper = Downcast<dom::Element>(*input.UserAgentShadowRoot()->FirstChild());
      return Downcast<dom::Element>(*wrapper.FirstChild());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ColorInputType: the swatch", "[HTML][ColorInputType]")
  {
    EditorFixture fixture(u8"<input type=color id=c value=#FF8000>");
    auto &input = InputById(fixture, u8"c");

    // The value is lowercased.
    CHECK(input.Value() == u8"#ff8000");
    auto &wrapper = Downcast<dom::Element>(*input.UserAgentShadowRoot()->FirstChild());
    CHECK(wrapper.UserAgentPart() == u8"-webkit-color-swatch-wrapper");
    auto &swatch = Swatch(input);
    CHECK(swatch.UserAgentPart() == u8"-webkit-color-swatch");
    CHECK(swatch.GetAttribute(u8"style") == u8"background-color: rgb(255, 128, 0);");

    // The swatch follows the value.
    (void)input.SetValue(u8"#0000ff");
    CHECK(swatch.GetAttribute(u8"style") == u8"background-color: rgb(0, 0, 255);");
    (void)input.SetAttribute(u8"value", u8"#00ff00");
    CHECK(Swatch(input).GetAttribute(u8"style") == u8"background-color: rgb(0, 0, 255);");

    // It's drawn with the colour.
    (void)fixture.Document->UpdateLayout();
    REQUIRE(swatch.Renderer());
    CHECK(SerialisationForCSS(
            swatch.Renderer()->Style().ColorResolvingCurrentColor(css::PropertyId::BackgroundColor))
          == u8"rgb(0, 0, 255)");
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "ColorInputType: any CSS colour, with alpha and colorspace",
    "[HTML][ColorInputType][html:update-a-color-well-control-color]"
    "[html:serialize-a-color-well-control-color][html:attr-input-alpha][html:attr-input-colorspace]")
  {
    EditorFixture fixture(u8"<input type=color id=c>");
    auto &input = InputById(fixture, u8"c");
    auto value = [&](const char8_t *text)
    {
      (void)input.SetValue(text);
      return css::tests::ToNarrow(input.Value());
    };

    // Any CSS colour is parsed; without alpha it's opaque and serialised as a simple colour.
    CHECK(value(u8"red") == "#ff0000");
    CHECK(value(u8"rgb(0 128 255 / 0.5)") == "#0080ff");
    CHECK(value(u8"currentcolor") == "#000000");
    CHECK(value(u8"nonsense") == "#000000");

    // With alpha, the alpha is kept and the value is a color() function.
    (void)input.SetAttribute(u8"alpha", u8"");
    CHECK(input.Value() == u8"color(srgb 0 0 0)");
    // Components are rounded to 0 to 255, alpha too: 0.4 is 102.
    CHECK(value(u8"rgb(255 0 0 / 0.4)") == "color(srgb 1 0 0 / 0.4)");

    // In Display P3, the value is in that colour space.
    (void)input.RemoveAttribute(u8"alpha");
    (void)input.SetAttribute(u8"colorspace", u8"display-p3");
    CHECK(value(u8"color(display-p3 1 0 0)") == "color(display-p3 1 0 0)");
    CHECK(input.ColourSpace() == u8"display-p3");

    // Changing the attributes updates a dirty value.
    (void)input.RemoveAttribute(u8"colorspace");
    CHECK(input.Value() == u8"#ff0000");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ColorInputType: the colour chooser", "[HTML][ColorInputType]")
  {
    auto chromeClient = CreateUnique<ColorChromeClient>();
    auto *client = chromeClient.get();
    EditorFixture fixture(u8"<input type=color id=c value=#336699>", krys::move(chromeClient));
    auto &input = InputById(fixture, u8"c");
    auto inputs = fixture.Listen(u8"input");
    auto changes = fixture.Listen(u8"change");

    SECTION("a click without the user's activation doesn't show it")
    {
      input.Click();
      CHECK_FALSE(client->Chooser);
    }

    SECTION("a click shows it, with the value")
    {
      UserClick(input);
      REQUIRE(client->Chooser);
      CHECK(SerialisationForHTML(client->Chooser->Selected) == u8"#336699");
      CHECK(input.IsPresentingAttachedView());

      // Choosing a colour sets the value, firing input and change.
      client->Chooser->Client->DidChooseColor(SRGBA<uint8> {255, 0, 0});
      CHECK(input.Value() == u8"#ff0000");
      CHECK(Swatch(input).GetAttribute(u8"style") == u8"background-color: rgb(255, 0, 0);");
      CHECK(inputs->Count == 1);
      CHECK(changes->Count == 1);

      // Choosing the same colour does nothing.
      client->Chooser->Client->DidChooseColor(SRGBA<uint8> {255, 0, 0});
      CHECK(changes->Count == 1);

      // Setting the value tells the chooser; showing it again reattaches it.
      (void)input.SetValue(u8"#00ff00");
      CHECK(SerialisationForHTML(client->Chooser->Selected) == u8"#00ff00");
      CHECK_FALSE(ShowPicker(input).HasException());
      CHECK(client->Chooser->Reattached == 1);
    }

    SECTION("input and change fire once the colour is shown")
    {
      UserClick(input);
      REQUIRE(client->Chooser);

      List<dom::DOMString> swatchAtInput;
      auto handle =
        input.AddEventListener(dom::DOMStringAtom(u8"input"), [&](dom::Event &)
                               { swatchAtInput.push_back(*Swatch(input).GetAttribute(u8"style")); });
      client->Chooser->Client->DidChooseColor(SRGBA<uint8> {255, 0, 0});
      REQUIRE(swatchAtInput.size() == 1);
      CHECK(swatchAtInput[0] == u8"background-color: rgb(255, 0, 0);");
    }

    SECTION("losing the focus or the renderer ends it")
    {
      input.Focus();
      UserClick(input);
      REQUIRE(client->Chooser);
      input.Blur();
      CHECK(client->Chooser->Ended);
      CHECK_FALSE(input.IsPresentingAttachedView());

      CHECK_FALSE(ShowPicker(input).HasException());
      REQUIRE_FALSE(client->Chooser->Ended);
      (void)input.SetAttribute(u8"hidden", u8"");
      (void)fixture.Document->UpdateLayout();
      CHECK(client->Chooser->Ended);
    }

    SECTION("disabled inputs have none")
    {
      (void)input.SetAttribute(u8"disabled", u8"");
      UserClick(input);
      CHECK_FALSE(client->Chooser);
      CHECK(ShowPicker(input).HasException());
    }
  }
}
