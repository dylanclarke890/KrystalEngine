#include "Krystal.Booey/HTML/InputTypes/NumberInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

namespace krys::boo::tests
{
  TEST_CASE("NumberInputType: numbers in the element's locale", "[HTML][Forms][Input]")
  {
    // Form controls follow their lang, so the test doesn't depend on the system's language.
    EditorFixture fixture(u8"");
    auto settings = Settings::Create();
    settings->LangAttributeAwareFormControlUIEnabled = true;
    fixture.Document->SetSettings(krys::move(settings));
    REQUIRE_FALSE(
      fixture.Body()
        .InnerHTML(u8"<input type=number id=de lang=de value=1.5><input type=number id=en lang=en "
                   u8"value=1.5><input type=number id=exp lang=de value=1e3>")
        .HasException());
    (void)fixture.Document->UpdateLayout();
    auto &german = Downcast<html::HTMLInputElement>(fixture.ById(u8"de"));

    // The field shows the value with the locale's decimal separator; the value stays an HTML number.
    CHECK(german.InnerTextValue() == u8"1,5");
    CHECK(german.Value() == u8"1.5");
    CHECK(Downcast<html::HTMLInputElement>(fixture.ById(u8"en")).InnerTextValue() == u8"1.5");
    // Scientific notation isn't localised.
    CHECK(Downcast<html::HTMLInputElement>(fixture.ById(u8"exp")).InnerTextValue() == u8"1e3");

    // What the user types in the locale's way is read back as a number.
    german.Focus();
    german.Select();
    CHECK(fixture.GetEditor().InsertText(u8"2,25", nullptr));
    CHECK(german.InnerTextValue() == u8"2,25");
    CHECK(german.Value() == u8"2.25");
    CHECK(german.ValueAsNumber() == 2.25);

    // A second decimal separator, in either form, is refused.
    CHECK(fixture.GetEditor().InsertText(u8",", nullptr));
    CHECK(fixture.GetEditor().InsertText(u8".", nullptr));
    CHECK(german.InnerTextValue() == u8"2,25");
  }
}
