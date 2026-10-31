#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    constexpr DOMStringView XHTML_NS = u8"http://www.w3.org/1999/xhtml";
  }

  TEST_CASE("Valid namespace prefix: non-empty, without ASCII whitespace, NULL, '/' or '>'",
            "[HTML][NameValidation][dom:valid-namespace-prefix]")
  {
    SECTION("Empty string is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidNamespacePrefix(u8""));
    }

    SECTION("String with ASCII whitespace is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidNamespacePrefix(u8"foo bar"));
    }

    SECTION("String with null character is invalid")
    {
      dom::DOMString str = u8"foobar";
      str.insert(str.begin() + 3uz, '\0');
      REQUIRE_FALSE(NameValidation::IsValidNamespacePrefix(str));
    }

    SECTION("String with '/' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidNamespacePrefix(u8"foo/bar"));
    }

    SECTION("String with '>' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidNamespacePrefix(u8"foo>bar"));
    }

    SECTION("Valid namespace prefix")
    {
      REQUIRE(NameValidation::IsValidNamespacePrefix(u8"foo"));
    }
  }

  TEST_CASE("Valid attribute local name: non-empty, without ASCII whitespace, NULL, '/', '=' or '>'",
            "[HTML][NameValidation][dom:valid-attribute-local-name]")
  {
    SECTION("Empty string is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(u8""));
    }

    SECTION("String with ASCII whitespace is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(u8"foo bar"));
    }

    SECTION("String with null character is invalid")
    {
      dom::DOMString str = u8"foobar";
      str.insert(str.begin() + 3uz, '\0');
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(str));
    }

    SECTION("String with '/' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(u8"foo/bar"));
    }

    SECTION("String with '=' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(u8"foo=bar"));
    }

    SECTION("String with '>' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidAttributeLocalName(u8"foo>bar"));
    }

    SECTION("Valid attribute local name")
    {
      REQUIRE(NameValidation::IsValidAttributeLocalName(u8"foo"));
    }
  }

  TEST_CASE("Valid element local name: an ASCII alpha start is lenient, others follow the name production",
            "[HTML][NameValidation][dom:valid-element-local-name]")
  {
    SECTION("Empty string is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8""));
    }

    SECTION("First character is ASCII Alpha")
    {
      SECTION("String with ASCII whitespace is invalid")
      {
        REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo bar"));
      }

      SECTION("String with null character is invalid")
      {
        dom::DOMString str = u8"foobar";
        str.insert(str.begin() + 3uz, '\0');
        REQUIRE_FALSE(NameValidation::IsValidElementLocalName(str));
      }

      SECTION("String with '/' is invalid")
      {
        REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo/bar"));
      }

      SECTION("String with '>' is invalid")
      {
        REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo>bar"));
      }

      SECTION("valid string")
      {
        REQUIRE(NameValidation::IsValidElementLocalName(u8"foo"));
      }
    }

    SECTION("Single character name starting with ':' is valid")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8":"));
    }

    SECTION("Single character name starting with '_' is valid")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8"_"));
    }

    SECTION("Single character name starting with non-ASCII character is valid")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8"é"));
    }

    SECTION("First character is not ASCII Alpha, ':', '_', or non-ASCII character")
    {
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"1foo"));
    }

    SECTION(
      "Subsequent characters can be ASCII Alpha, ASCII Digit, '-', '.', ':', '_', or non-ASCII characters")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8"foo-bar"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"foo.bar"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"foo:bar"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"foo_bar"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"fooébar"));
    }

    SECTION("Subsequent characters cannot be ASCII whitespace, null character, '/', or '>'")
    {
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo bar"));
      dom::DOMString str = u8"foobar";
      str.insert(str.begin() + 3uz, '\0');
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(str));
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo/bar"));
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo>bar"));
    }

    SECTION("Code points up to U+10FFFF are allowed, first and after")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8"\U0001F600"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"_\U0001F600"));
      REQUIRE(NameValidation::IsValidElementLocalName(u8"\U0010FFFFx"));
    }

    SECTION("Without an ASCII alpha start, other ASCII punctuation is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"-foo"));
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"_foo!"));
    }

    SECTION("With an ASCII alpha start, other ASCII punctuation is valid")
    {
      REQUIRE(NameValidation::IsValidElementLocalName(u8"foo!bar"));
    }
  }

  TEST_CASE("Valid doctype name: without ASCII whitespace, NULL or '>', and possibly empty",
            "[HTML][NameValidation][dom:valid-doctype-name]")
  {
    SECTION("String with ASCII whitespace is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidDocTypeName(u8"foo bar"));
    }

    SECTION("String with null character is invalid")
    {
      dom::DOMString str = u8"foobar";
      str.insert(str.begin() + 3uz, '\0');
      REQUIRE_FALSE(NameValidation::IsValidDocTypeName(str));
    }

    SECTION("String with '>' is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidDocTypeName(u8"foo>bar"));
    }

    SECTION("Valid doctype name")
    {
      REQUIRE(NameValidation::IsValidDocTypeName(u8"foobar"));
    }

    SECTION("Empty string is valid")
    {
      REQUIRE(NameValidation::IsValidDocTypeName(u8""));
    }
  }

  TEST_CASE("Validate and extract: splits a qualified name and checks it against its namespace",
            "[HTML][NameValidation][dom:validate-and-extract]")
  {
    SECTION("Contains ':' and has invalid prefix")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8":foo",
                                                       ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }

    SECTION("With 'attribute' context")
    {
      SECTION("Qualified name with empty local name is invalid")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:",
                                                         ValidateAndExtractContext::Attribute);
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Qualified name with invalid characters is invalid")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar=",
                                                         ValidateAndExtractContext::Attribute);
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Valid qualified name")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar",
                                                         ValidateAndExtractContext::Attribute);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value().NamespaceURI().View() == u8"http://www.w3.org/1999/xhtml");
        REQUIRE(result.Value().NamespacePrefix().View() == u8"foo");
        REQUIRE(result.Value().LocalName().View() == u8"bar");
      }
    }

    SECTION("With 'element' context")
    {
      SECTION("Qualified name with empty local name is invalid")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:",
                                                         ValidateAndExtractContext::Element);
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Qualified name with invalid characters is invalid")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar>",
                                                         ValidateAndExtractContext::Element);
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Valid qualified name")
      {
        auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar",
                                                         ValidateAndExtractContext::Element);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value().NamespaceURI().View() == u8"http://www.w3.org/1999/xhtml");
        REQUIRE(result.Value().NamespacePrefix().View() == u8"foo");
        REQUIRE(result.Value().LocalName().View() == u8"bar");
      }
    }

    SECTION("prefix with missing namespace is NamespaceError")
    {
      auto result = NameValidation::ValidateAndExtract(u8"", u8"foo:bar", ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xml prefix with non-xml namespace is an error")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"xml:foo",
                                                       ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xml prefix with xml namespace is valid")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/XML/1998/namespace", u8"xml:foo",
                                                       ValidateAndExtractContext::Element);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespaceURI().View() == u8"http://www.w3.org/XML/1998/namespace");
      REQUIRE(result.Value().NamespacePrefix().View() == u8"xml");
      REQUIRE(result.Value().LocalName().View() == u8"foo");
    }

    SECTION("xmlns prefix with non xmlns namespace is an error")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"xmlns:foo",
                                                       ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xmlns prefix with xmlns namespace is valid")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/2000/xmlns/", u8"xmlns:foo",
                                                       ValidateAndExtractContext::Element);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespaceURI().View() == u8"http://www.w3.org/2000/xmlns/");
      REQUIRE(result.Value().NamespacePrefix().View() == u8"xmlns");
      REQUIRE(result.Value().LocalName().View() == u8"foo");
    }

    SECTION("An empty namespace is null")
    {
      auto result = NameValidation::ValidateAndExtract(u8"", u8"foo", ValidateAndExtractContext::Element);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespaceURI() == DOMStringAtom::Null());
      REQUIRE(result.Value().NamespacePrefix() == DOMStringAtom::Null());
      REQUIRE(result.Value().LocalName().View() == u8"foo");
    }

    SECTION("Only the first ':' separates the prefix")
    {
      for (auto context : {ValidateAndExtractContext::Attribute, ValidateAndExtractContext::Element})
      {
        auto result = NameValidation::ValidateAndExtract(XHTML_NS, u8"a:b:c", context);
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value().NamespacePrefix().View() == u8"a");
        REQUIRE(result.Value().LocalName().View() == u8"b:c");
      }
    }

    SECTION("A qualified name of xmlns needs the XMLNS namespace")
    {
      auto result = NameValidation::ValidateAndExtract(u8"", u8"xmlns", ValidateAndExtractContext::Attribute);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);

      result = NameValidation::ValidateAndExtract(XHTML_NS, u8"xmlns", ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);

      result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/2000/xmlns/", u8"xmlns",
                                                  ValidateAndExtractContext::Attribute);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespacePrefix() == DOMStringAtom::Null());
      REQUIRE(result.Value().LocalName().View() == u8"xmlns");
    }

    SECTION("The XMLNS namespace needs a qualified name or prefix of xmlns")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/2000/xmlns/", u8"foo",
                                                       ValidateAndExtractContext::Attribute);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);

      result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/2000/xmlns/", u8"bar:xmlns",
                                                  ValidateAndExtractContext::Element);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }
  }

  TEST_CASE("NameValidation::IsValidCustomElementName", "[HTML][NameValidation]")
  {
    REQUIRE(NameValidation::IsValidCustomElementName(u8"x-widget"));
    REQUIRE(NameValidation::IsValidCustomElementName(u8"my-element-2"));
    REQUIRE_FALSE(NameValidation::IsValidCustomElementName(u8"div"));
    REQUIRE_FALSE(NameValidation::IsValidCustomElementName(u8"a"));
    REQUIRE_FALSE(NameValidation::IsValidCustomElementName(u8"X-widget"));
    REQUIRE_FALSE(NameValidation::IsValidCustomElementName(u8"x-Widget"));
    REQUIRE_FALSE(NameValidation::IsValidCustomElementName(u8"font-face"));
  }
}
