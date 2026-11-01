#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  TEST_CASE("Form owners: the nearest ancestor form", "[HTML][Forms]")
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

  TEST_CASE("Form owners: the form attribute", "[HTML][Forms]")
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
}
