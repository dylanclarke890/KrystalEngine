#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeEditElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/DateTimeChooser.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserClient.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserParameters.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"

namespace krys::boo::tests
{
  using html::DateTimeEditElement;
  using html::HTMLInputElement;

  namespace
  {
    /// @brief A date chooser that records how it's shown.
    struct RecordingDateTimeChooser final : DateTimeChooser
    {
      DateTimeChooserClient *Client {nullptr};
      Maybe<DateTimeChooserParameters> Parameters;
      int32 Shows {0};
      bool Ended {false};

      explicit RecordingDateTimeChooser(DateTimeChooserClient &client) noexcept : Client(&client)
      {
      }

      void ShowChooser(const DateTimeChooserParameters &parameters) noexcept final
      {
        Parameters = parameters;
        ++Shows;
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

    /// @brief A chrome client whose date choosers record how they're shown.
    struct DateChromeClient final : ChromeClient
    {
      RefPtr<RecordingDateTimeChooser> Chooser;

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      KRYS_NODISCARD RefPtr<DateTimeChooser>
        CreateDateTimeChooser(DateTimeChooserClient &client) noexcept final
      {
        Chooser = CreateRef<RecordingDateTimeChooser>(client);
        return Chooser;
      }
    };

    /// @brief A fixture whose form controls follow their `lang`, so tests don't depend on the system's
    /// language.
    struct DateFixture : EditorFixture
    {
      explicit DateFixture(UniquePtr<ChromeClient> chromeClient = nullptr) noexcept
          : EditorFixture(u8"", krys::move(chromeClient))
      {
        auto settings = Settings::Create();
        settings->LangAttributeAwareFormControlUIEnabled = true;
        Document->SetSettings(krys::move(settings));
      }

      /// @brief Appends an input of @p type to the body, in the language @p lang, with the attributes given.
      HTMLInputElement &
        Input(utf8_stringview type, utf8_stringview lang = u8"en-US",
              std::initializer_list<Pair<utf8_stringview, utf8_stringview>> attributes = {}) noexcept
      {
        auto element = Document->CreateElement(dom::DOMStringAtom(u8"input")).Value();
        auto &input = Downcast<HTMLInputElement>(*element);
        (void)input.SetAttribute(u8"lang", dom::DOMString(lang));
        for (auto &[name, value] : attributes)
        {
          (void)input.SetAttribute(dom::DOMStringAtom(name), dom::DOMString(value));
        }
        (void)input.SetAttribute(u8"type", dom::DOMString(type));
        (void)Body().AppendChild(input);
        (void)Document->UpdateLayout();
        return input;
      }

      /// @brief Presses the key @p identifier (with the code @p code), as the host does.
      bool Press(utf8_string identifier, utf8_string code = {}, int32 keyCode = 0) noexcept
      {
        return Frame->GetEventHandler().KeyEvent(
          PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", identifier, krys::move(code),
                                identifier, keyCode, false, false, false, {}, {}));
      }

      /// @brief Types @p text, a character, as the host does after its key down, at @p seconds.
      bool Type(utf8_string text, int64 seconds = 0) noexcept
      {
        auto c = static_cast<int32>(static_cast<char8>(text[0]));
        auto timestamp = std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::seconds(seconds));
        return Frame->GetEventHandler().KeyEvent(PlatformKeyboardEvent(
          PlatformEvent::Type::Char, text, text, text, u8"", u8"", c, false, false, false, {}, timestamp));
      }
    };

    KRYS_NODISCARD DateTimeEditElement &EditElement(HTMLInputElement &input) noexcept
    {
      // The shadow tree is the edit element alone.
      return static_cast<DateTimeEditElement &>(*input.UserAgentShadowRoot()->FirstChild());
    }

    /// @brief The edit element's fields and literals: each field as its part's name without the
    /// `-webkit-datetime-edit-` and `-field`, and each literal quoted.
    KRYS_NODISCARD utf8_string Layout(HTMLInputElement &input) noexcept
    {
      utf8_string layout;
      auto &wrapper = EditElement(input).FieldsWrapperElement();
      for (auto *child = wrapper.FirstChild(); child; child = child->NextSibling())
      {
        auto &element = Downcast<dom::Element>(*child);
        auto part = element.UserAgentPart().View();
        constexpr utf8_stringview prefix = u8"-webkit-datetime-edit-";
        if (part == u8"-webkit-datetime-edit-text")
        {
          layout += u8'"';
          layout += element.TextContent().value_or(dom::DOMString());
          layout += u8'"';
        }
        else
        {
          part.remove_prefix(prefix.size());
          part.remove_suffix(utf8_stringview(u8"-field").size());
          layout += part;
        }
      }
      return layout;
    }

