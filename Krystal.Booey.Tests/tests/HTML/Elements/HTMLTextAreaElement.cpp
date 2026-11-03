#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"

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

  TEST_CASE("HTMLTextAreaElement: cols and rows are positive, with fallbacks of 20 and 2",
            "[HTML][Forms][HTMLTextAreaElement][html:attr-textarea-cols][html:attr-textarea-rows]"
            "[html:attr-textarea-cols-value][html:attr-textarea-rows-value]")
  {
    FormFixture fixture(u8"<textarea id=t cols=0 rows=x></textarea>");
    auto &textarea = fixture.Get<HTMLTextAreaElement>(u8"t");
    CHECK(textarea.Cols() == 20u);
    CHECK(textarea.Rows() == 2u);
    textarea.Cols(7u);
    CHECK(textarea.Cols() == 7u);
    textarea.Rows(0u);
    CHECK(textarea.Rows() == 2u);
    CHECK(Str(textarea.FormControlType()) == "textarea");
  }

  TEST_CASE(
    "HTMLTextAreaElement: the raw value follows the child text content until it's dirty",
    "[HTML][Forms][HTMLTextAreaElement][html:concept-textarea-raw-value][html:dom-textarea-defaultvalue]"
    "[html:dom-textarea-value][html:dom-textarea-textlength][html:the-textarea-element]")
  {
    FormFixture fixture(u8"<form id=f><textarea id=t>a\r\nb</textarea></form>");
    auto &textarea = fixture.Get<HTMLTextAreaElement>(u8"t");
    // The parser normalizes the newlines.
    CHECK(Str(textarea.Value()) == "a\nb");
    CHECK(textarea.TextLength() == 3u);

    // The default value is the child text content, not the descendant text content.
    auto span = fixture.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(span->AppendChild(*CreateRef<dom::Text>(*fixture.Document, u8"x")).HasException());
    REQUIRE_FALSE(textarea.AppendChild(*span).HasException());
    CHECK(Str(textarea.DefaultValue()) == "a\nb");

    textarea.DefaultValue(u8"c");
    CHECK(Str(textarea.Value()) == "c");

    // The API value has its newlines normalized.
    REQUIRE_FALSE(textarea.SetValue(u8"x\r\ny\rz").HasException());
    CHECK(Str(textarea.Value()) == "x\ny\nz");

    // The text length counts UTF-16 code units.
    REQUIRE_FALSE(textarea.SetValue(u8"\U0001F600").HasException());
    CHECK(textarea.TextLength() == 2u);
    textarea.DefaultValue(u8"d");
    CHECK(Str(textarea.Value()) == "\U0001F600");

    // A clone takes the raw value and the dirty value flag.
    auto clone = textarea.CloneNode(true).ReleaseValue();
    CHECK(Str(Downcast<HTMLTextAreaElement>(*clone).Value()) == "\U0001F600");
    Downcast<HTMLTextAreaElement>(*clone).DefaultValue(u8"e");
    CHECK(Str(Downcast<HTMLTextAreaElement>(*clone).Value()) == "\U0001F600");

    // Resetting it cleans it.
    fixture.Get<HTMLFormElement>(u8"f").Reset();
    CHECK(Str(textarea.Value()) == "d");
  }

  TEST_CASE(
    "HTMLTextAreaElement: wrap is only soft or hard",
    "[HTML][Forms][HTMLTextAreaElement][html:attr-textarea-wrap][html:textarea-wrapping-transformation]")
  {
    FormFixture fixture(u8"<form id=f action=https://example.com/s>"
                        u8"<textarea id=t name=t cols=3 wrap=hard style='font: 10px monospace'>aaaa aaaa aaaa"
                        u8"</textarea></form>");
    auto &form = fixture.Get<HTMLFormElement>(u8"f");
    auto &textarea = fixture.Get<HTMLTextAreaElement>(u8"t");
    form.Submit();
    REQUIRE(fixture.Submissions().size() == 1);
    CHECK(fixture.Submissions()[0].URL.find("%0D%0A") != std::string::npos);

    // The legacy aliases of hard are not keywords.
    textarea.Wrap(u8"physical");
    form.Submit();
    REQUIRE(fixture.Submissions().size() == 2);
    CHECK(fixture.Submissions()[1].URL == "https://example.com/s?t=aaaa+aaaa+aaaa");
  }

  TEST_CASE("HTMLTextAreaElement: a readonly textarea is barred from constraint validation and not missing",
            "[HTML][Forms][HTMLTextAreaElement][html:attr-textarea-readonly][html:attr-textarea-required]")
  {
    FormFixture fixture(u8"<textarea id=t required></textarea>");
    auto &textarea = fixture.Get<HTMLTextAreaElement>(u8"t");
    CHECK(textarea.ValueMissing());
    textarea.ReadOnly(true);
    CHECK_FALSE(textarea.WillValidate());
    CHECK_FALSE(textarea.ValueMissing());
  }
}
