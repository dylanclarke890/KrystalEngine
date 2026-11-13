#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
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

  TEST_CASE("HTMLInputElement: the value and checked attributes, before the user changes them",
            "[HTML][Forms][Input][html:attr-input-value][html:attr-input-checked]"
            "[html:concept-input-checked-dirty-flag][html:dom-input-checked]")
  {
    FormFixture fixture(u8"<input id=t value=a><input id=c type=checkbox>");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    REQUIRE_FALSE(t.SetAttribute(u8"value", u8"b").HasException());
    CHECK(Str(t.Value()) == "b");
    // A dirty value keeps its value.
    REQUIRE_FALSE(t.SetValue(u8"typed").HasException());
    REQUIRE_FALSE(t.SetAttribute(u8"value", u8"c").HasException());
    CHECK(Str(t.Value()) == "typed");

    auto &c = fixture.Get<HTMLInputElement>(u8"c");
    REQUIRE_FALSE(c.SetAttribute(u8"checked", u8"").HasException());
    CHECK(c.Checked());
    c.RemoveAttribute(u8"checked");
    CHECK_FALSE(c.Checked());
    // Dirty checkedness keeps its checkedness.
    c.Checked(true);
    REQUIRE_FALSE(c.SetAttribute(u8"checked", u8"").HasException());
    c.RemoveAttribute(u8"checked");
    CHECK(c.Checked());
  }

  TEST_CASE("HTMLInputElement: changing the type carries the value between the value modes",
            "[HTML][Forms][Input][html:input-activation-behavior][html:signal-a-type-change]")
  {
    FormFixture fixture(u8"<input id=t value=a>");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    REQUIRE_FALSE(t.SetValue(u8"typed").HasException());
    // From the value mode to the default/on mode, the value becomes the value attribute.
    t.Type(u8"checkbox");
    CHECK(Str(t.GetAttribute(u8"value").value_or(u8"")) == "typed");
    // And back, the value attribute is the value.
    t.Type(u8"text");
    CHECK(Str(t.Value()) == "typed");
    // To the filename mode, the value is empty.
    REQUIRE_FALSE(t.SetValue(u8"x").HasException());
    t.Type(u8"file");
    CHECK(Str(t.Value()) == "");
  }

  TEST_CASE("HTMLInputElement: clicking a checkbox or radio button, and canceling the click",
            "[HTML][Forms][Input][html:input-activation-behavior][html:input-support-picker]"
            "[html:dom-input-indeterminate]")
  {
    FormFixture fixture(u8"<input id=c type=checkbox><input id=rad1 type=radio name=g checked>"
                        u8"<input id=rad2 type=radio name=g>");
    auto &c = fixture.Get<HTMLInputElement>(u8"c");
    c.Indeterminate(true);
    CHECK(c.Indeterminate());
    auto events = EventLog::ListenTo(c, {u8"input", u8"change", u8"click"});
    c.Click();
    CHECK(events->Joined() == "click input change");
    CHECK(c.Checked());
    CHECK_FALSE(c.Indeterminate());

    // A canceled click restores the checkedness and indeterminateness.
    c.Indeterminate(true);
    events->PreventDefault = true;
    c.Click();
    CHECK(c.Checked());
    CHECK(c.Indeterminate());

    // And the radio button group's checked button.
    auto &rad1 = fixture.Get<HTMLInputElement>(u8"rad1");
    auto &rad2 = fixture.Get<HTMLInputElement>(u8"rad2");
    auto radioEvents = EventLog::ListenTo(rad2, {u8"click"});
    radioEvents->PreventDefault = true;
    rad2.Click();
    CHECK(rad1.Checked());
    CHECK_FALSE(rad2.Checked());
  }

  TEST_CASE("HTMLInputElement: email values are sanitized, and a range's default value is its middle",
            "[HTML][Forms][Input][html:value-sanitization-algorithm][html:concept-input-value-default-range]")
  {
    FormFixture fixture(u8"<input id=e type=email multiple><input id=e1 type=email>"
                        u8"<input id=r type=range min=0 max=100><input id=r2 type=range min=50 max=10>");
    auto &e = fixture.Get<HTMLInputElement>(u8"e");
    REQUIRE_FALSE(e.SetValue(u8" a@b.c , d@e.f ,").HasException());
    CHECK(Str(e.Value()) == "a@b.c,d@e.f,");
    auto &e1 = fixture.Get<HTMLInputElement>(u8"e1");
    REQUIRE_FALSE(e1.SetValue(u8" a@b\n.c ").HasException());
    CHECK(Str(e1.Value()) == "a@b.c");

    CHECK(Str(fixture.Get<HTMLInputElement>(u8"r").Value()) == "50");
    // When the maximum is less than the minimum, the minimum.
    CHECK(Str(fixture.Get<HTMLInputElement>(u8"r2").Value()) == "50");
  }

  TEST_CASE(
    "HTMLInputElement: step, stepUp, pattern and reversed ranges",
    "[HTML][Forms][Input][html:concept-input-step][html:concept-input-min-zero][html:dom-input-stepdown]"
    "[html:compiled-pattern-regular-expression][html:regexp-match-a-string][html:has-a-reversed-range]"
    "[html:concept-input-min][html:concept-input-max]")
  {
    FormFixture fixture(
      u8"<input id=n type=number step=any value=5><input id=n2 type=number min=10 max=5 value=7>"
      u8"<input id=n3 type=number value=1.5 step=1><input id=p pattern='a|b'><input id=p2 pattern='['>"
      u8"<input id=tm type=time min=22:00 max=02:00 value=23:00>");
    // No allowed value step.
    auto stepped = fixture.Get<HTMLInputElement>(u8"n").StepUp(1);
    REQUIRE(stepped.HasException());
    CHECK(stepped.GetException().Code() == dom::ExceptionCode::InvalidStateError);

    // A minimum greater than the maximum: nothing.
    auto &n2 = fixture.Get<HTMLInputElement>(u8"n2");
    CHECK_FALSE(n2.StepUp(1).HasException());
    CHECK(Str(n2.Value()) == "7");

    // The step base is the value attribute.
    auto &n3 = fixture.Get<HTMLInputElement>(u8"n3");
    CHECK_FALSE(n3.StepUp(1).HasException());
    CHECK(Str(n3.Value()) == "2.5");

    // The pattern matches the whole value, and one that doesn't compile is no constraint.
    auto &p = fixture.Get<HTMLInputElement>(u8"p");
    REQUIRE_FALSE(p.SetValue(u8"ab").HasException());
    CHECK(p.PatternMismatch());
    REQUIRE_FALSE(p.SetValue(u8"b").HasException());
    CHECK_FALSE(p.PatternMismatch());
    auto &p2 = fixture.Get<HTMLInputElement>(u8"p2");
    REQUIRE_FALSE(p2.SetValue(u8"zz").HasException());
    CHECK_FALSE(p2.PatternMismatch());

    // A value within a reversed range is fine; one outside both underflows and overflows.
    auto &tm = fixture.Get<HTMLInputElement>(u8"tm");
    CHECK_FALSE(tm.RangeOverflow());
    CHECK_FALSE(tm.RangeUnderflow());
    REQUIRE_FALSE(tm.SetValue(u8"12:00").HasException());
    CHECK(tm.RangeOverflow());
    CHECK(tm.RangeUnderflow());
  }

  TEST_CASE("HTMLInputElement: colorSpace, buttons without a form, and showPicker without activation",
            "[HTML][Forms][Input][html:dom-input-colorspace][html:submit-button-state-(type=submit)]"
            "[html:dom-input-showpicker]")
  {
    FormFixture fixture(u8"<input id=c type=color><input id=sub type=submit><input id=e type=email>");
    auto &c = fixture.Get<HTMLInputElement>(u8"c");
    CHECK(Str(c.ColourSpace()) == "limited-srgb");
    c.ColourSpace(u8"DISPLAY-P3");
    CHECK(Str(c.ColourSpace()) == "display-p3");

    fixture.Get<HTMLInputElement>(u8"sub").Click();
    CHECK(fixture.Submissions().empty());

    auto picker = fixture.Get<HTMLInputElement>(u8"e").ShowPicker();
    REQUIRE(picker.HasException());
    CHECK(picker.GetException().Code() == dom::ExceptionCode::NotAllowedError);
  }

  TEST_CASE("HTMLInputElement: type", "[HTML][Forms][Input][html:dom-input-type][html:attr-input-type]")
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

  TEST_CASE("HTMLInputElement: width and height are an image button's dimensions, and 0 for other types",
            "[HTML][Forms][Input][html:determine-dimensions][html:dom-input-width][html:dom-input-height]")
  {
    FormFixture fixture(u8"<input id=text width=50 height=20>"
                        u8"<input id=hidden type=image width=50 height=x style='display: none'>"
                        u8"<input id=shown type=image width=50 height=20 "
                        u8"style='display: inline-block; width: 40px; height: 30px; padding: 3px'>");
    CHECK(fixture.Get<HTMLInputElement>(u8"text").Width() == 0u);
    CHECK(fixture.Get<HTMLInputElement>(u8"text").Height() == 0u);

    // Without a box, the attributes that parse.
    CHECK(fixture.Get<HTMLInputElement>(u8"hidden").Width() == 50u);
    CHECK(fixture.Get<HTMLInputElement>(u8"hidden").Height() == 0u);

    // With one, its content box.
    CHECK(fixture.Get<HTMLInputElement>(u8"shown").Width() == 40u);
    CHECK(fixture.Get<HTMLInputElement>(u8"shown").Height() == 30u);
  }

  TEST_CASE("HTMLInputElement: value sanitisation", "[HTML][Forms][Input][html:value-sanitization-algorithm]")
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
    CHECK(Sanitized(u8"color", u8"red") == "#ff0000");
    CHECK(Sanitized(u8"color", u8"nonsense") == "#000000");
    CHECK(Sanitized(u8"date", u8"2026-02-30").empty());
    CHECK(Sanitized(u8"date", u8"2026-02-28") == "2026-02-28");
    CHECK(Sanitized(u8"time", u8"25:00").empty());
    CHECK(Sanitized(u8"month", u8"2026-10") == "2026-10");
    CHECK(Sanitized(u8"week", u8"2026-W53") == "2026-W53");
    CHECK(Sanitized(u8"week", u8"2025-W53").empty());
    CHECK(Sanitized(u8"datetime-local", u8"2026-10-22T09:30:00") == "2026-10-22T09:30");
  }

  TEST_CASE("HTMLInputElement: value modes", "[HTML][Forms][Input][html:dom-input-value-value]")
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

  TEST_CASE("HTMLInputElement: validity",
            "[HTML][Forms][Input][html:the-maxlength-and-minlength-attributes][html:the-required-attribute]"
            "[html:the-pattern-attribute][html:the-min-and-max-attributes][html:the-step-attribute]")
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

    SECTION("email addresses")
    {
      CHECK(validAs(u8"email", u8"a.b!#$%&'*+/=?^_`{|}~-@localhost"));
      CHECK(validAs(u8"email", u8"a@x-1.EXAMPLE"));
      CHECK(validAs(u8"email", dom::DOMString(u8"a@") + dom::DOMString(63, u8'x')));
      CHECK_FALSE(validAs(u8"email", dom::DOMString(u8"a@") + dom::DOMString(64, u8'x')));
      CHECK_FALSE(validAs(u8"email", u8"@example.com"));
      CHECK_FALSE(validAs(u8"email", u8"a@"));
      CHECK_FALSE(validAs(u8"email", u8"a@b..c"));
      CHECK_FALSE(validAs(u8"email", u8"a@b."));
      CHECK_FALSE(validAs(u8"email", u8"a@-b.c"));
      CHECK_FALSE(validAs(u8"email", u8"a@b-.c"));
      CHECK_FALSE(validAs(u8"email", u8"a@b@c"));
      CHECK_FALSE(validAs(u8"email", u8"a(b)@c"));
      CHECK_FALSE(validAs(u8"email", u8"\u00E9@example.com"));
    }
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
      // The pattern must compile on its own, before it's anchored.
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"a)|(b").HasException());
      CHECK_FALSE(input.PatternMismatch());
      // Unknown escapes are errors, as with the v flag.
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"\\q").HasException());
      (void)input.SetValue(u8"q");
      CHECK_FALSE(input.PatternMismatch());

      // Alternatives are anchored as a whole.
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"a|b").HasException());
      (void)input.SetValue(u8"ab");
      CHECK(input.PatternMismatch());

      // Patterns match code points, and know Unicode properties.
      REQUIRE_FALSE(input.SetAttribute(u8"pattern", u8"\\p{L}.").HasException());
      (void)input.SetValue(u8"\u00E9\U0001F600");
      CHECK_FALSE(input.PatternMismatch());
      (void)input.SetValue(u8"1\U0001F600");
      CHECK(input.PatternMismatch());
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

  TEST_CASE("HTMLInputElement: numbers and dates", "[HTML][Forms][Input][html:dom-input-stepdown]")
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

  TEST_CASE("HTMLInputElement: checkboxes", "[HTML][Forms][Input][html:checkbox-state-(type=checkbox)]")
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

  TEST_CASE("HTMLInputElement: radio buttons", "[HTML][Forms][Input][html:radio-button-group]")
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
      b.AddEventListener(dom::DOMStringAtom(u8"click"), events, false);
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

  TEST_CASE("HTMLInputElement: a list naming a shadow host takes the datalist its reference target names",
            "[HTML][Forms][Input][html:concept-input-list][html:resolve-the-reference-target]")
  {
    FormFixture fixture(u8R"(<input id=i list=h><x-host id=h></x-host>)");
    // A shadow root on the element with ID @p id, its reference target @p target, holding @p markup.
    auto attach = [&](dom::DOMStringView id, const char8_t *target, const char8_t *markup)
    {
      auto shadow = fixture.ById(id).AttachShadow({.ReferenceTarget = dom::DOMString(target)}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTMLUnsafe(markup).HasException());
      return shadow;
    };
    auto shadow = attach(u8"h", u8"d", u8"<datalist id=d><option value=x></datalist>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    CHECK(input.List().get() == shadow->GetElementById(u8"d").get());
    shadow->SetReferenceTarget(null);
    CHECK_FALSE(input.List());
  }
}
