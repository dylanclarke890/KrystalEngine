#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOutputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  TEST_CASE("Form owners: the nearest ancestor form", "[HTML][Forms][html:reset-the-form-owner][html:concept-"
                                                      "form-association][html:form-owner][html:dom-fae-form]")
  {
    FormFixture fixture(u8R"(<form id=f><input id=a><fieldset id=fs><select id=s></select></fieldset></form>
      <input id=b><form id=g></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &b = fixture.Get<HTMLInputElement>(u8"b");
    CHECK(a.Form() == &form);
    CHECK(fixture.Get<HTMLSelectElement>(u8"s").Form() == &form);
    CHECK(fixture.Get<HTMLFieldSetElement>(u8"fs").Form() == &form);
    CHECK(b.Form() == nullptr);

    SECTION("Moving a control changes its owner")
    {
      auto &g = fixture.Get<HTMLFormElement>(u8"g");
      REQUIRE_FALSE(g.AppendChild(a).HasException());
      CHECK(a.Form() == &g);
      Ref<HTMLInputElement> protectedA = ShareRef(a);
      a.Remove();
      CHECK(a.Form() == nullptr);
    }

    SECTION("The form's listed elements are in tree order")
    {
      CHECK(form.Length() == 3);
      CHECK(form.Item(0).get() == &a);
      CHECK(form.Item(1).get() == &fixture.Get<HTMLFieldSetElement>(u8"fs"));
      CHECK(form.Elements()->Length() == 3);
    }
  }

  TEST_CASE("Form owners: the form attribute", "[HTML][Forms][html:reset-the-form-owner][html:attr-fae-form]")
  {
    FormFixture fixture(
      u8R"(<form id=f></form><form id=g><input id=a form=f><button id=b form=nothing></button></form>
      <p id=p></p>)");
    auto &f = fixture.Get<HTMLFormElement>(u8"f");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &b = fixture.Get<HTMLButtonElement>(u8"b");
    CHECK(a.Form() == &f);
    // A form attribute that names no form leaves the control without an owner.
    CHECK(b.Form() == nullptr);
    CHECK(f.Length() == 1);

    SECTION("Giving another element the id doesn't make it the owner unless it's a form")
    {
      REQUIRE_FALSE(fixture.ById(u8"p").SetAttribute(u8"id", u8"nothing").HasException());
      CHECK(b.Form() == nullptr);
    }

    SECTION("The owner follows the id")
    {
      REQUIRE_FALSE(f.SetAttribute(u8"id", u8"nothing").HasException());
      CHECK(a.Form() == nullptr);
      CHECK(b.Form() == &f);
    }

    SECTION("Removing the attribute falls back to the ancestor")
    {
      a.RemoveAttribute(u8"form");
      CHECK(a.Form() == &fixture.Get<HTMLFormElement>(u8"g"));
    }
  }

  TEST_CASE("Form owners: a form attribute naming a shadow host reaches the form its reference target names",
            "[HTML][Forms][html:reset-the-form-owner][html:attr-fae-form][html:resolve-the-reference-target]")
  {
    FormFixture fixture(u8R"(<input id=before form=h><x-host id=h></x-host><input id=a form=h>
      <fieldset form=h><legend id=legend></legend></fieldset><select id=s form=h><option id=o></select>)");
    // A shadow root on the element with ID @p id, its reference target @p target, holding @p markup.
    auto attach = [&](dom::DOMStringView id, const char8_t *target, const char8_t *markup)
    {
      auto shadow = fixture.ById(id).AttachShadow({.ReferenceTarget = dom::DOMString(target)}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTMLUnsafe(markup).HasException());
      return shadow;
    };
    auto shadow = attach(u8"h", u8"f", u8"<form id=f></form>");
    auto &form = Downcast<HTMLFormElement>(*shadow->GetElementById(u8"f"));
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &before = fixture.Get<HTMLInputElement>(u8"before");
    CHECK(a.Form() == &form);
    CHECK(before.Form() == &form);

    SECTION("the form IDL attributes show the host, as the form is in its shadow tree")
    {
      auto &host = fixture.ById(u8"h");
      CHECK(a.FormForBindings().get() == &host);
      CHECK(fixture.Get<HTMLLegendElement>(u8"legend").FormForBindings().get() == &host);
      CHECK(fixture.Get<HTMLOptionElement>(u8"o").FormForBindings().get() == &host);

      // A form in the control's own tree is itself.
      shadow->SetReferenceTarget(null);
      REQUIRE_FALSE(host.SetAttribute(u8"id", u8"other").HasException());
      auto outerElement = fixture.Document->CreateElement(u8"form").ReleaseValue();
      auto &outer = Downcast<HTMLFormElement>(*outerElement);
      REQUIRE_FALSE(outer.SetAttribute(u8"id", u8"h").HasException());
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(outer).HasException());
      CHECK(a.FormForBindings().get() == &outer);
    }

    SECTION("in tree order, placed where the host is")
    {
      REQUIRE(form.Length() == 4);
      CHECK(form.Item(0).get() == &before);
      CHECK(form.Item(1).get() == &a);
      CHECK(form.Item(3).get() == &fixture.Get<HTMLSelectElement>(u8"s"));
    }

    SECTION("changing the reference target changes it")
    {
      shadow->SetReferenceTarget(null);
      CHECK(a.Form() == nullptr);
      shadow->SetReferenceTarget(dom::DOMString(u8"f"));
      CHECK(a.Form() == &form);
    }

    SECTION("so does the form gaining or losing that ID")
    {
      REQUIRE_FALSE(form.SetAttribute(u8"id", u8"g").HasException());
      CHECK(a.Form() == nullptr);
      REQUIRE_FALSE(form.SetAttribute(u8"id", u8"f").HasException());
      CHECK(a.Form() == &form);
    }

    SECTION("or leaving and joining the shadow tree")
    {
      Ref<HTMLFormElement> protectedForm = ShareRef(form);
      form.Remove();
      CHECK(a.Form() == nullptr);
      REQUIRE_FALSE(shadow->AppendChild(form).HasException());
      CHECK(a.Form() == &form);
    }

    SECTION("or the host losing its ID")
    {
      REQUIRE_FALSE(fixture.ById(u8"h").SetAttribute(u8"id", u8"other").HasException());
      CHECK(a.Form() == nullptr);
    }
  }
}
