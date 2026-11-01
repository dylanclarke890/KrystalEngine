#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  namespace
  {
    KRYS_NODISCARD std::string Str(dom::DOMStringView text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }
  }

  TEST_CASE("HTMLOutputElement: the value and the default value override",
            "[HTML][Forms][HTMLOutputElement][html:concept-output-default-value][html:dom-output-value]"
            "[html:dom-output-defaultvalue][html:concept-output-default-value-override][html:dom-output-type]"
            "[html:the-output-element]")
  {
    FormFixture fixture(u8"<form id=f><output id=o>a<b>b</b></output></form>");
    auto &output = fixture.Get<HTMLOutputElement>(u8"o");
    CHECK(Str(output.Type()) == "output");
    CHECK(Str(output.Value()) == "ab");
    CHECK(Str(output.DefaultValue()) == "ab");

    // Without an override, setting the default value sets the content.
    output.DefaultValue(u8"c");
    CHECK(Str(output.Value()) == "c");

    // Setting the value keeps the default value as the override.
    output.Value(u8"d");
    CHECK(Str(output.Value()) == "d");
    CHECK(Str(output.DefaultValue()) == "c");
    output.DefaultValue(u8"e");
    CHECK(Str(output.Value()) == "d");

    // Resetting puts the default value back, and drops the override.
    fixture.Get<HTMLFormElement>(u8"f").Reset();
    CHECK(Str(output.Value()) == "e");
    output.DefaultValue(u8"f");
    CHECK(Str(output.Value()) == "f");
  }
}
