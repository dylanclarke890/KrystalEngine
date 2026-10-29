#include "Krystal.Booey/HTML/ColorInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/ColorChooser.hpp"
#include "Krystal.Booey/Platform/ColorChooserClient.hpp"

namespace krys::boo::tests
{
  using html::HTMLInputElement;

  namespace
  {
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
            swatch.Renderer()->Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor))
          == u8"rgb(0, 0, 255)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ColorInputType: the colour chooser", "[HTML][ColorInputType]")
  {
    auto chromeClient = CreateUnique<ColorChromeClient>();
    auto *client = chromeClient.get();
    EditorFixture fixture(u8"<input type=color id=c value=#336699>", krys::move(chromeClient));
    auto &input = InputById(fixture, u8"c");
    auto inputs = fixture.Listen(u8"input");
    auto changes = fixture.Listen(u8"change");

    SECTION("a click shows it, with the value")
    {
      input.Click();
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

    SECTION("losing the focus or the renderer ends it")
    {
      input.Focus();
      input.Click();
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
      input.Click();
      CHECK_FALSE(client->Chooser);
      CHECK(ShowPicker(input).HasException());
    }
  }
}
