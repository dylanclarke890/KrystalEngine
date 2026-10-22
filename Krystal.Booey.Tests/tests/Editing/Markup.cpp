#include "Krystal.Booey/Editing/Markup.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "Markup: serialising fragments", "[Editing][Markup]")
  {
    EditorFixture fixture(
      u8"<div id=a title='x\"y'>a &amp; b &lt;c&gt;<b>bold</b><br><!--note--><img src=i.png>"
      u8"<span> </span></div><template id=t><i>t</i></template>");

    SECTION("Children, with attributes and text escaped")
    {
      CHECK(Narrow(fixture.InnerHTML(fixture.ById(u8"a")))
            == "a &amp; b &lt;c&gt;<b>bold</b><br><!--note--><img src=\"i.png\"><span>&nbsp;</span>");
    }

    SECTION("The node itself")
    {
      auto markup = SerializeFragment(fixture.ById(u8"a"), SerializedNodes::SubtreeIncludingNode);
      CHECK(Narrow(markup).starts_with("<div id=\"a\" title=\"x&quot;y\">"));
      CHECK(Narrow(markup).ends_with("</div>"));
    }

    SECTION("A template's contents")
    {
      CHECK(Narrow(fixture.InnerHTML(fixture.ById(u8"t"))) == "<i>t</i>");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Markup: fragments from markup and text", "[Editing][Markup]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div><pre id=pre contenteditable>xyz</pre>");

    SECTION("Markup is parsed as if in a body")
    {
      auto fragment = CreateFragmentFromMarkup(*fixture.Document, u8"<b>x</b>y<p>z", u8"");
      CHECK(Narrow(fixture.InnerHTML(*fragment)) == "<b>x</b>y<p>z</p>");
    }

    SECTION("Text without line breaks is a text node")
    {
      auto &text = fixture.TextIn(u8"e");
      auto fragment = CreateFragmentFromText(fixture.CollapsedRange(text, 1), u8"a < b");
      CHECK(Narrow(fixture.InnerHTML(*fragment)) == "a &lt; b");
    }

    SECTION("Each line of text is a paragraph")
    {
      auto &text = fixture.TextIn(u8"e");
      auto fragment = CreateFragmentFromText(fixture.CollapsedRange(text, 1), u8"one\ntwo\n\nthree");
      CHECK(Narrow(fixture.InnerHTML(*fragment))
            == "<div>one</div><div>two</div><div><br></div><div>three</div>");
    }

    SECTION("Text pasted into preformatted text keeps its line breaks")
    {
      auto &text = fixture.TextIn(u8"pre");
      auto fragment = CreateFragmentFromText(fixture.CollapsedRange(text, 1), u8"one\ntwo");
      CHECK(Narrow(fixture.InnerHTML(*fragment)) == "one\ntwo");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Markup: serialising a selection", "[Editing][Markup]")
  {
    EditorFixture fixture(u8"<p id=a>The <b id=b>quick</b> fox</p>");
    auto &bold = Downcast<dom::Text>(*fixture.ById(u8"b").FirstChild());

    fixture.Select(bold, 1, 5);
    auto markup = Narrow(SerializePreservingVisualAppearance(fixture.Selection().Selection()));

    // The text keeps its bold weight and font, however it's pasted.
    CHECK(markup.find("uick") != std::string::npos);
    CHECK(markup.find("<b") != std::string::npos);
    CHECK(markup.find("The") == std::string::npos);
  }
}
