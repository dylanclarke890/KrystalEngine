#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include <cmath>

namespace krys::boo::html::tests
{
  using boo::tests::EventLog;
  using boo::tests::FormFixture;
  using boo::tests::Str;

  namespace
  {
    /// @brief The value an input of type @p type gets when its value is set to @p value.
    KRYS_NODISCARD std::string Sanitized(dom::DOMStringView type, dom::DOMStringView value,
                                         dom::DOMStringView attributes = {})
    {
      FormFixture fixture(u8"<input id=i " + utf8_string(attributes) + u8">");
      auto &input = fixture.Get<HTMLInputElement>(u8"i");
      REQUIRE_FALSE(input.SetAttribute(u8"type", dom::DOMString(type)).HasException());
      REQUIRE_FALSE(input.SetValue(value).HasException());
      return Str(input.Value());
    }

    KRYS_NODISCARD bool Matches(const dom::Element &element, dom::DOMStringView selector)
    {
      auto result = element.Matches(selector);
      REQUIRE_FALSE(result.HasException());
      return result.ReleaseValue();
    }
  }

  TEST_CASE("HTMLInputElement: type", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=i>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    CHECK(Str(input.Type()) == "text");
    input.Type(u8"CheckBox");
    CHECK(Str(input.Type()) == "checkbox");
    CHECK(input.IsCheckbox());
    input.Type(u8"nonsense");
    CHECK(Str(input.Type()) == "text");
    input.Type(u8"datetime-local");
    CHECK(input.IsDateTimeLocalField());
    CHECK(Str(input.GetAttribute(u8"type").value_or(u8"")) == "datetime-local");
  }

  TEST_CASE("HTMLInputElement: value sanitisation", "[HTML][Forms][Input]")
  {
    CHECK(Sanitized(u8"text", u8"a\r\nb\nc") == "abc");
    CHECK(Sanitized(u8"search", u8"  s  ") == "  s  ");
    CHECK(Sanitized(u8"url", u8"  https://example.com/  ") == "https://example.com/");
    CHECK(Sanitized(u8"email", u8" a@b.c ") == "a@b.c");
    CHECK(Sanitized(u8"email", u8" a@b.c , d@e.f", u8"multiple") == "a@b.c,d@e.f");
    CHECK(Sanitized(u8"number", u8"1e3") == "1e3");
    CHECK(Sanitized(u8"number", u8"abc").empty());
    CHECK(Sanitized(u8"number", u8"+1").empty());
    CHECK(Sanitized(u8"range", u8"") == "50");
    CHECK(Sanitized(u8"range", u8"150") == "100");
    CHECK(Sanitized(u8"range", u8"7", u8"min=0 max=10 step=5") == "5");
    CHECK(Sanitized(u8"range", u8"1", u8"min=10 max=5") == "10");
    CHECK(Sanitized(u8"color", u8"#ABCDEF") == "#abcdef");
    CHECK(Sanitized(u8"color", u8"red") == "#000000");
    CHECK(Sanitized(u8"date", u8"2026-02-30").empty());
    CHECK(Sanitized(u8"date", u8"2026-02-28") == "2026-02-28");
    CHECK(Sanitized(u8"time", u8"25:00").empty());
    CHECK(Sanitized(u8"month", u8"2026-10") == "2026-10");
    CHECK(Sanitized(u8"week", u8"2026-W53") == "2026-W53");
    CHECK(Sanitized(u8"week", u8"2025-W53").empty());
    CHECK(Sanitized(u8"datetime-local", u8"2026-10-22T09:30:00") == "2026-10-22T09:30");
  }

