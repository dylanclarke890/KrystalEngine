#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::tests
{
  using html::HTMLOptionElement;
  using html::HTMLSelectElement;

  namespace
  {
    KRYS_NODISCARD HTMLOptionElement &OptionById(EditorFixture &fixture, utf8_stringview id) noexcept
    {
      return Downcast<HTMLOptionElement>(fixture.ById(id));
    }
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "HTMLOptionElement: text, value and label",
    "[HTML][HTMLOptionElement][html:get-html-aware-text-content][html:dom-option-text]"
    "[html:dom-option-value][html:attr-option-value][html:dom-option-label][html:concept-option-label]")
  {
    EditorFixture fixture(
      u8"<select id=s><option id=a label=Label>  Some <b>bold</b>  text <script>x</script></option>"
      u8"<option id=b>B</option><option id=e label=''>Empty</option></select>");

    auto &a = OptionById(fixture, u8"a");
    CHECK(a.Text() == u8"Some bold text");
    CHECK(a.Label() == u8"Label");
    CHECK(a.Value() == u8"Some bold text");
    CHECK(OptionById(fixture, u8"b").Label() == u8"B");

    // An empty label attribute is the label IDL attribute, but the option shows its text.
    auto &e = OptionById(fixture, u8"e");
    CHECK(e.Label().empty());
    CHECK(e.TextIndentedToRespectGroupLabel() == u8"Empty");

    // Setting the text keeps a menu list's selection.
    auto &select = Downcast<HTMLSelectElement>(fixture.ById(u8"s"));
    select.SelectedIndex(1);
    OptionById(fixture, u8"b").Text(u8"Bee");
    CHECK(select.SelectedIndex() == 1);
    CHECK(select.Value() == u8"Bee");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLOptionElement: Option() sets the selectedness without making the option dirty",
                   "[HTML][HTMLOptionElement][html:dom-option]")
  {
    EditorFixture fixture(u8"<p></p>");
    auto created = HTMLOptionElement::CreateForLegacyFactoryFunction(*fixture.Document, u8"New",
                                                                     dom::DOMString(u8"n"), true, false);
    REQUIRE_FALSE(created.HasException());
    auto option = created.ReleaseValue();
    CHECK(option->LocalName() == u8"option");
    CHECK(option->Text() == u8"New");
    CHECK(option->Value() == u8"n");
    CHECK(option->DefaultSelected());
    CHECK_FALSE(option->Selected());

    // Still clean, so its selectedness follows its selected attribute.
    option->DefaultSelected(false);
    option->DefaultSelected(true);
    CHECK(option->Selected());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLOptionElement: changing disabled resets its select's selectedness",
                   "[HTML][HTMLOptionElement][html:concept-option-disabled][html:attr-option-disabled]")
  {
    EditorFixture fixture(u8"<select id=s><option id=a disabled>A</option></select>"
                          u8"<select><optgroup disabled><datalist><option id=d>D</option></datalist>"
                          u8"<div><option id=v>V</option></div></optgroup></select>");
    auto &select = Downcast<HTMLSelectElement>(fixture.ById(u8"s"));
    CHECK(select.SelectedIndex() == -1);
    OptionById(fixture, u8"a").Disabled(false);
    CHECK(select.SelectedIndex() == 0);

    // The nearest optgroup, select, hr, datalist or option ancestor decides.
    CHECK_FALSE(OptionById(fixture, u8"d").IsDisabledFormControl());
    CHECK(OptionById(fixture, u8"v").IsDisabledFormControl());
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "HTMLOptionElement: index, form and the nearest ancestor select",
    "[HTML][HTMLOptionElement][html:concept-option-index][html:dom-option-index][html:dom-option-form]"
    "[html:option-element-nearest-ancestor-select][html:update-an-option's-nearest-ancestor-select]")
  {
    EditorFixture fixture(
      u8"<form id=f><select><option>A</option><optgroup><option id=b>B</option></optgroup>"
      u8"</select></form><div><option id=o>O</option></div>");
    auto &b = OptionById(fixture, u8"b");
    CHECK(b.Index() == 1);
    CHECK(b.Form() == &fixture.ById(u8"f"));
    auto &o = OptionById(fixture, u8"o");
    CHECK(o.Index() == 0);
    CHECK_FALSE(o.Form());

    // Moving it out of the select leaves its list of options.
    Ref<HTMLOptionElement> keep = ShareRef(b);
    REQUIRE_FALSE(fixture.ById(u8"o").ParentNode()->AppendChild(b).HasException());
    CHECK_FALSE(b.OwnerSelectElement());
    CHECK(b.Index() == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLOptionElement: options and optgroups in a disabled select are actually disabled",
                   "[HTML][HTMLOptionElement][html:concept-element-disabled]")
  {
    EditorFixture fixture(u8"<select id=s disabled><optgroup id=g><option id=o>A</option></optgroup>"
                          u8"<option id=p>B</option></select>"
                          u8"<select id=t><optgroup id=h disabled><option id=q>C</option></optgroup>"
                          u8"<option id=r>D</option></select>");
    auto matches = [&](utf8_stringview id, utf8_stringview selector)
    {
      return fixture.ById(id).Matches(dom::DOMString(selector)).ReleaseValue();
    };

    for (auto id : {u8"s", u8"g", u8"o", u8"p", u8"h", u8"q"})
    {
      CHECK(matches(id, u8":disabled"));
      CHECK_FALSE(matches(id, u8":enabled"));
    }
    CHECK(matches(u8"r", u8":enabled"));

    // Enabling the select enables them, though the option is still disabled for selection by its optgroup.
    fixture.ById(u8"s").RemoveAttribute(u8"disabled");
    CHECK(matches(u8"o", u8":enabled"));
    CHECK(matches(u8"g", u8":enabled"));
    REQUIRE_FALSE(fixture.ById(u8"t").SetAttribute(u8"disabled", u8"").HasException());
    CHECK(matches(u8"r", u8":disabled"));
    CHECK_FALSE(OptionById(fixture, u8"r").IsDisabledFormControl());
  }

}
