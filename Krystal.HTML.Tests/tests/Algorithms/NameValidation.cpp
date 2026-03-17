#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("NameValidation::IsValidNamespacePrefix", "[HTML][NameValidation]")
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
      DOMString str = u8"foobar";
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

  TEST_CASE("NameValidation::IsValidAttributeLocalName", "[HTML][NameValidation]")
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
      DOMString str = u8"foobar";
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

  TEST_CASE("NameValidation::IsValidElementLocalName", "[HTML][NameValidation]")
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
        DOMString str = u8"foobar";
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
      DOMString str = u8"foobar";
      str.insert(str.begin() + 3uz, '\0');
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(str));
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo/bar"));
      REQUIRE_FALSE(NameValidation::IsValidElementLocalName(u8"foo>bar"));
    }
  }

  TEST_CASE("NameValidation::IsValidDocTypeName", "[HTML][NameValidation]")
  {
    SECTION("String with ASCII whitespace is invalid")
    {
      REQUIRE_FALSE(NameValidation::IsValidDocTypeName(u8"foo bar"));
    }

    SECTION("String with null character is invalid")
    {
      DOMString str = u8"foobar";
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

  TEST_CASE("NameValidation::ValidateAndExtract", "[HTML][NameValidation]")
  {
    SECTION("Contains ':' and has invalid prefix")
    {
      auto result =
        NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8":foo", u8"element");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }

    SECTION("With 'attribute' context")
    {
      SECTION("Qualified name with empty local name is invalid")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:", u8"attribute");
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Qualified name with invalid characters is invalid")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar=", u8"attribute");
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Valid qualified name")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar", u8"attribute");
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value().NamespaceURI.View() == u8"http://www.w3.org/1999/xhtml");
        REQUIRE(result.Value().Prefix.View() == u8"foo");
        REQUIRE(result.Value().LocalName.View() == u8"bar");
      }
    }

    SECTION("With 'element' context")
    {
      SECTION("Qualified name with empty local name is invalid")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:", u8"element");
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Qualified name with invalid characters is invalid")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar>", u8"element");
        REQUIRE(result.HasException());
        REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }

      SECTION("Valid qualified name")
      {
        auto result =
          NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"foo:bar", u8"element");
        REQUIRE_FALSE(result.HasException());
        REQUIRE(result.Value().NamespaceURI.View() == u8"http://www.w3.org/1999/xhtml");
        REQUIRE(result.Value().Prefix.View() == u8"foo");
        REQUIRE(result.Value().LocalName.View() == u8"bar");
      }
    }

    SECTION("prefix with missing namespace is NamespaceError")
    {
      auto result = NameValidation::ValidateAndExtract(u8"", u8"foo:bar", u8"element");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xml prefix with non-xml namespace is an error")
    {
      auto result =
        NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"xml:foo", u8"element");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xml prefix with xml namespace is valid")
    {
      auto result = NameValidation::ValidateAndExtract(u8"http://www.w3.org/XML/1998/namespace", u8"xml:foo",
                                                       u8"element");
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespaceURI.View() == u8"http://www.w3.org/XML/1998/namespace");
      REQUIRE(result.Value().Prefix.View() == u8"xml");
      REQUIRE(result.Value().LocalName.View() == u8"foo");
    }

    SECTION("xmlns prefix with non xmlns namespace is an error")
    {
      auto result =
        NameValidation::ValidateAndExtract(u8"http://www.w3.org/1999/xhtml", u8"xmlns:foo", u8"element");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::NamespaceError);
    }

    SECTION("xmlns prefix with xmlns namespace is valid")
    {
      auto result =
        NameValidation::ValidateAndExtract(u8"http://www.w3.org/2000/xmlns/", u8"xmlns:foo", u8"element");
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value().NamespaceURI.View() == u8"http://www.w3.org/2000/xmlns/");
      REQUIRE(result.Value().Prefix.View() == u8"xmlns");
      REQUIRE(result.Value().LocalName.View() == u8"foo");
    }
  }
}