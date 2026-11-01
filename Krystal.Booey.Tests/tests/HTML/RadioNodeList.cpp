#include "Krystal.Booey/HTML/RadioNodeList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  namespace
  {
    KRYS_NODISCARD Ref<RadioNodeList> Radios(FormFixture &fixture, dom::DOMStringView name) noexcept
    {
      auto item = fixture.Get<HTMLFormElement>(u8"f").Elements()->NamedItemOrItems(name);
      REQUIRE(item);
      REQUIRE(std::holds_alternative<RefPtr<RadioNodeList>>(*item));
      return ShareRef(*std::get<RefPtr<RadioNodeList>>(*item));
    }
  }

  TEST_CASE("HTMLFormControlsCollection::NamedItemOrItems: one element, or a RadioNodeList of several",
            "[HTML][Forms][html:dom-htmlformcontrolscollection-nameditem]")
  {
    FormFixture fixture(u8"<form id=f><input id=a name=one><input id=b name=two><input id=two>"
                        u8"<input type=image name=img></form>");
    auto collection = fixture.Get<HTMLFormElement>(u8"f").Elements();
    CHECK_FALSE(collection->NamedItemOrItems(u8""));
    CHECK_FALSE(collection->NamedItemOrItems(u8"none"));
    // Image buttons aren't in the collection.
    CHECK_FALSE(collection->NamedItemOrItems(u8"img"));

    auto one = collection->NamedItemOrItems(u8"one");
    REQUIRE(one);
    REQUIRE(std::holds_alternative<RefPtr<dom::Element>>(*one));
    CHECK(std::get<RefPtr<dom::Element>>(*one).get() == &fixture.ById(u8"a"));

    // An ID and a name both match, in tree order.
    auto list = Radios(fixture, u8"two");
    REQUIRE(list->Length() == 2uz);
    CHECK(list->Item(0).get() == &fixture.ById(u8"b"));
    CHECK(list->Item(1).get() == &fixture.ById(u8"two"));
  }

  TEST_CASE("RadioNodeList::Value: the first checked radio button's value, \"on\" without one",
            "[HTML][Forms][html:dom-radionodelist-value]")
  {
    FormFixture fixture(u8"<form id=f><input type=radio name=r id=empty value=''>"
                        u8"<input type=radio name=r id=plain><input type=radio name=r id=x value=x>"
                        u8"<input type=text name=r value=text></form>");
    auto list = Radios(fixture, u8"r");
    CHECK(list->Value() == u8"");

    fixture.Get<HTMLInputElement>(u8"x").SetChecked(true);
    CHECK(list->Value() == u8"x");
    fixture.Get<HTMLInputElement>(u8"plain").SetChecked(true);
    CHECK(list->Value() == u8"on");
    // A checked radio button with an empty value gives the empty string.
    fixture.Get<HTMLInputElement>(u8"empty").SetChecked(true);
    CHECK(list->Value() == u8"");

    // Setting checks the first radio button with the value, or without one for "on".
    list->SetValue(u8"x");
    CHECK(fixture.Get<HTMLInputElement>(u8"x").Checked());
    list->SetValue(u8"on");
    CHECK(fixture.Get<HTMLInputElement>(u8"plain").Checked());
    list->SetValue(u8"");
    CHECK(fixture.Get<HTMLInputElement>(u8"empty").Checked());
    list->SetValue(u8"text");
    CHECK(fixture.Get<HTMLInputElement>(u8"empty").Checked());
  }
}