  TEST_CASE("HTMLInputElement: value modes", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=i value=initial>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");

    SECTION("value: a dirty value separate from the attribute")
    {
      CHECK(Str(input.Value()) == "initial");
      (void)input.SetValue(u8"typed");
      CHECK(Str(input.Value()) == "typed");
      CHECK(Str(input.DefaultValue()) == "initial");
      input.DefaultValue(u8"other");
      CHECK(Str(input.Value()) == "typed");
    }

    SECTION("default: the value is the attribute")
    {
      input.Type(u8"hidden");
      (void)input.SetValue(u8"set");
      CHECK(Str(input.GetAttribute(u8"value").value_or(u8"")) == "set");
    }

    SECTION("A dirty value moves into the attribute when the type stops storing it")
    {
      (void)input.SetValue(u8"typed");
      input.Type(u8"submit");
      CHECK(Str(input.GetAttribute(u8"value").value_or(u8"")) == "typed");
      input.Type(u8"text");
      CHECK(Str(input.Value()) == "typed");
    }

    SECTION("default/on: checkboxes")
    {
      input.RemoveAttribute(u8"value");
      input.Type(u8"checkbox");
      CHECK(Str(input.Value()) == "on");
    }

    SECTION("filename: files can only be cleared")
    {
      input.Type(u8"file");
      CHECK(input.Value().empty());
      CHECK(input.SetValue(u8"C:\\x").HasException());
      CHECK_FALSE(input.SetValue(u8"").HasException());
    }
  }

