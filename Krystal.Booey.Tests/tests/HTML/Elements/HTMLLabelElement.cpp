#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOutputElement.hpp"

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

  TEST_CASE("HTMLLabelElement: the labeled control goes through a shadow host's reference target",
            "[HTML][Forms][html:labeled-control][html:attr-label-for][html:resolve-the-reference-target]")
  {
    FormFixture fixture(u8R"(<label id=l1 for=h1>One</label><x-host id=h1></x-host>
      <label id=l2>Two <x-host id=h2></x-host></label>)");
    // A shadow root on the element with ID @p id, its reference target @p target, holding @p markup.
    auto attach = [&](dom::DOMStringView id, const char8_t *target, const char8_t *markup)
    {
      auto shadow = fixture.ById(id).AttachShadow({.ReferenceTarget = dom::DOMString(target)}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTMLUnsafe(markup).HasException());
      return shadow;
    };
    auto &l1 = fixture.Get<HTMLLabelElement>(u8"l1");
    auto &l2 = fixture.Get<HTMLLabelElement>(u8"l2");
    auto shadow1 = attach(u8"h1", u8"i", u8"<form id=f><input id=i></form>");
    auto shadow2 = attach(u8"h2", u8"j", u8"<input id=j>");

    // Through the for attribute, and through a descendant.
    CHECK(l1.Control().get() == shadow1->GetElementById(u8"i").get());
    CHECK(l2.Control().get() == shadow2->GetElementById(u8"j").get());
    CHECK(l1.Form() == shadow1->GetElementById(u8"f").get());

    // The IDL attributes show the hosts, as the control and its form are in their shadow trees.
    CHECK(l1.ControlForBindings().get() == &fixture.ById(u8"h1"));
    CHECK(l1.FormForBindings().get() == &fixture.ById(u8"h1"));
    CHECK(l2.ControlForBindings().get() == &fixture.ById(u8"h2"));
    CHECK_FALSE(l2.FormForBindings());

    // A host whose reference target names nothing isn't a control, nor labelable itself.
    shadow1->SetReferenceTarget(dom::DOMString(u8"missing"));
    CHECK_FALSE(l1.Control());
    shadow2->SetReferenceTarget(null);
    CHECK_FALSE(l2.Control());
  }
}