    /// @brief The text the fields show.
    KRYS_NODISCARD utf8_string Shown(HTMLInputElement &input) noexcept
    {
      return EditElement(input).FieldsWrapperElement().TextContent().value_or(dom::DOMString());
    }

    KRYS_NODISCARD dom::Element &Field(HTMLInputElement &input, size_t index) noexcept
    {
      return *EditElement(input).Fields()[index];
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: field layouts",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    DateFixture fixture;

    SECTION("dates follow the locale")
    {
      CHECK(Layout(fixture.Input(u8"date")) == u8"month\"/\"day\"/\"year");
      CHECK(Layout(fixture.Input(u8"date", u8"en-GB")) == u8"day\"/\"month\"/\"year");
      CHECK(Layout(fixture.Input(u8"date", u8"de")) == u8"day\".\"month\".\"year");
      CHECK(Layout(fixture.Input(u8"date", u8"ja")) == u8"year\"/\"month\"/\"day");
    }

    SECTION("times show seconds and milliseconds as the step needs them")
    {
      // A narrow no-break space before the period.
      CHECK(Layout(fixture.Input(u8"time")) == u8"hour\":\"minute\"\u202f\"meridiem");
      CHECK(Layout(fixture.Input(u8"time", u8"en-US", {{u8"step", u8"1"}}))
            == u8"hour\":\"minute\":\"second\"\u202f\"meridiem");
      CHECK(Layout(fixture.Input(u8"time", u8"en-US", {{u8"step", u8"0.001"}}))
            == u8"hour\":\"minute\":\"second\".\"millisecond\"\u202f\"meridiem");
      CHECK(Layout(fixture.Input(u8"time", u8"de")) == u8"hour\":\"minute");
      // A value with seconds shows them.
      CHECK(Layout(fixture.Input(u8"time", u8"de", {{u8"value", u8"10:20:30"}}))
            == u8"hour\":\"minute\":\"second");
    }

    SECTION("months, weeks and local date times")
    {
      CHECK(Layout(fixture.Input(u8"month")) == u8"month\" \"year");
      CHECK(Layout(fixture.Input(u8"week")) == u8"\"Week \"week\", \"year");
      CHECK(Layout(fixture.Input(u8"datetime-local", u8"en-GB"))
            == u8"day\"/\"month\"/\"year\", \"hour\":\"minute");
    }

    SECTION("the fields are focused instead of the input")
    {
      auto &input = fixture.Input(u8"date");
      CHECK(input.UserAgentShadowRoot()->DelegatesFocus());
      // Changing the type replaces the fields, and the input takes focus itself again.
      (void)input.SetAttribute(u8"type", u8"text");
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(input.UserAgentShadowRoot()->DelegatesFocus());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: values",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    DateFixture fixture;

    SECTION("the fields show the value")
    {
      auto &date = fixture.Input(u8"date", u8"en-US", {{u8"value", u8"2026-03-07"}});
      CHECK(Shown(date) == u8"03/07/2026");
      (void)date.SetValue(u8"1999-12-31");
      CHECK(Shown(date) == u8"12/31/1999");

      auto &month = fixture.Input(u8"month", u8"en-US", {{u8"value", u8"2026-09"}});
      CHECK(Shown(month) == u8"Sep 2026");

      auto &week = fixture.Input(u8"week", u8"en-US", {{u8"value", u8"2026-W05"}});
      CHECK(Shown(week) == u8"Week 05, 2026");

      auto &time = fixture.Input(u8"time", u8"en-US", {{u8"value", u8"13:05"}});
      CHECK(Shown(time) == u8"01:05\u202fPM");
    }

    SECTION("empty fields show placeholders")
    {
      // Times' placeholders are fixed: 12:30 PM.
      auto &time = fixture.Input(u8"time");
      CHECK(Shown(time) == u8"12:30\u202fPM");
      CHECK(time.Value().empty());
    }

    SECTION("Arabic digits")
    {
      auto &date = fixture.Input(u8"date", u8"ar-EG", {{u8"value", u8"2026-03-07"}});
      CHECK(Shown(date).find(u8"\u0662\u0660\u0662\u0666") != utf8_string::npos);
    }

    SECTION("the visible value is localised")
    {
      auto &date = fixture.Input(u8"date", u8"en-GB", {{u8"value", u8"2026-03-07"}});
      CHECK(date.VisibleValue() == u8"07/03/2026");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: editing with the keyboard",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    DateFixture fixture;
    auto &input = fixture.Input(u8"date");
    auto inputs = fixture.Listen(u8"input");
    auto changes = fixture.Listen(u8"change");

    // Focusing the input focuses its first field.
    input.Focus();
    REQUIRE(fixture.Document->FocusedElement() == &Field(input, 0));

    SECTION("typing digits, moving between fields")
    {
      CHECK(fixture.Type(u8"0", 0));
      CHECK(fixture.Type(u8"3", 0));
      // Right, or a separator, moves to the next field.
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Document->FocusedElement() == &Field(input, 1));
      CHECK(fixture.Type(u8"7", 0));
      CHECK(fixture.Press(u8"U+002F", u8"Slash"));
      CHECK(fixture.Document->FocusedElement() == &Field(input, 2));
      // A partly filled date is bad input, and has no value.
      CHECK(input.Value().empty());
      CHECK(input.Validity().BadInput());
      CHECK(inputs->Count == 0);

      CHECK(fixture.Type(u8"2", 0));
      CHECK(fixture.Type(u8"0", 0));
      CHECK(fixture.Type(u8"2", 0));
      CHECK(fixture.Type(u8"6", 0));
      CHECK(input.Value() == u8"2026-03-07");
      CHECK_FALSE(input.Validity().BadInput());
      // Each digit of the year made a valid date: 0002, 0020, 0202 and 2026.
      CHECK(inputs->Count == 4);
      CHECK(changes->Count == 0);

      // Left goes back.
      CHECK(fixture.Press(u8"Left"));
      CHECK(fixture.Document->FocusedElement() == &Field(input, 1));

      // Leaving the fields fires change.
      input.Blur();
      CHECK(changes->Count == 1);
    }

    SECTION("typing starts over after a pause")
    {
      CHECK(fixture.Type(u8"1", 0));
      CHECK(Shown(input).starts_with(u8"01/"));
      CHECK(fixture.Type(u8"2", 5));
      CHECK(Shown(input).starts_with(u8"02/"));
      CHECK(fixture.Type(u8"1", 5));
      CHECK(Shown(input).starts_with(u8"12/"));
    }

    SECTION("the arrow keys step a field, wrapping")
    {
      CHECK(fixture.Press(u8"Up"));
      CHECK(Shown(input).starts_with(u8"01/"));
      CHECK(fixture.Press(u8"Down"));
      CHECK(Shown(input).starts_with(u8"12/"));
      CHECK(fixture.Press(u8"Down"));
      CHECK(Shown(input).starts_with(u8"11/"));
      // Backspace empties the field.
      CHECK(fixture.Press(u8"U+0008"));
      CHECK_FALSE(input.Validity().BadInput());
    }

    SECTION("a read-only input's fields don't change")
    {
      (void)input.SetAttribute(u8"readonly", u8"");
      CHECK_FALSE(fixture.Type(u8"4", 0));
      CHECK_FALSE(input.Validity().BadInput());
      // They can still be moved between.
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Document->FocusedElement() == &Field(input, 1));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: names and periods",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    DateFixture fixture;

    SECTION("a month's name is chosen by typing its start")
    {
      auto &month = fixture.Input(u8"month");
      month.Focus();
      CHECK(fixture.Type(u8"m", 0));
      CHECK(Shown(month).starts_with(u8"Mar "));
      // Typing the same letter again cycles through the names starting with it.
      CHECK(fixture.Type(u8"m", 0));
      CHECK(Shown(month).starts_with(u8"May "));
      CHECK(fixture.Press(u8"Up"));
      CHECK(Shown(month).starts_with(u8"Jun "));
    }

    SECTION("AM and PM")
    {
      auto &time = fixture.Input(u8"time", u8"en-US", {{u8"value", u8"09:15"}});
      time.Focus();
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Type(u8"p", 0));
      CHECK(time.Value() == u8"21:15");
      CHECK(fixture.Press(u8"Down"));
      CHECK(time.Value() == u8"09:15");
      // Typing quickly switches back and forth: what was typed before doesn't stick.
      CHECK(fixture.Type(u8"p", 0));
      CHECK(time.Value() == u8"21:15");
      CHECK(fixture.Type(u8"a", 0));
      CHECK(time.Value() == u8"09:15");
      CHECK(fixture.Type(u8"p", 0));
      CHECK(time.Value() == u8"21:15");
    }

    SECTION("24 hour clocks")
    {
      auto &time = fixture.Input(u8"time", u8"de");
      time.Focus();
      CHECK(fixture.Type(u8"2", 0));
      CHECK(fixture.Type(u8"3", 0));
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Type(u8"5", 0));
      CHECK(time.Value() == u8"23:05");
    }

