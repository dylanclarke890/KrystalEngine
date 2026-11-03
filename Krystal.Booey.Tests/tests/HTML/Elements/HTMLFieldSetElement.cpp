#include "Krystal.Booey/HTML/Elements/HTMLFieldSetElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLegendElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  TEST_CASE("HTMLFieldSetElement: elements are its listed descendants, the same collection each time",
            "[HTML][Forms][HTMLFieldSetElement][html:dom-fieldset-elements][html:dom-fieldset-type]")
  {
    FormFixture fixture(
      u8"<fieldset id=f><input><div><output></output><img></div><fieldset></fieldset></fieldset>");
    auto &fieldset = fixture.Get<HTMLFieldSetElement>(u8"f");
    auto elements = fieldset.Elements();
    CHECK(elements->Length() == 3uz);
    CHECK(fieldset.Elements().get() == elements.get());
    CHECK(fieldset.FormControlType() == u8"fieldset");
  }

  TEST_CASE(
    "HTMLFieldSetElement: a disabled fieldset disables its controls, but not those in its first legend",
    "[HTML][Forms][HTMLFieldSetElement][html:attr-fieldset-disabled][html:concept-fieldset-disabled]")
  {
    FormFixture fixture(
      u8"<fieldset id=f disabled><legend><input id=first></legend><legend><input id=second>"
      u8"</legend><input id=plain><fieldset id=inner><input id=nested></fieldset></fieldset>");
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"first").IsDisabledFormControl());
    CHECK(fixture.Get<HTMLInputElement>(u8"second").IsDisabledFormControl());
    CHECK(fixture.Get<HTMLInputElement>(u8"plain").IsDisabledFormControl());
    CHECK(fixture.Get<HTMLInputElement>(u8"nested").IsDisabledFormControl());
    CHECK(fixture.Get<HTMLFieldSetElement>(u8"inner").IsActuallyDisabled());

    fixture.Get<HTMLFieldSetElement>(u8"f").Disabled(false);
    CHECK_FALSE(fixture.Get<HTMLInputElement>(u8"plain").IsDisabledFormControl());
    CHECK_FALSE(fixture.Get<HTMLFieldSetElement>(u8"inner").IsActuallyDisabled());
  }

  TEST_CASE("HTMLLegendElement: form is its parent fieldset's form",
            "[HTML][Forms][HTMLLegendElement][html:dom-legend-form][html:the-legend-element]")
  {
    FormFixture fixture(u8"<form id=f><fieldset><legend id=in></legend></fieldset><legend id=out></legend>"
                        u8"<div><legend id=div></legend></div></form>");
    CHECK(fixture.Get<HTMLLegendElement>(u8"in").Form() == &fixture.Get<HTMLFormElement>(u8"f"));
    CHECK_FALSE(fixture.Get<HTMLLegendElement>(u8"out").Form());
    CHECK_FALSE(fixture.Get<HTMLLegendElement>(u8"div").Form());
  }
}
