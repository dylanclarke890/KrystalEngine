#include "Krystal.Booey/XML/PseudoAttributes.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::xml::tests
{
  namespace
  {
    /// @brief The pseudo-attributes as `name=value` strings, or "error".
    List<dom::DOMString> Parsed(dom::DOMStringView string)
    {
      auto result = ParsePseudoAttributes(string);
      if (!result)
      {
        return {u8"error"};
      }
      List<dom::DOMString> parsed;
      for (auto &attribute : *result)
      {
        parsed.push_back(attribute.Name + u8"=" + attribute.Value);
      }
      return parsed;
    }
  }

  TEST_CASE("Rules for parsing pseudo-attributes: names and quoted values, in order",
            "[XML][PseudoAttributes]")
  {
    CHECK(Parsed(u8"href=\"a.css\" type='text/css' x:y=\"z\"")
          == List<dom::DOMString> {u8"href=a.css", u8"type=text/css", u8"x:y=z"});
    CHECK(Parsed(u8"").empty());
    CHECK(Parsed(u8" \t\r\n").empty());
    // Spaces may surround the string and the equals sign.
    CHECK(Parsed(u8"  a = 'b'\n") == List<dom::DOMString> {u8"a=b"});
    // The other quote, spaces and > are plain characters in a value.
    CHECK(Parsed(u8"a=\"it's > 1 \"") == List<dom::DOMString> {u8"a=it's > 1 "});
    CHECK(Parsed(u8"a='say \"hi\"'") == List<dom::DOMString> {u8"a=say \"hi\""});
    CHECK(Parsed(u8"a=''") == List<dom::DOMString> {u8"a="});
    CHECK(Parsed(u8"é=\"é\"") == List<dom::DOMString> {u8"é=é"});
  }

  TEST_CASE("Rules for parsing pseudo-attributes: references are replaced by their characters",
            "[XML][PseudoAttributes]")
  {
    CHECK(Parsed(u8"a=\"&amp;&lt;&gt;&quot;&apos;\"") == List<dom::DOMString> {u8"a=&<>\"'"});
    CHECK(Parsed(u8"a=\"&#38;&#x41;&#xe9;&#x1F600;\"") == List<dom::DOMString> {u8"a=&Aé\U0001F600"});

    // Other entities, unterminated references and characters XML doesn't allow are errors.
    CHECK(Parsed(u8"a=\"&nbsp;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&amp\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#x;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#0;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#xD800;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#x110000;\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"&#99999999999999999999;\"") == List<dom::DOMString> {u8"error"});
  }

  TEST_CASE("Rules for parsing pseudo-attributes: anything that doesn't match PseudoAtts is an error",
            "[XML][PseudoAttributes]")
  {
    for (auto string : {u8"href=unquoted", u8"a", u8"a=", u8"=\"b\"", u8"a=\"b", u8"a=\"<\"",
                        u8"a=\"b\"c=\"d\"", u8"1a=\"b\"", u8"a b=\"c\"", u8"a=\"b\" junk"})
    {
      INFO(reinterpret_cast<const char *>(string));
      CHECK(Parsed(string) == List<dom::DOMString> {u8"error"});
    }
  }

  TEST_CASE("Rules for parsing pseudo-attributes: a name twice is an error", "[XML][PseudoAttributes]")
  {
    CHECK(Parsed(u8"a=\"1\" a=\"2\"") == List<dom::DOMString> {u8"error"});
    CHECK(Parsed(u8"a=\"1\" A=\"2\"") == List<dom::DOMString> {u8"a=1", u8"A=2"});
  }
}
