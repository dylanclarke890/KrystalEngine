#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

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

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDataListElement: suggestions",
                   "[HTML][HTMLDataListElement][html:concept-input-list]")
  {
    // The suggestions are the options that aren't disabled and have a value.
    EditorFixture fixture(u8"<datalist id=d><option value=a></option><option disabled value=b></option>"
                          u8"<option></option><optgroup disabled><option value=c></option></optgroup>"
                          u8"<div><option>d</option></div></datalist>");
    auto suggestions = Downcast<html::HTMLDataListElement>(fixture.ById(u8"d")).Suggestions();
    REQUIRE(suggestions.size() == 2uz);
    CHECK(suggestions[0]->Value() == u8"a");
    CHECK(suggestions[1]->Value() == u8"d");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLInputElement: list",
                   "[HTML][HTMLInputElement][html:dom-input-list][html:attr-input-list]")
  {
    EditorFixture fixture(u8"<datalist id=d></datalist><p id=p></p><input id=i list=d>");
    auto &input = Downcast<html::HTMLInputElement>(fixture.ById(u8"i"));
    auto &datalist = fixture.ById(u8"d");
    CHECK(input.List().get() == &datalist);

    SECTION("The attribute applies to the types with suggestions")
    {
      for (auto type : {u8"search", u8"url", u8"tel", u8"email", u8"number", u8"range", u8"color", u8"date",
                        u8"month", u8"week", u8"time", u8"datetime-local"})
      {
        REQUIRE_FALSE(input.SetAttribute(u8"type", type).HasException());
        CHECK(input.List().get() == &datalist);
      }
      for (auto type : {u8"password", u8"hidden", u8"checkbox", u8"radio", u8"file", u8"submit", u8"image",
                        u8"reset", u8"button"})
      {
        REQUIRE_FALSE(input.SetAttribute(u8"type", type).HasException());
        CHECK_FALSE(input.List());
      }
    }

    SECTION("It is the first element with the ID, if that is a datalist")
    {
      REQUIRE_FALSE(input.SetAttribute(u8"list", u8"p").HasException());
      CHECK_FALSE(input.List());
      REQUIRE_FALSE(input.SetAttribute(u8"list", u8"").HasException());
      CHECK_FALSE(input.List());
      REQUIRE_FALSE(input.SetAttribute(u8"list", u8"d").HasException());
      REQUIRE_FALSE(datalist.Remove().HasException());
      CHECK_FALSE(input.List());
    }
  }
}
