#include "Krystal.Booey/HTML/ValidatedFormListedElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::EventLog;
  using boo::tests::FormFixture;
  using boo::tests::Str;

  namespace
  {
    KRYS_NODISCARD bool Matches(const dom::Element &element, dom::DOMStringView selector)
    {
      auto result = element.Matches(selector);
      REQUIRE_FALSE(result.HasException());
      return result.ReleaseValue();
    }
  }

  TEST_CASE("Constraint validation: value missing", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form id=f><fieldset id=fs><input id=t required></fieldset>
      <textarea id=ta required></textarea><select id=s required><option value="">-</option><option id=x>x</option>
      </select></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    auto &ta = fixture.Get<HTMLTextAreaElement>(u8"ta");
    auto &s = fixture.Get<HTMLSelectElement>(u8"s");

    CHECK(t.WillValidate());
    CHECK(t.ValueMissing());
    CHECK(ta.ValueMissing());
    CHECK(s.ValueMissing());
    CHECK(Str(t.ValidationMessage()) == "Fill out this field");
    CHECK(Matches(t, u8":invalid"));
    CHECK(Matches(t, u8":required"));
    CHECK(Matches(fixture.ById(u8"fs"), u8":invalid"));
    CHECK(Matches(form, u8":invalid"));
    CHECK_FALSE(Matches(t, u8":user-invalid"));

    auto invalidEvents = EventLog::ListenTo(t, {u8"invalid"});
    CHECK_FALSE(form.CheckValidity());
    CHECK(invalidEvents->Joined() == "invalid");

    (void)t.SetValue(u8"a");
    (void)ta.SetValue(u8"b");
    fixture.Get<HTMLOptionElement>(u8"x").Selected(true);
    CHECK_FALSE(t.ValueMissing());
    CHECK_FALSE(s.ValueMissing());
    CHECK(Matches(t, u8":valid"));
    CHECK(Matches(fixture.ById(u8"fs"), u8":valid"));
    CHECK(Matches(form, u8":valid"));
    CHECK(form.CheckValidity());
  }

  TEST_CASE("Constraint validation: barred controls", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<form><input id=hidden type=hidden required><input id=disabled disabled required>
      <input id=readonly readonly required><button id=button type=button></button><button id=submit></button>
      <fieldset disabled><legend><input id=inlegend></legend><input id=infieldset></fieldset></form>)");
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"hidden").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"disabled").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"readonly").WillValidate());
    CHECK_FALSE(fixture.Get<HTMLButtonElement>(u8"button").WillValidate());
    CHECK(fixture.Get<HTMLButtonElement>(u8"submit").WillValidate());

    // A disabled fieldset disables its descendants, except those in its first legend.
    auto &inLegend = fixture.Get<HTMLInputElement>(u8"inlegend");
    auto &inFieldset = fixture.Get<HTMLInputElement>(u8"infieldset");
    CHECK_FALSE(inLegend.IsDisabledFormControl());
    CHECK(inFieldset.IsDisabledFormControl());
    CHECK(Matches(inFieldset, u8":disabled"));
    CHECK(Matches(inLegend, u8":enabled"));
    CHECK(Matches(fixture.ById(u8"readonly"), u8":read-only"));
    CHECK(Matches(inLegend, u8":read-write"));
  }

  TEST_CASE("Constraint validation: custom validity", "[HTML][Forms]")
  {
    FormFixture fixture(u8R"(<input id=t>)");
    auto &t = fixture.Get<HTMLInputElement>(u8"t");
    t.SetCustomValidity(u8"Not like that");
    CHECK(t.CustomError());
    CHECK(Str(t.ValidationMessage()) == "Not like that");
    CHECK(Matches(t, u8":invalid"));
    t.SetCustomValidity(u8"");
    CHECK_FALSE(t.CustomError());
    CHECK(Matches(t, u8":valid"));
  }
}