    SECTION("weeks")
    {
      auto &week = fixture.Input(u8"week");
      week.Focus();
      CHECK(fixture.Type(u8"9", 0));
      CHECK(fixture.Press(u8"Right"));
      CHECK(fixture.Type(u8"2", 0));
      CHECK(fixture.Type(u8"0", 0));
      CHECK(fixture.Type(u8"2", 0));
      CHECK(fixture.Type(u8"6", 0));
      CHECK(week.Value() == u8"2026-W09");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: focusing the fields",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    DateFixture fixture;
    auto &input = fixture.Input(u8"date");

    SECTION("sequential navigation moves between the fields")
    {
      auto order = html::FocusAlgorithms::SequentialFocusNavigationOrder(*fixture.Document);
      REQUIRE(order.size() == 3);
      CHECK(order[0] == &Field(input, 0));
      CHECK(order[2] == &Field(input, 2));
    }

    SECTION("a click focuses the field, or the first field")
    {
      CHECK(html::FocusAlgorithms::ClickFocusableInclusiveAncestor(Field(input, 1)) == &Field(input, 1));
      html::FocusAlgorithms::FocusForClick(input);
      CHECK(fixture.Document->FocusedElement() == &Field(input, 0));
    }

    SECTION("a disabled input's fields aren't focusable")
    {
      (void)input.SetAttribute(u8"disabled", u8"");
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(html::FocusAlgorithms::IsFocusableArea(Field(input, 0)));
      CHECK(html::FocusAlgorithms::SequentialFocusNavigationOrder(*fixture.Document).empty());
      input.Focus();
      CHECK(fixture.Document->FocusedElement() != &Field(input, 0));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BaseDateAndTimeInputType: the date chooser",
                   "[HTML][BaseDateAndTimeInputType]")
  {
    auto chromeClient = CreateUnique<DateChromeClient>();
    auto *client = chromeClient.get();
    DateFixture fixture(krys::move(chromeClient));
    auto &input =
      fixture.Input(u8"date", u8"en-US", {{u8"value", u8"2026-03-07"}, {u8"min", u8"2026-01-01"}});
    auto inputs = fixture.Listen(u8"input");
    auto changes = fixture.Listen(u8"change");

    SECTION("showPicker() shows it with the input's value and range")
    {
      CHECK_FALSE(input.ShowPicker().HasException());
      REQUIRE(client->Chooser);
      REQUIRE(client->Chooser->Parameters);
      auto &parameters = *client->Chooser->Parameters;
      CHECK(parameters.Type == u8"date");
      CHECK(parameters.CurrentValue == u8"2026-03-07");
      CHECK(parameters.Locale == u8"en-US");
      CHECK(parameters.Minimum == input.Minimum());
      // A day, in milliseconds.
      CHECK(parameters.Step == 86'400'000.0);
      CHECK_FALSE(parameters.AnchorRectInRootView.IsEmpty());
      CHECK(input.IsPresentingAttachedView());

      // Choosing a value sets it, as the user would.
      client->Chooser->Client->DidChooseValue(u8"2026-04-01");
      CHECK(input.Value() == u8"2026-04-01");
      CHECK(Shown(input) == u8"04/01/2026");
      CHECK(inputs->Count == 1);
      CHECK(changes->Count == 1);
    }

    SECTION("a click shows it; leaving the fields ends it")
    {
      input.Click();
      REQUIRE(client->Chooser);
      CHECK(client->Chooser->Shows == 1);
      // Editing a field updates it.
      input.Focus();
      CHECK(fixture.Press(u8"Up"));
      CHECK(client->Chooser->Shows == 2);
      CHECK(client->Chooser->Parameters->CurrentValue == u8"2026-04-07");
      input.Blur();
      CHECK(client->Chooser->Ended);
      CHECK_FALSE(input.IsPresentingAttachedView());
    }

    SECTION("Space in a field shows it")
    {
      input.Focus();
      CHECK(fixture.Press(u8"U+0020", u8"Space", vk::Space));
      REQUIRE(client->Chooser);
      CHECK(client->Chooser->Parameters->WasActivatedByKeyboard);
    }

    SECTION("hiding the input ends it")
    {
      CHECK_FALSE(input.ShowPicker().HasException());
      REQUIRE(client->Chooser);
      (void)input.SetAttribute(u8"style", u8"display:none");
      (void)fixture.Document->UpdateLayout();
      CHECK(client->Chooser->Ended);
    }

    SECTION("read-only inputs and times have none")
    {
      (void)input.SetAttribute(u8"readonly", u8"");
      CHECK(input.ShowPicker().HasException());
      auto &time = fixture.Input(u8"time");
      CHECK_FALSE(time.ShowPicker().HasException());
      CHECK_FALSE(client->Chooser);
    }
  }
}
