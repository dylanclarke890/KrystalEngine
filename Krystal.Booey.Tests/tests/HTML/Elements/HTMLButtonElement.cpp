#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Events/ToggleEvent.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    KRYS_NODISCARD std::string Narrow(dom::DOMStringView string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }

    KRYS_NODISCARD std::string NarrowOrNull(const Maybe<dom::DOMString> &string)
    {
      return string ? Narrow(dom::DOMStringView(*string)) : std::string("null");
    }

    KRYS_NODISCARD std::string IdOf(const dom::Element *element)
    {
      return element ? NarrowOrNull(element->GetAttribute(u8"id")) : std::string("null");
    }

    /// @brief Records the command events fired at elements, as `"<target>:command:<command>:<source>"`, and
    /// the toggle events, as `"<target>:<type>:<source>"`.
    struct CommandLog
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;
      bool PreventCommand {false};

      void Watch(dom::Element &element) noexcept
      {
        auto id = IdOf(&element);
        Handles.push_back(element.AddEventListener(
          u8"command",
          [this, id](dom::Event &event)
          {
            auto &command = Downcast<dom::CommandEvent>(event);
            CHECK_FALSE(event.Bubbles());
            CHECK(event.Cancelable());
            CHECK(event.IsTrusted());
            Events.push_back(id + ":command:" + Narrow(command.Command()) + ":" + IdOf(command.Source()));
            if (PreventCommand)
            {
              event.PreventDefault();
            }
          }));
        Handles.push_back(element.AddEventListener(
          u8"beforetoggle", [this, id](dom::Event &event)
          { Events.push_back(id + ":beforetoggle:" + IdOf(Downcast<dom::ToggleEvent>(event).Source())); }));
      }

      /// @brief The events so far, after running the queued tasks.
      KRYS_NODISCARD List<std::string> Take() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
        return std::exchange(Events, {});
      }
    };

    void Click(LocalFrame &frame, double x, double y) noexcept
    {
      auto &handler = frame.GetEventHandler();
      (void)handler.HandleMousePressEvent(
        PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left,
                           PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now()));
      (void)handler.HandleMouseReleaseEvent(
        PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left,
                           PlatformEvent::Type::MouseReleased, 1, {}, MonotonicTime::Now()));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLButtonElement: type is the button's state, submit when Auto",
                   "[HTML][HTMLButtonElement][html:dom-button-type][html:attr-button-type-auto-state]"
                   "[html:concept-submit-button]")
  {
    NavigationFixture fixture(u8"<button id=b>B</button>");
    auto &button = fixture.Get<HTMLButtonElement>(u8"b");
    CHECK(Narrow(button.Type()) == "submit");
    button.Type(u8"RESET");
    CHECK(Narrow(button.Type()) == "reset");
    CHECK(NarrowOrNull(button.GetAttribute(u8"type")) == "RESET");
    button.Type(u8"Button");
    CHECK(Narrow(button.Type()) == "button");
    button.Type(u8"bogus");
    CHECK(Narrow(button.Type()) == "submit");
    // The Auto state, with a command: not a submit button.
    button.Command(u8"--spin");
    CHECK(Narrow(button.Type()) == "button");
    CHECK_FALSE(button.IsSubmitButton());
  }

  TEST_CASE("HTMLButtonElement: a button whose parent is a select isn't a submit button in the Auto state",
            "[HTML][HTMLButtonElement][html:concept-submit-button]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto select = document->CreateElement(u8"select").ReleaseValue();
    auto div = document->CreateElement(u8"div").ReleaseValue();
    auto button = document->CreateElement(u8"button").ReleaseValue();
    auto &element = Downcast<HTMLButtonElement>(*button);
    CHECK(element.IsSubmitButton());

    REQUIRE_FALSE(select->AppendChild(*button).HasException());
    CHECK_FALSE(element.IsSubmitButton());
    CHECK(Narrow(element.Type()) == "button");

    SECTION("only while it's there")
    {
      REQUIRE_FALSE(select->RemoveChild(*button).HasException());
      CHECK(element.IsSubmitButton());
      REQUIRE_FALSE(select->AppendChild(*button).HasException());
      CHECK_FALSE(element.IsSubmitButton());
      REQUIRE_FALSE(div->AppendChild(*button).HasException());
      CHECK(element.IsSubmitButton());
    }

    SECTION("not for a descendant of the select")
    {
      REQUIRE_FALSE(select->AppendChild(*div).HasException());
      REQUIRE_FALSE(div->AppendChild(*button).HasException());
      CHECK(element.IsSubmitButton());
    }

    SECTION("not in a state other than Auto")
    {
      REQUIRE_FALSE(button->SetAttribute(u8"type", u8"submit").HasException());
      CHECK(element.IsSubmitButton());
      button->RemoveAttribute(u8"type");
      CHECK_FALSE(element.IsSubmitButton());
    }
  }

  TEST_CASE("HTMLButtonElement: a button in a document that isn't fully active does nothing",
            "[HTML][HTMLButtonElement][html:the-button-element]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto button = document->CreateElement(u8"button").ReleaseValue();
    auto target = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(target->SetAttribute(u8"id", u8"t").HasException());
    REQUIRE_FALSE(button->SetAttribute(u8"commandfor", u8"t").HasException());
    REQUIRE_FALSE(button->SetAttribute(u8"command", u8"--spin").HasException());
    auto body = document->CreateElement(u8"body").ReleaseValue();
    REQUIRE_FALSE(body->AppendChild(*button).HasException());
    REQUIRE_FALSE(body->AppendChild(*target).HasException());
    REQUIRE_FALSE(document->AppendChild(*body).HasException());
    REQUIRE(Downcast<HTMLButtonElement>(*button).CommandForElement().get() == target.get());

    int commands = 0;
    auto listener = target->AddEventListener(u8"command", [&](dom::Event &) { ++commands; });
    Downcast<HTMLElement>(*button).Click();
    CHECK(commands == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLButtonElement: command attributes",
                   "[HTML][HTMLButtonElement][html:attr-button-command][html:dom-button-command]"
                   "[html:attr-button-command-unknown-state][html:attr-button-command-custom]")
  {
    NavigationFixture fixture(u8"<button id=b>B</button><div id=p popover></div>"
                              u8"<form><button id=f commandfor=p>F</button><button id=fs type=submit "
                              u8"commandfor=p>S</button></form>");
    auto &button = fixture.Get<HTMLButtonElement>(u8"b");
    auto &popover = fixture.ById(u8"p");

    SECTION("command reflects its known values, and custom commands as they are")
    {
      CHECK(Narrow(button.Command()).empty());
      CHECK(button.GetCommandType() == dom::CommandType::Invalid);
      button.Command(u8"Toggle-Popover");
      CHECK(NarrowOrNull(button.GetAttribute(u8"command")) == "Toggle-Popover");
      CHECK(Narrow(button.Command()) == "toggle-popover");
      CHECK(button.GetCommandType() == dom::CommandType::TogglePopover);
      button.Command(u8"REQUEST-CLOSE");
      CHECK(button.GetCommandType() == dom::CommandType::RequestClose);
      button.Command(u8"--Spin");
      CHECK(Narrow(button.Command()) == "--Spin");
      CHECK(button.GetCommandType() == dom::CommandType::Custom);
      button.Command(u8"-spin");
      CHECK(Narrow(button.Command()).empty());
      CHECK(button.GetCommandType() == dom::CommandType::Invalid);
    }

    SECTION("commandForElement sets the attribute to the empty string")
    {
      CHECK_FALSE(button.CommandForElement());
      button.CommandForElementForBindings(&popover);
      CHECK(NarrowOrNull(button.GetAttribute(u8"commandfor")) == "");
      CHECK(button.CommandForElement().get() == &popover);
      CHECK(button.CommandForElementForBindings().get() == &popover);
      (void)button.SetAttribute(dom::DOMStringAtom(u8"commandfor"), u8"nope");
      CHECK_FALSE(button.CommandForElement());
      (void)button.SetAttribute(dom::DOMStringAtom(u8"commandfor"), u8"p");
      CHECK(button.CommandForElement().get() == &popover);
      button.CommandForElementForBindings(nullptr);
      CHECK_FALSE(button.HasAttribute(u8"commandfor"));
    }

    SECTION("A button with a command, and no type, is a plain button")
    {
      CHECK(Narrow(button.FormControlType()) == "submit");
      button.Command(u8"--spin");
      CHECK(Narrow(button.FormControlType()) == "button");
      (void)button.RemoveAttribute(u8"command");
      CHECK(Narrow(button.FormControlType()) == "submit");

      auto &inForm = fixture.Get<HTMLButtonElement>(u8"f");
      CHECK(Narrow(inForm.FormControlType()) == "button");
      CHECK_FALSE(inForm.IsSubmitButton());
      CHECK(fixture.Get<HTMLButtonElement>(u8"fs").IsSubmitButton());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLButtonElement: popover commands",
                   "[HTML][HTMLButtonElement][html:determine-if-command-is-valid][html:the-button-element]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 } button { display: block; width: 50px; height: 20px; margin: 0 }</style>"
      u8"<button id=t commandfor=p command=toggle-popover>T</button>"
      u8"<button id=show commandfor=p command=show-popover>S</button>"
      u8"<button id=hide commandfor=p command=hide-popover>H</button>"
      u8"<button id=modal commandfor=p command=show-modal>M</button>"
      u8"<button id=custom commandfor=p command=--spin>C</button>"
      u8"<button id=nothing commandfor=p command=spin>N</button>"
      u8"<div id=p popover>P</div>");
    auto &document = *fixture.Document;
    auto &popover = Downcast<HTMLElement>(fixture.ById(u8"p"));
    auto &toggle = fixture.Get<HTMLButtonElement>(u8"t");
    CommandLog log;
    log.Watch(popover);
    // The command event doesn't bubble.
    log.Watch(*document.DocumentElement());

    SECTION("The popover commands show and hide it, its invoker the button")
    {
      toggle.Click();
      CHECK(popover.IsPopoverShowing());
      CHECK(popover.GetPopoverData()->Invoker() == &toggle);
      CHECK(log.Take() == List<std::string> {"p:command:toggle-popover:t", "p:beforetoggle:t"});
      toggle.Click();
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK(log.Take() == List<std::string> {"p:command:toggle-popover:t", "p:beforetoggle:t"});

      // Showing a showing popover sends the command, but does nothing.
      fixture.Get<HTMLButtonElement>(u8"show").Click();
      fixture.Get<HTMLButtonElement>(u8"show").Click();
      CHECK(popover.IsPopoverShowing());
      CHECK(log.Take()
            == List<std::string> {"p:command:show-popover:show", "p:beforetoggle:show",
                                  "p:command:show-popover:show"});
      fixture.Get<HTMLButtonElement>(u8"hide").Click();
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK(log.Take() == List<std::string> {"p:command:hide-popover:hide", "p:beforetoggle:hide"});
    }

    SECTION("Commands the element doesn't carry out aren't sent; custom commands only fire the event")
    {
      fixture.Get<HTMLButtonElement>(u8"modal").Click();
      fixture.Get<HTMLButtonElement>(u8"nothing").Click();
      CHECK(log.Take().empty());
      fixture.Get<HTMLButtonElement>(u8"custom").Click();
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK(log.Take() == List<std::string> {"p:command:--spin:custom"});
    }

    SECTION("Canceling the command event cancels the command")
    {
      log.PreventCommand = true;
      toggle.Click();
      CHECK_FALSE(popover.IsPopoverShowing());
      CHECK(log.Take() == List<std::string> {"p:command:toggle-popover:t"});
    }

    SECTION("A click on the button doesn't light dismiss the popover it toggles")
    {
      toggle.Click();
      REQUIRE(popover.IsPopoverShowing());
      (void)document.UpdateLayout();
      Click(*fixture.Frame, 10, 10);
      CHECK_FALSE(popover.IsPopoverShowing());
      Click(*fixture.Frame, 10, 10);
      CHECK(popover.IsPopoverShowing());
      // A click elsewhere still does.
      Click(*fixture.Frame, 300, 300);
      CHECK_FALSE(popover.IsPopoverShowing());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLButtonElement: dialog commands",
                   "[HTML][HTMLButtonElement][html:determine-if-command-is-valid][html:the-button-element]"
                   "[html:is-valid-command-steps][html:command-steps]")
  {
    NavigationFixture fixture(
      u8"<button id=open commandfor=d command=show-modal>O</button>"
      u8"<dialog id=d><button id=close commandfor=d command=close value=done>C</button>"
      u8"<button id=ask commandfor=d command=request-close value=asked>R</button>"
      u8"<button id=bare commandfor=d command=close>X</button></dialog>"
      u8"<form><button id=implicit commandfor=d command=show-modal>I</button>"
      u8"<button id=explicit type=button commandfor=d command=show-modal>E</button></form>"
      u8"<button id=disabled disabled commandfor=d command=show-modal>D</button>"
      u8"<dialog id=pd popover></dialog><button id=pdmodal commandfor=pd command=show-modal>P</button>"
      u8"<button id=pdtoggle commandfor=pd command=toggle-popover>Q</button>");
    auto &dialog = fixture.Get<HTMLDialogElement>(u8"d");
    auto click = [&](dom::DOMStringView id)
    {
      fixture.Get<HTMLButtonElement>(id).Click();
    };
    CommandLog log;
    log.Watch(dialog);
    List<std::string> cancels;
    auto cancelListener =
      dialog.AddEventListener(u8"cancel", [&](dom::Event &) { cancels.push_back("cancel"); });

    SECTION("show-modal shows it modally, and close closes it with the button's value")
    {
      click(u8"open");
      CHECK(dialog.Open());
      CHECK(dialog.IsModal());
      CHECK(log.Take() == List<std::string> {"d:command:show-modal:open", "d:beforetoggle:open"});
      // Showing an open dialog sends the command, but does nothing.
      click(u8"open");
      CHECK(log.Take() == List<std::string> {"d:command:show-modal:open"});

      click(u8"close");
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()) == "done");
      CHECK(log.Take() == List<std::string> {"d:command:close:close", "d:beforetoggle:close"});

      // A button without a value leaves the return value.
      click(u8"open");
      click(u8"bare");
      CHECK_FALSE(dialog.Open());
      CHECK(Narrow(dialog.ReturnValue()) == "done");
    }

    SECTION("request-close asks it to close")
    {
      click(u8"open");
      click(u8"ask");
      (void)log.Take();
      CHECK_FALSE(dialog.Open());
      CHECK(cancels == List<std::string> {"cancel"});
      CHECK(Narrow(dialog.ReturnValue()) == "asked");
    }

    SECTION("A button in a form needs an explicit type to send its command; a disabled one sends none")
    {
      click(u8"implicit");
      click(u8"disabled");
      CHECK_FALSE(dialog.Open());
      CHECK(log.Take().empty());
      click(u8"explicit");
      CHECK(dialog.Open());
    }

    SECTION("A dialog showing as a popover takes the popover commands, but not show-modal")
    {
      auto &popoverDialog = fixture.Get<HTMLDialogElement>(u8"pd");
      click(u8"pdtoggle");
      CHECK(popoverDialog.IsPopoverShowing());
      click(u8"pdmodal");
      CHECK_FALSE(popoverDialog.Open());
      CHECK_FALSE(popoverDialog.IsModal());
    }
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "HTMLButtonElement: commandfor naming a shadow host commands its reference target",
    "[HTML][HTMLButtonElement][html:attr-button-commandfor][html:resolve-the-reference-target]")
  {
    NavigationFixture fixture(
      u8"<button id=t commandfor=h command=toggle-popover>T</button><x-host id=h></x-host>");
    // A shadow root on the element with ID @p id, its reference target @p target, holding @p markup.
    auto attach = [&](dom::DOMStringView id, const char8_t *target, const char8_t *markup)
    {
      auto shadow = fixture.ById(id).AttachShadow({.ReferenceTarget = dom::DOMString(target)}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTMLUnsafe(markup).HasException());
      return shadow;
    };
    auto shadow = attach(u8"h", u8"p", u8"<div id=p popover>P</div>");
    auto &popover = Downcast<HTMLElement>(*shadow->GetElementById(u8"p"));
    auto &button = fixture.Get<HTMLButtonElement>(u8"t");

    CHECK(button.CommandForElement().get() == &popover);
    CHECK(button.CommandForElementForBindings().get() == &fixture.ById(u8"h"));
    button.Click();
    CHECK(popover.IsPopoverShowing());
  }
}