  TEST_CASE("HTMLInputElement: validity", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=i>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    auto validAs = [&](dom::DOMStringView type, dom::DOMStringView value)
    {
      input.Type(dom::DOMString(type));
      (void)input.SetValue(value);
      return !input.TypeMismatch();
    };
    CHECK(validAs(u8"email", u8"someone@example.com"));
    CHECK_FALSE(validAs(u8"email", u8"someone"));
    CHECK(validAs(u8"url", u8"https://example.com"));
    CHECK_FALSE(validAs(u8"url", u8"example"));

    SECTION("pattern")
    {
      input.Type(u8"text");
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"[a-c]+").HasException());
      (void)input.SetValue(u8"abc");
      CHECK_FALSE(input.PatternMismatch());
      (void)input.SetValue(u8"abcd");
      CHECK(input.PatternMismatch());
      CHECK(Matches(input, u8":invalid"));
      // An invalid pattern matches anything.
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"(").HasException());
      CHECK_FALSE(input.PatternMismatch());
    }

    SECTION("ranges and steps")
    {
      input.Type(u8"number");
      REQUIRE_FALSE(input.SetAttribute(u8"min", u8"1").HasException());
      REQUIRE_FALSE(input.SetAttribute(u8"max", u8"10").HasException());
      REQUIRE_FALSE(input.SetAttribute(u8"step", u8"2").HasException());
      (void)input.SetValue(u8"0");
      CHECK(input.RangeUnderflow());
      CHECK(Matches(input, u8":out-of-range"));
      (void)input.SetValue(u8"11");
      CHECK(input.RangeOverflow());
      (void)input.SetValue(u8"4");
      CHECK(input.StepMismatch());
      (void)input.SetValue(u8"5");
      CHECK_FALSE(input.StepMismatch());
      CHECK(Matches(input, u8":in-range"));
      CHECK(Matches(input, u8":valid"));
    }

    SECTION("tooLong only for what the user typed")
    {
      input.Type(u8"text");
      input.MaxLength(2);
      (void)input.SetValue(u8"abc");
      CHECK_FALSE(input.TooLong());
      // Values set for the user (autofill, pickers) aren't typed either.
      input.SetValueForUser(u8"abcd");
      CHECK_FALSE(input.TooLong());
      CHECK_FALSE(input.IsValidValue(u8"abcd"));
    }
  }

  TEST_CASE("HTMLInputElement: numbers and dates", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=i type=number value=5 step=2 min=1>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    CHECK(input.ValueAsNumber() == 5);
    REQUIRE_FALSE(input.StepUp().HasException());
    CHECK(Str(input.Value()) == "7");
    REQUIRE_FALSE(input.StepDown(3).HasException());
    CHECK(Str(input.Value()) == "1");
    REQUIRE_FALSE(input.SetValueAsNumber(2.5).HasException());
    CHECK(Str(input.Value()) == "2.5");
    CHECK(input.SetValueAsNumber(std::numeric_limits<double>::infinity()).HasException());

    SECTION("dates")
    {
      input.Type(u8"date");
      (void)input.SetValue(u8"1970-01-02");
      CHECK(input.ValueAsDate() == 86400000.0);
      CHECK(input.ValueAsNumber() == 86400000.0);
      REQUIRE_FALSE(input.SetValueAsDate(0).HasException());
      CHECK(Str(input.Value()) == "1970-01-01");
      input.Type(u8"text");
      CHECK(std::isnan(input.ValueAsDate()));
      CHECK(input.StepUp().HasException());
    }
  }

  TEST_CASE("HTMLInputElement: checkboxes", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=c type=checkbox checked>");
    auto &input = fixture.Get<HTMLInputElement>(u8"c");
    CHECK(input.Checked());
    CHECK(Matches(input, u8":checked"));
    CHECK(Matches(input, u8":default"));

    auto events = EventLog::ListenTo(input, {u8"click", u8"input", u8"change"});
    input.Click();
    CHECK_FALSE(input.Checked());
    CHECK(events->Joined() == "click input change");

    SECTION("Canceling the click restores the state")
    {
      input.SetIndeterminate(true);
      events->PreventDefault = true;
      events->Types.clear();
      input.Click();
      CHECK_FALSE(input.Checked());
      CHECK(input.Indeterminate());
      CHECK(events->Joined() == "click");
    }

    SECTION("The checked attribute only applies until the checkedness is dirty")
    {
      input.RemoveAttribute(u8"checked");
      REQUIRE_FALSE(input.SetAttribute(u8"checked", u8"").HasException());
      CHECK_FALSE(input.Checked());
    }

    SECTION("Required checkboxes must be checked")
    {
      REQUIRE_FALSE(input.SetAttribute(u8"required", u8"").HasException());
      CHECK(input.ValueMissing());
      input.Checked(true);
      CHECK_FALSE(input.ValueMissing());
    }
  }

  TEST_CASE("HTMLInputElement: radio buttons", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8R"(<form id=f><input id=a type=radio name=g checked><input id=b type=radio name=g>
      <input id=c type=radio name=other></form><input id=d type=radio name=g>)");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &b = fixture.Get<HTMLInputElement>(u8"b");
    auto &c = fixture.Get<HTMLInputElement>(u8"c");
    auto &d = fixture.Get<HTMLInputElement>(u8"d");
    CHECK(a.Checked());

    b.Checked(true);
    CHECK_FALSE(a.Checked());
    CHECK(b.Checked());

    // Groups are per name and per form owner.
    c.Checked(true);
    d.Checked(true);
    CHECK(b.Checked());
    CHECK(c.Checked());
    CHECK(d.Checked());
    CHECK(a.CheckedRadioButtonForGroup().get() == &b);
    CHECK(a.RadioButtonGroup().size() == 2);

    SECTION("Clicking checks; canceling restores the checked button")
    {
      auto events = EventLog::ListenTo(a, {u8"click", u8"change"});
      a.Click();
      CHECK(a.Checked());
      CHECK_FALSE(b.Checked());
      CHECK(events->Joined() == "click change");

      events->PreventDefault = true;
      b.AddEventListener(dom::DOMStringAtom(u8"click"), Ref<dom::EventListenerCallback> {events}, false);
      b.Click();
      CHECK(a.Checked());
      CHECK_FALSE(b.Checked());
    }

    SECTION("A required group is missing a value until one is checked")
    {
      b.Checked(false);
      REQUIRE_FALSE(a.SetAttribute(u8"required", u8"").HasException());
      CHECK(a.ValueMissing());
      CHECK(b.ValueMissing());
      CHECK(Matches(b, u8":indeterminate"));
      b.Checked(true);
      CHECK_FALSE(a.ValueMissing());
      CHECK_FALSE(Matches(b, u8":indeterminate"));
    }

    SECTION("Moving a button into the group's form joins the group")
    {
      REQUIRE_FALSE(fixture.ById(u8"f").AppendChild(d).HasException());
      CHECK(d.Checked());
      CHECK_FALSE(b.Checked());
    }
  }
}
