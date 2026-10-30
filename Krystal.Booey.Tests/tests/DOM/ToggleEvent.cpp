#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/ToggleEventTask.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD std::string Narrow(const DOMString &string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ToggleEvent and CommandEvent", "[DOM][ToggleEvent]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><div id=host></div><button id=button></button>)html"};
    auto &host = fixture.ById(u8"host");
    auto &button = fixture.ById(u8"button");

    SECTION("A toggle event made by the page is untrusted and takes its dictionary")
    {
      ToggleEventInit init;
      init.OldState = u8"closed";
      init.NewState = u8"open";
      init.Source = ShareRefPtr(&button);
      auto event = CreateRef<ToggleEvent>(DOMStringAtom(u8"toggle"), init);
      CHECK_FALSE(event->IsTrusted());
      CHECK(Narrow(event->OldState()) == "closed");
      CHECK(Narrow(event->NewState()) == "open");
      CHECK(event->Source() == &button);
      CHECK(Is<ToggleEvent>(*event));

      auto trusted = ToggleEvent::Create(DOMStringAtom(u8"beforetoggle"), init, true);
      CHECK(trusted->IsTrusted());
      CHECK(trusted->Cancelable());
      CHECK_FALSE(trusted->Bubbles());
    }

    SECTION("The source is retargeted, as a related target is")
    {
      auto shadow = host.AttachShadow({}).ReleaseValue();
      auto inner = fixture.Document->CreateElement(u8"button").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

      CommandEventInit init;
      init.Source = inner;
      init.Command = u8"--refresh";
      auto event = CommandEvent::Create(DOMStringAtom(u8"command"), init, true);
      CHECK(event->IsTrusted());
      CHECK(Narrow(event->Command()) == "--refresh");
      CHECK(Is<CommandEvent>(*event));
      // Outside a dispatch, against the document: the shadow host.
      CHECK(event->Source() == &host);

      // A listener in the shadow tree sees the button itself.
      Element *seen = nullptr;
      auto handle = inner->AddEventListener(u8"command", [&](Event &dispatched)
                                            { seen = Downcast<CommandEvent>(dispatched).Source(); });
      (void)inner->DispatchEvent(*event);
      CHECK(seen == inner.get());
    }

    SECTION("A toggle event task coalesces the toggles before it runs")
    {
      List<std::string> events;
      Element *source = nullptr;
      auto handle =
        host.AddEventListener(u8"toggle",
                              [&](Event &event)
                              {
                                auto &toggle = Downcast<ToggleEvent>(event);
                                CHECK(toggle.IsTrusted());
                                events.push_back(Narrow(toggle.OldState()) + ">" + Narrow(toggle.NewState()));
                                source = toggle.Source();
                              });
      auto task = ToggleEventTask::Create(host);

      task->Queue(ToggleState::Closed, ToggleState::Open);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events == List<std::string> {"closed>open"});

      task->Queue(ToggleState::Open, ToggleState::Closed);
      task->Queue(ToggleState::Closed, ToggleState::Open, &button);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events == List<std::string> {"closed>open", "open>open"});
      CHECK(source == &button);
    }
  }
}
