#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDataListElement: options are its option descendants",
                   "[HTML][HTMLDataListElement][html:dom-datalist-options][html:the-datalist-element]")
  {
    EditorFixture fixture(u8"<datalist id=d><option>A</option><div><option>B</option></div>"
                          u8"<input id=i required></datalist>");
    auto &datalist = Downcast<html::HTMLDataListElement>(fixture.ById(u8"d"));
    auto options = datalist.Options();
    CHECK(options->Length() == 2uz);
    CHECK(datalist.Options().get() == options.get());

    // Its descendants are barred from constraint validation.
    CHECK_FALSE(Downcast<html::HTMLInputElement>(fixture.ById(u8"i")).WillValidate());
  }
}
