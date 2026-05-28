#include "Krystal.HTML/DOMTokenList.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<Element> Element;
      DOMTokenList TokenList;

      CommonTestData()
          : Document(CreateRef<HTML::Document>()), Element(Document->CreateElement(u8"div").Value()),
            TokenList(*Element, u8"test-tokens")
      {
      }
    };
  }

  TEST_CASE("DOMTokenList::Length", "[HTML][DOMTokenList]")
  {
    CommonTestData data;
    REQUIRE(data.TokenList.Length() == 0uz);

    REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
    REQUIRE(data.TokenList.Length() == 1uz);

    REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
    REQUIRE(data.TokenList.Length() == 2uz);

    REQUIRE_FALSE(data.TokenList.Remove(u8"bar").HasException());
    REQUIRE(data.TokenList.Length() == 1uz);

    REQUIRE_FALSE(data.TokenList.Remove(u8"foo").HasException());
    REQUIRE(data.TokenList.Length() == 0uz);
  }

  TEST_CASE("DOMTokenList::Item", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    REQUIRE_FALSE(data.TokenList.Item(0).has_value());
    REQUIRE_FALSE(data.TokenList.Item(1).has_value());

    REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
    REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());

    REQUIRE(data.TokenList.Item(0) == u8"foo");
    REQUIRE(data.TokenList.Item(1) == u8"bar");

    REQUIRE_FALSE(data.TokenList.Item(2).has_value());
  }

  TEST_CASE("DOMTokenList::operator[size_t index]", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    REQUIRE_FALSE(data.TokenList[0].has_value());
    REQUIRE_FALSE(data.TokenList[1].has_value());

    REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
    REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());

    REQUIRE(data.TokenList[0] == u8"foo");
    REQUIRE(data.TokenList[1] == u8"bar");

    REQUIRE_FALSE(data.TokenList.Item(2).has_value());
  }

  TEST_CASE("DOMTokenList::Contains", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    REQUIRE_FALSE(data.TokenList.Contains(u8"foo"));
    REQUIRE_FALSE(data.TokenList.Contains(u8"bar"));

    REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
    REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());

    REQUIRE(data.TokenList.Contains(u8"foo"));
    REQUIRE(data.TokenList.Contains(u8"bar"));

    REQUIRE_FALSE(data.TokenList.Contains(u8"baz"));
  }

  TEST_CASE("DOMTokenList::Add", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    SECTION("Happy path")
    {
      REQUIRE(data.TokenList.Length() == 0uz);
      REQUIRE(data.TokenList.Item(0).has_value() == false);

      REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 1uz);
      REQUIRE(data.TokenList.Contains(u8"foo"));
      REQUIRE(data.TokenList.Item(0) == u8"foo");
      REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"foo");

      REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 1uz);

      REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
      REQUIRE(data.TokenList.Length() == 2uz);

      REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"foo bar");
    }

    SECTION("Invalid tokens")
    {
      REQUIRE(data.TokenList.Length() == 0uz);

      auto result = data.TokenList.Add(u8"foo bar");
      REQUIRE(
        (result.HasException() && result.GetException().Code() == ExceptionCode::InvalidCharacterError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Add(u8"");
      REQUIRE((result.HasException() && result.GetException().Code() == ExceptionCode::SyntaxError));
      REQUIRE(data.TokenList.Length() == 0uz);
    }
  }

  TEST_CASE("DOMTokenList::Remove", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    SECTION("Happy path")
    {
      REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 1uz);

      REQUIRE_FALSE(data.TokenList.Remove(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 0uz);
      REQUIRE_FALSE(data.TokenList.Contains(u8"foo"));
      REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"");

      REQUIRE_FALSE(data.TokenList.Remove(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 0uz);
    }

    SECTION("Invalid tokens")
    {
      REQUIRE(data.TokenList.Length() == 0uz);

      auto result = data.TokenList.Remove(u8"foo bar");
      REQUIRE(
        (result.HasException() && result.GetException().Code() == ExceptionCode::InvalidCharacterError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Remove(u8"");
      REQUIRE((result.HasException() && result.GetException().Code() == ExceptionCode::SyntaxError));
      REQUIRE(data.TokenList.Length() == 0uz);
    }
  }

  TEST_CASE("DOMTokenList::Toggle", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    SECTION("Happy path")
    {
      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar").HasException());
      REQUIRE(data.TokenList.Length() == 1uz);
      REQUIRE(data.TokenList.Contains(u8"bar"));
      REQUIRE(data.TokenList.Item(0) == u8"bar");

      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar", true).HasException());
      REQUIRE(data.TokenList.Length() == 1uz);
      REQUIRE(data.TokenList.Contains(u8"bar"));
      REQUIRE(data.TokenList.Item(0) == u8"bar");

      REQUIRE_FALSE(data.TokenList.Remove(u8"bar").HasException());
      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar", true).HasException());
      REQUIRE(data.TokenList.Length() == 1uz);
      REQUIRE(data.TokenList.Contains(u8"bar"));
      REQUIRE(data.TokenList.Item(0) == u8"bar");

      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar").HasException());
      REQUIRE(data.TokenList.Length() == 0uz);
      REQUIRE_FALSE(data.TokenList.Contains(u8"bar"));

      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar", false).HasException());
      REQUIRE(data.TokenList.Length() == 0uz);

      REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
      REQUIRE_FALSE(data.TokenList.Toggle(u8"bar", false).HasException());
      REQUIRE(data.TokenList.Length() == 0uz);
    }

    SECTION("Invalid tokens")
    {
      REQUIRE(data.TokenList.Length() == 0uz);

      auto result = data.TokenList.Toggle(u8"foo bar");
      REQUIRE(
        (result.HasException() && result.GetException().Code() == ExceptionCode::InvalidCharacterError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Toggle(u8"");
      REQUIRE((result.HasException() && result.GetException().Code() == ExceptionCode::SyntaxError));
      REQUIRE(data.TokenList.Length() == 0uz);
    }
  }

  TEST_CASE("DOMTokenList::Replace", "[HTML][DOMTokenList]")
  {
    CommonTestData data;

    SECTION("Happy path")
    {
      REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
      REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
      REQUIRE(data.TokenList.Length() == 2uz);

      REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"foo bar");

      REQUIRE_FALSE(data.TokenList.Replace(u8"foo", u8"baz").HasException());
      REQUIRE(data.TokenList.Length() == 2uz);
      REQUIRE_FALSE(data.TokenList.Contains(u8"foo"));
      REQUIRE(data.TokenList.Contains(u8"baz"));

      REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"baz bar");
    }

    SECTION("Invalid tokens")
    {
      REQUIRE(data.TokenList.Length() == 0uz);

      auto result = data.TokenList.Replace(u8"foo bar", u8"foo");
      REQUIRE(
        (result.HasException() && result.GetException().Code() == ExceptionCode::InvalidCharacterError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Replace(u8"", u8"foo");
      REQUIRE((result.HasException() && result.GetException().Code() == ExceptionCode::SyntaxError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Replace(u8"foo", u8"foo bar");
      REQUIRE(
        (result.HasException() && result.GetException().Code() == ExceptionCode::InvalidCharacterError));
      REQUIRE(data.TokenList.Length() == 0uz);

      result = data.TokenList.Replace(u8"foo", u8"");
      REQUIRE((result.HasException() && result.GetException().Code() == ExceptionCode::SyntaxError));
      REQUIRE(data.TokenList.Length() == 0uz);
    }
  }

  TEST_CASE("DOMTokenList::Value", "[HTML][DOMTokenList]")
  {
    CommonTestData data;
    REQUIRE(data.TokenList.Value() == u8"");
    REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
    REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
    REQUIRE(data.TokenList.Value() == u8"foo bar");

    data.TokenList.Value(u8"baz qux");
    REQUIRE(data.TokenList.Value() == u8"baz qux");
    REQUIRE(data.Element->GetAttribute(u8"test-tokens") == u8"baz qux");
  }

  TEST_CASE("DOMTokenList::Supports", "[HTML][DOMTokenList]")
  {
    SECTION("Default function always returns true")
    {
      CommonTestData data;
      auto supports = data.TokenList.Supports(u8"foo");
      REQUIRE((supports.HasValue() && supports.Value() == true));
    }

    SECTION("Custom function can be provided")
    {
      CommonTestData data;
      data.TokenList = DOMTokenList(*data.Element, u8"test-tokens",
                                    [](Document &, DOMStringView token) { return token == u8"foo"; });

      auto supportsFoo = data.TokenList.Supports(u8"foo");
      REQUIRE((supportsFoo.HasValue() && supportsFoo.Value() == true));
      auto supportsBar = data.TokenList.Supports(u8"bar");
      REQUIRE((supportsBar.HasValue() && supportsBar.Value() == false));
    }
  }
}