#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLOptGroupElement: changing disabled resets its select's selectedness",
                   "[HTML][HTMLOptGroupElement][html:attr-optgroup-disabled][html:optgroup-option-group]")
  {
    EditorFixture fixture(
      u8"<select id=s><optgroup id=g disabled><option id=a>A</option></optgroup></select>");
    auto &select = Downcast<html::HTMLSelectElement>(fixture.ById(u8"s"));
    auto &group = Downcast<html::HTMLOptGroupElement>(fixture.ById(u8"g"));
    CHECK(Downcast<html::HTMLOptionElement>(fixture.ById(u8"a")).IsDisabledFormControl());
    CHECK(select.SelectedIndex() == -1);
    group.Disabled(false);
    CHECK(select.SelectedIndex() == 0);
  }
}
