#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLIFrameElement.hpp"
#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<Element> Element;
      DOMTokenList &TokenList;

      CommonTestData()
          : Document(CreateRef<dom::Document>()), Element(Document->CreateElement(u8"div").Value()),
            TokenList(Element->ClassList())
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
      REQUIRE(data.Element->GetAttribute(u8"class") == u8"foo");

      REQUIRE_FALSE(data.TokenList.Add(u8"foo").HasException());
      REQUIRE(data.TokenList.Length() == 1uz);

      REQUIRE_FALSE(data.TokenList.Add(u8"bar").HasException());
      REQUIRE(data.TokenList.Length() == 2uz);

      REQUIRE(data.Element->GetAttribute(u8"class") == u8"foo bar");
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
      REQUIRE(data.Element->GetAttribute(u8"class") == u8"");

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

      REQUIRE(data.Element->GetAttribute(u8"class") == u8"foo bar");

      REQUIRE_FALSE(data.TokenList.Replace(u8"foo", u8"baz").HasException());
      REQUIRE(data.TokenList.Length() == 2uz);
      REQUIRE_FALSE(data.TokenList.Contains(u8"foo"));
      REQUIRE(data.TokenList.Contains(u8"baz"));

      REQUIRE(data.Element->GetAttribute(u8"class") == u8"baz bar");
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
    REQUIRE(data.Element->GetAttribute(u8"class") == u8"baz qux");
  }

  TEST_CASE("DOMTokenList::Supports", "[HTML][DOMTokenList]")
  {
    SECTION("Default function always returns true")
    {
      CommonTestData data;
      auto supports = data.TokenList.Supports(u8"foo");
      REQUIRE((supports.HasValue() && supports.Value() == true));
    }
  }

  namespace
  {
    /// @brief The tokens of @p list, joined by spaces.
    DOMString Joined(DOMTokenList &list)
    {
      DOMString joined;
      for (auto &token : list)
      {
        joined += joined.empty() ? u8"" : u8" ";
        joined += token;
      }
      return joined;
    }

    /// @brief Checks that @p list, read before its attribute @p attribute changes, follows each change.
    void CheckFollowsAttribute(Element &element, DOMTokenList &list, DOMStringAtom attribute)
    {
      REQUIRE(list.Length() == 0);

      REQUIRE_FALSE(element.SetAttribute(attribute, u8"a b a").HasException());
      CHECK(Joined(list) == u8"a b");
      CHECK(list.Contains(u8"b"));

      REQUIRE_FALSE(element.SetAttribute(attribute, u8"  c ").HasException());
      CHECK(Joined(list) == u8"c");
      CHECK(list.Length() == 1);

      element.RemoveAttribute(attribute);
      CHECK(list.Length() == 0);
      CHECK_FALSE(list.Contains(u8"c"));
    }

    template <typename TElement>
    TElement &CreateHTML(Document &document, DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      REQUIRE_FALSE(document.AppendChild(*element.Value()).HasException());
      if constexpr (SameType<TElement, Element>)
      {
        return *element.Value();
      }
      else
      {
        return Downcast<TElement>(Downcast<html::HTMLElement>(*element.Value()));
      }
    }
  }

  TEST_CASE("DOMTokenList follows its attribute", "[HTML][DOMTokenList]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("classList")
    {
      auto &element = CreateHTML<Element>(*document, u8"div");
      CheckFollowsAttribute(element, element.ClassList(), u8"class");
    }
    SECTION("part")
    {
      auto &element = CreateHTML<Element>(*document, u8"div");
      CheckFollowsAttribute(element, element.Part(), u8"part");
    }
    SECTION("a's relList")
    {
      auto &element = CreateHTML<html::HTMLAnchorElement>(*document, u8"a");
      CheckFollowsAttribute(element, element.RelList(), u8"rel");
    }
    SECTION("area's relList")
    {
      auto &element = CreateHTML<html::HTMLAreaElement>(*document, u8"area");
      CheckFollowsAttribute(element, element.RelList(), u8"rel");
    }
    SECTION("form's relList")
    {
      auto &element = CreateHTML<html::HTMLFormElement>(*document, u8"form");
      CheckFollowsAttribute(element, element.RelList(), u8"rel");
    }
    SECTION("iframe's sandbox")
    {
      auto &element = CreateHTML<html::HTMLIFrameElement>(*document, u8"iframe");
      CheckFollowsAttribute(element, element.Sandbox(), u8"sandbox");
    }
    SECTION("link's relList, sizes and blocking")
    {
      auto &element = CreateHTML<html::HTMLLinkElement>(*document, u8"link");
      CheckFollowsAttribute(element, element.RelList(), u8"rel");
      CheckFollowsAttribute(element, element.Sizes(), u8"sizes");
      CheckFollowsAttribute(element, element.Blocking(), u8"blocking");
    }
    SECTION("output's htmlFor")
    {
      auto &element = CreateHTML<html::HTMLOutputElement>(*document, u8"output");
      CheckFollowsAttribute(element, element.HtmlFor(), u8"for");
    }
    SECTION("script's blocking")
    {
      auto &element = CreateHTML<html::HTMLScriptElement>(*document, u8"script");
      CheckFollowsAttribute(element, element.Blocking(), u8"blocking");
    }
    SECTION("style's blocking")
    {
      auto &element = CreateHTML<html::HTMLStyleElement>(*document, u8"style");
      CheckFollowsAttribute(element, element.Blocking(), u8"blocking");
    }
    SECTION("SVG a's relList")
    {
      auto element = document->CreateElementNS(DOMStringAtom(infra::Namespaces::SVG), DOMStringAtom(u8"a"));
      REQUIRE_FALSE(element.HasException());
      auto &anchor = Downcast<svg::SVGAElement>(Downcast<svg::SVGElement>(*element.Value()));
      CheckFollowsAttribute(anchor, anchor.RelList(), u8"rel");
    }
  }

  TEST_CASE("DOMTokenList and its attribute", "[HTML][DOMTokenList]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto &element = CreateHTML<Element>(*document, u8"div");

    SECTION("An attribute set before the list is read")
    {
      REQUIRE_FALSE(element.SetAttribute(u8"class", u8"x y").HasException());
      CHECK(Joined(element.ClassList()) == u8"x y");
    }

    SECTION("The list's own changes keep its tokens")
    {
      auto &list = element.ClassList();
      REQUIRE_FALSE(list.Add(List<DOMString> {u8"a", u8"b"}).HasException());
      CHECK(element.GetAttribute(u8"class") == u8"a b");
      CHECK(Joined(list) == u8"a b");
      REQUIRE_FALSE(list.Remove(u8"a").HasException());
      CHECK(element.GetAttribute(u8"class") == u8"b");
      CHECK(Joined(list) == u8"b");
    }

    SECTION("An attribute of the same name in another namespace")
    {
      auto &list = element.ClassList();
      REQUIRE_FALSE(element.SetAttribute(u8"class", u8"a").HasException());
      REQUIRE_FALSE(element.SetAttributeNS(DOMStringAtom(u8"urn:x"), u8"x:class", u8"b").HasException());
      CHECK(Joined(list) == u8"a");
    }
  }
}
