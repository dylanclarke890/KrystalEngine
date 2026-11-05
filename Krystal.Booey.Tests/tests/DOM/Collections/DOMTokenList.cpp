#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLIFrameElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOutputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
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

  TEST_CASE(
    "DOMTokenList.length: the size of its token set",
    "[DOM][DOMTokenList][dom:interface-domtokenlist][dom:dom-domtokenlist-length][dom:concept-dtl-tokens]")
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

  TEST_CASE("DOMTokenList.item(): the token at the index, or null past the end",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-item]")
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

  TEST_CASE("DOMTokenList: its indexed getter is item()", "[DOM][DOMTokenList][dom:dom-domtokenlist-item]")
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

  TEST_CASE("DOMTokenList.contains(): whether the token is in its token set",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-contains]")
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

  TEST_CASE("DOMTokenList.add(): appends each token not already in it, then updates the attribute",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-add][dom:concept-dtl-update]")
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

  TEST_CASE("DOMTokenList.remove(): removes each token, then updates the attribute",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-remove][dom:concept-dtl-update]")
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

  TEST_CASE("DOMTokenList.toggle(): removes or appends the token, or as force says",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-toggle]")
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

  TEST_CASE("DOMTokenList.replace(): replaces the token with the new one",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-replace]")
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

  TEST_CASE("DOMTokenList.value: gets and sets its attribute value",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-value][dom:concept-dtl-serialize]")
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

  TEST_CASE("DOMTokenList.supports(): throws a TypeError when its attribute defines no supported tokens",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-supports][dom:concept-domtokenlist-validation]"
            "[dom:concept-supported-tokens]")
  {
    CommonTestData data;
    auto supports = data.TokenList.Supports(u8"foo");
    REQUIRE(supports.HasException());
    CHECK(supports.GetException().Code() == ExceptionCode::TypeError);
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

  TEST_CASE("DOMTokenList: its token set follows its attribute",
            "[DOM][DOMTokenList][dom:concept-dtl-serialize][dom:domtokenlist-element]"
            "[dom:domtokenlist-attribute-name]")
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

  TEST_CASE("DOMTokenList: its token set when it's created and as it updates its attribute",
            "[DOM][DOMTokenList][dom:sets][dom:concept-dtl-serialize][dom:concept-dtl-update]")
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

  TEST_CASE("DOMTokenList.replace(): a new token already in the set takes the place of the first of the two",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-replace]")
  {
    CommonTestData data;
    data.TokenList.Value(u8"a b c");

    SECTION("After the token: the token's place")
    {
      CHECK(data.TokenList.Replace(u8"a", u8"c").ReleaseValue());
      CHECK(data.TokenList.Value() == u8"c b");
      CHECK(data.TokenList.Length() == 2uz);
    }

    SECTION("Before the token: its own place")
    {
      CHECK(data.TokenList.Replace(u8"c", u8"a").ReleaseValue());
      CHECK(data.TokenList.Value() == u8"a b");
    }

    SECTION("The token itself")
    {
      CHECK(data.TokenList.Replace(u8"b", u8"b").ReleaseValue());
      CHECK(data.TokenList.Value() == u8"a b c");
    }

    SECTION("A token not in the set changes nothing")
    {
      CHECK_FALSE(data.TokenList.Replace(u8"d", u8"a").ReleaseValue());
      CHECK(data.TokenList.Value() == u8"a b c");
    }

    SECTION("An empty token is checked for before whitespace in either")
    {
      auto result = data.TokenList.Replace(u8"a b", u8"");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::SyntaxError);
    }
  }

  TEST_CASE("DOMTokenList update steps: an absent attribute stays absent while the token set is empty",
            "[DOM][DOMTokenList][dom:concept-dtl-update]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.TokenList.Remove(u8"a").HasException());
    CHECK_FALSE(data.Element->HasAttribute(u8"class"));
    CHECK_FALSE(data.TokenList.Toggle(u8"a", false).ReleaseValue());
    CHECK_FALSE(data.Element->HasAttribute(u8"class"));

    // Once it's there, it's set to the serialized set, even when that's empty.
    REQUIRE_FALSE(data.Element->SetAttribute(u8"class", u8" a  a ").HasException());
    REQUIRE_FALSE(data.TokenList.Remove(u8"b").HasException());
    CHECK(data.Element->GetAttribute(u8"class") == u8"a");
    REQUIRE_FALSE(data.TokenList.Remove(u8"a").HasException());
    CHECK(data.Element->GetAttribute(u8"class") == u8"");
  }

  TEST_CASE("DOMTokenList.value: its serialize steps give the attribute value, unparsed",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-value][dom:concept-dtl-serialize]")
  {
    CommonTestData data;
    data.TokenList.Value(u8" b  a b ");
    CHECK(data.TokenList.Value() == u8" b  a b ");
    CHECK(data.TokenList.Length() == 2uz);
  }

  TEST_CASE("DOMTokenList.supports(): an element's supported tokens, compared ASCII case-insensitively",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-supports][dom:concept-domtokenlist-validation]"
            "[dom:concept-supported-tokens]")
  {
    // A document can have only one element child, so each element gets its own.
    auto newDocument = []
    {
      return CreateRef<HTMLDocument>();
    };

    SECTION("a's relList")
    {
      auto document = newDocument();
      auto &anchor = CreateHTML<html::HTMLAnchorElement>(*document, u8"a");
      CHECK(anchor.RelList().Supports(u8"NoOpener").ReleaseValue());
      CHECK(anchor.RelList().Supports(u8"noreferrer").ReleaseValue());
      CHECK_FALSE(anchor.RelList().Supports(u8"stylesheet").ReleaseValue());
    }

    SECTION("area's and form's relList")
    {
      auto areaDocument = newDocument();
      auto &area = CreateHTML<html::HTMLAreaElement>(*areaDocument, u8"area");
      CHECK(area.RelList().Supports(u8"opener").ReleaseValue());

      auto formDocument = newDocument();
      auto &form = CreateHTML<html::HTMLFormElement>(*formDocument, u8"form");
      CHECK_FALSE(form.RelList().Supports(u8"icon").ReleaseValue());
    }

    SECTION("blocking")
    {
      auto styleDocument = newDocument();
      auto &style = CreateHTML<html::HTMLStyleElement>(*styleDocument, u8"style");
      CHECK(style.Blocking().Supports(u8"RENDER").ReleaseValue());
      CHECK_FALSE(style.Blocking().Supports(u8"paint").ReleaseValue());

      auto scriptDocument = newDocument();
      auto &script = CreateHTML<html::HTMLScriptElement>(*scriptDocument, u8"script");
      CHECK(script.Blocking().Supports(u8"render").ReleaseValue());

      auto linkDocument = newDocument();
      auto &link = CreateHTML<html::HTMLLinkElement>(*linkDocument, u8"link");
      CHECK(link.Blocking().Supports(u8"render").ReleaseValue());
    }

    SECTION("No supported tokens: part, link's sizes and output's htmlFor")
    {
      auto divDocument = newDocument();
      CHECK(CreateHTML<Element>(*divDocument, u8"div").Part().Supports(u8"a").HasException());

      auto linkDocument = newDocument();
      CHECK(
        CreateHTML<html::HTMLLinkElement>(*linkDocument, u8"link").Sizes().Supports(u8"any").HasException());

      auto outputDocument = newDocument();
      auto &output = CreateHTML<html::HTMLOutputElement>(*outputDocument, u8"output");
      CHECK(output.HtmlFor().Supports(u8"a").HasException());
    }
  }

  TEST_CASE("DOMTokenList.supports(): link's relList supports the link types Booey processes",
            "[DOM][DOMTokenList][dom:dom-domtokenlist-supports]")
  {
    auto linkDocument = CreateRef<HTMLDocument>();
    auto &link = CreateHTML<html::HTMLLinkElement>(*linkDocument, u8"link");
    CHECK(link.RelList().Supports(u8"stylesheet").ReleaseValue());
    CHECK(link.RelList().Supports(u8"Alternate").ReleaseValue());
    CHECK_FALSE(link.RelList().Supports(u8"nonsense").ReleaseValue());
    CHECK_FALSE(link.RelList().Supports(u8"preload").ReleaseValue());
  }
}
