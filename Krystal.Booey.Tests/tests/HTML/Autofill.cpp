#include "Krystal.Booey/HTML/Autofill.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;
  using boo::tests::Str;

  TEST_CASE("Autofill: the IDL-exposed autofill value",
            "[HTML][Forms][Autofill][html:idl-exposed-autofill-value][html:dom-fe-autocomplete]"
            "[html:determine-a-field's-category][html:autofill-detail-tokens]")
  {
    FormFixture fixture(u8"<input id=i><input id=h type=hidden><select id=s></select>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    auto autocomplete = [&](dom::DOMStringView value)
    {
      REQUIRE_FALSE(input.SetAttribute(u8"autocomplete", dom::DOMString(value)).HasException());
      return Str(input.Autocomplete());
    };
    CHECK(Str(input.Autocomplete()).empty());
    CHECK(autocomplete(u8"on") == "on");
    CHECK(autocomplete(u8" OFF ") == "off");
    CHECK(autocomplete(u8"off on").empty());
    CHECK(autocomplete(u8"bogus").empty());
    CHECK(autocomplete(u8"name") == "name");
    CHECK(autocomplete(u8"shipping name") == "shipping name");
    CHECK(autocomplete(u8"Section-A BILLING Home tel") == "section-a billing home tel");
    CHECK(autocomplete(u8"home name").empty());
    CHECK(autocomplete(u8"a b tel").empty());
    CHECK(autocomplete(u8"webauthn") == "webauthn");
    CHECK(autocomplete(u8"username webauthn") == "username webauthn");
    CHECK(autocomplete(u8"section-x work email webauthn") == "section-x work email webauthn");
    CHECK(autocomplete(u8"on webauthn").empty());

    // A hidden input's attribute wears the autofill anchor mantle: on and off are no values.
    auto &hidden = fixture.Get<HTMLInputElement>(u8"h");
    REQUIRE_FALSE(hidden.SetAttribute(u8"autocomplete", u8"on").HasException());
    CHECK(Str(hidden.Autocomplete()).empty());
    REQUIRE_FALSE(hidden.SetAttribute(u8"autocomplete", u8"email").HasException());
    CHECK(Str(hidden.Autocomplete()) == "email");

    auto &select = fixture.Get<HTMLSelectElement>(u8"s");
    REQUIRE_FALSE(select.SetAttribute(u8"autocomplete", u8"country").HasException());
    CHECK(Str(select.Autocomplete()) == "country");
  }

  TEST_CASE("Autofill: the field name, hint set, scope and credential type",
            "[HTML][Forms][Autofill][html:autofill-field-name][html:autofill-hint-set][html:autofill-scope]"
            "[html:non-autofill-credential-type][html:autofill-processing-model]")
  {
    FormFixture fixture(
      u8"<form id=f autocomplete=off><input id=i><input id=h type=hidden></form><input id=o>");
    auto &input = fixture.Get<HTMLInputElement>(u8"i");
    CHECK(Str(AutofillData::Create(input).FieldName) == "off");
    CHECK(Str(AutofillData::Create(fixture.Get<HTMLInputElement>(u8"o")).FieldName) == "on");
    CHECK(Str(AutofillData::Create(fixture.Get<HTMLInputElement>(u8"h")).FieldName).empty());

    REQUIRE_FALSE(
      input.SetAttribute(u8"autocomplete", u8"section-a shipping mobile tel webauthn").HasException());
    auto data = AutofillData::Create(input);
    CHECK(Str(data.FieldName) == "webauthn");
    CHECK(data.CredentialType == NonAutofillCredentialType::WebAuthn);
    CHECK(data.HintSet == List<dom::DOMString> {u8"mobile", u8"shipping"});
    CHECK(data.Scope == List<dom::DOMString> {u8"section-a", u8"shipping", u8"mobile"});
  }
}
