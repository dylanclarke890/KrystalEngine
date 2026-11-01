#include "Krystal.Booey/HTML/HTMLLabelElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  TEST_CASE("HTMLLabelElement: the labeled control, its form owner and its labels",
            "[HTML][Forms][html:dom-label-control][html:dom-label-form][html:dom-lfe-labels]"
            "[html:labeled-control][html:attr-label-for]")
  {
    FormFixture fixture(u8R"(<form id=f><label id=l1>Name <input id=a></label><label id=l2 for=b>B</label>
      <output id=b></output></form>)");
    auto &a = fixture.Get<HTMLInputElement>(u8"a");
    auto &l1 = fixture.Get<HTMLLabelElement>(u8"l1");
    auto &l2 = fixture.Get<HTMLLabelElement>(u8"l2");
    CHECK(l1.Control().get() == &a);
    CHECK(l2.Control().get() == &fixture.Get<HTMLOutputElement>(u8"b"));
    CHECK(l1.Form() == &fixture.Get<HTMLFormElement>(u8"f"));
    auto labels = a.Labels();
    REQUIRE(labels);
    CHECK(labels->Length() == 1);
    CHECK(a.Labels().get() == labels.get());

    // A label whose for names nothing labelable has no control, so no form.
    REQUIRE_FALSE(l2.SetAttribute(u8"for", u8"f").HasException());
    CHECK_FALSE(l2.Control());
    CHECK_FALSE(l2.Form());

    SECTION("Hidden inputs aren't labelable")
    {
      REQUIRE_FALSE(a.SetAttribute(u8"type", u8"hidden").HasException());
      CHECK_FALSE(a.Labels());
    }
  }
}
