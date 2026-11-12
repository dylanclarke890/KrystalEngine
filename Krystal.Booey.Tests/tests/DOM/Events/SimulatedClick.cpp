#include "Krystal.Booey/DOM/Events/SimulatedClick.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Puts the root view 100 pixels right of and 200 below the screen's origin.
    class OffsetChromeClient final : public ChromeClient
    {
    public:
      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
      {
      }

      KRYS_NODISCARD IntPoint RootViewToScreen(const IntPoint &point) const noexcept override
      {
        return point + IntSize(100, 200);
      }
      KRYS_NODISCARD IntPoint ScreenToRootView(const IntPoint &point) const noexcept override
      {
        return point - IntSize(100, 200);
      }
    };

    /// @brief Keeps the last mouse event's coordinates.
    struct CoordinateRecorder : public dom::EventListenerCallback
    {
      double ScreenX {-1}, ScreenY {-1}, ClientX {-1}, ClientY {-1};

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto &mouse = static_cast<dom::MouseEvent &>(event);
        ScreenX = mouse.ScreenX();
        ScreenY = mouse.ScreenY();
        ClientX = mouse.ClientX();
        ClientY = mouse.ClientY();
      }
    };
  }

  TEST_CASE("SimulatedClick: the user agent's clicks are at the target's centre, on the screen too",
            "[DOM][SimulatedClick]")
  {
    auto chrome = CreateUnique<OffsetChromeClient>();
    auto page = Page::Create(krys::move(chrome));
    PageInFrameFixture fixture(*page, u8"<style>body{margin:0}</style>"
                                      u8"<button id=button style='position:absolute;left:10px;top:20px;"
                                      u8"width:40px;height:10px'></button>");
    auto &button = fixture.ById(u8"button");
    auto recorder = CreateRef<CoordinateRecorder>();
    button.AddEventListener(dom::DOMStringAtom(EventNames::Click), recorder, false);

    (void)fixture.Document->UpdateLayout();
    CHECK(button.ScreenRect() == IntRect(110, 220, 40, 10));
    CHECK(fixture.Document->View()->ScreenToContents({130, 225}) == IntPoint(30, 25));

    (void)button.DispatchSimulatedClick(nullptr);
    CHECK(recorder->ClientX == 30);
    CHECK(recorder->ClientY == 25);
    CHECK(recorder->ScreenX == 130);
    CHECK(recorder->ScreenY == 225);

    // The user agent's mouse events made without an underlying event are too.
    auto event =
      dom::MouseEvent::CreateSimulated(dom::DOMStringAtom(EventNames::Click), nullptr, button, true);
    CHECK(event->ScreenX() == 130);
    CHECK(event->ClientX() == 30);
  }
}
