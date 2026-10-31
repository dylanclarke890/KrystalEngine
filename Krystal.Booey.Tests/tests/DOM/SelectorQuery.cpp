#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD Element &ById(Document &document, DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    /// @brief The IDs of the elements in @p list, space-separated.
    KRYS_NODISCARD DOMString Ids(const NodeList &list)
    {
      DOMString ids;
      for (size_t i = 0; i < list.Length(); ++i)
      {
        auto node = list.Item(i);
        REQUIRE(node);
        if (!ids.empty())
        {
          ids += u8' ';
        }
        ids += Downcast<Element>(*node).Id();
      }
      return ids;
    }

    template <typename TParent>
    KRYS_NODISCARD DOMString QueryAllIds(TParent &parent, DOMStringView selectors)
    {
      auto result = parent.QuerySelectorAll(selectors);
      REQUIRE_FALSE(result.HasException());
      return Ids(*result.Value());
    }

    template <typename TParent>
    KRYS_NODISCARD DOMString QueryFirstId(TParent &parent, DOMStringView selectors)
    {
      auto result = parent.QuerySelector(selectors);
      REQUIRE_FALSE(result.HasException());
      return result.Value() ? result.Value()->Id() : DOMString(u8"(none)");
    }
  }

  TEST_CASE("Scope-match: querySelector and querySelectorAll match descendants of the node against its tree",
            "[DOM][SelectorQuery][dom:scope-match-a-selectors-string]")
  {
    auto document = Parse(u8"<!DOCTYPE html><div id=a class=x><p id=b class=x></p><span id=c title=t></span>"
                          u8"<p id=d><em id=e class='y x'></em></p></div><p id=f></p>");
    auto &a = ById(*document, u8"a");

    SECTION("Fast paths and the general path find the same elements in tree order")
    {
      REQUIRE(QueryAllIds(*document, u8"p") == u8"b d f");
      REQUIRE(QueryAllIds(*document, u8"P") == u8"b d f");
      REQUIRE(QueryAllIds(*document, u8".x") == u8"a b e");
      REQUIRE(QueryAllIds(*document, u8"[title=t]") == u8"c");
      REQUIRE(QueryAllIds(*document, u8"[title=T]") == u8"");
      REQUIRE(QueryAllIds(*document, u8"#d") == u8"d");
      REQUIRE(QueryAllIds(*document, u8"div > p") == u8"b d");
      REQUIRE(QueryAllIds(*document, u8"em, span, #a") == u8"a c e");
      REQUIRE(QueryAllIds(*document, u8"*|span") == u8"c");
      REQUIRE(QueryFirstId(*document, u8"p") == u8"b");
      REQUIRE(QueryFirstId(*document, u8".x") == u8"a");
      REQUIRE(QueryFirstId(*document, u8"p em") == u8"e");
      REQUIRE(QueryFirstId(*document, u8"article") == u8"(none)");
    }

    SECTION("Element queries search descendants but match selectors against the whole tree")
    {
      REQUIRE(QueryAllIds(a, u8".x") == u8"b e");
      REQUIRE(QueryAllIds(a, u8"body p") == u8"b d");
      REQUIRE(QueryAllIds(a, u8":scope > p") == u8"b d");
      REQUIRE(QueryAllIds(a, u8":scope em") == u8"e");
      REQUIRE(QueryFirstId(a, u8"div") == u8"(none)");
    }

    SECTION("Document fragments")
    {
      auto fragment = document->CreateDocumentFragment();
      auto first = document->CreateElement(u8"i").ReleaseValue();
      REQUIRE_FALSE(first->SetAttribute(u8"id", u8"i1").HasException());
      auto second = document->CreateElement(u8"i").ReleaseValue();
      REQUIRE_FALSE(second->SetAttribute(u8"id", u8"i2").HasException());
      REQUIRE_FALSE(fragment->AppendChild(*first).HasException());
      REQUIRE_FALSE(first->AppendChild(*second).HasException());
      REQUIRE(QueryAllIds(*fragment, u8"i") == u8"i1 i2");
      REQUIRE(QueryAllIds(*fragment, u8"i > i") == u8"i2");
      REQUIRE(QueryAllIds(*fragment, u8":scope") == u8"");
    }

    SECTION("The node itself is never a result")
    {
      REQUIRE(QueryAllIds(a, u8"div, p") == u8"b d");
      REQUIRE(QueryAllIds(a, u8":scope") == u8"");
    }

    SECTION("A document's :scope is its document element")
    {
      REQUIRE(QueryAllIds(*document, u8":scope > body > div") == u8"a");
      auto html = document->QuerySelector(u8":scope");
      REQUIRE_FALSE(html.HasException());
      REQUIRE(html.Value() == document->DocumentElement());
    }

    SECTION("Shadow trees are queried from their shadow root only")
    {
      auto shadow = a.AttachShadow({}).ReleaseValue();
      auto inner = document->CreateElement(u8"p").ReleaseValue();
      REQUIRE_FALSE(inner->SetAttribute(u8"id", u8"inner").HasException());
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

      REQUIRE(QueryAllIds(*document, u8"p") == u8"b d f");
      REQUIRE(QueryAllIds(*shadow, u8"p") == u8"inner");
      REQUIRE(QueryAllIds(*shadow, u8":host > p") == u8"inner");
      REQUIRE(QueryAllIds(*shadow, u8"div > p") == u8"");
    }

    SECTION("Elements in no namespace")
    {
      auto element = document->CreateElementNS(DOMStringAtom::Null(), u8"p").ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"id", u8"nons").HasException());
      REQUIRE_FALSE(ById(*document, u8"f").AppendChild(*element).HasException());

      REQUIRE(QueryAllIds(*document, u8"|p") == u8"nons");
      REQUIRE(QueryAllIds(*document, u8"|*") == u8"nons");
      REQUIRE(QueryAllIds(*document, u8"|*, |p") == u8"nons");
    }
  }

  // FIXME: A virtual scoping root doesn't act as the parent of its top-level elements (SelectorChecker.cpp).
  TEST_CASE("Scope-match: a fragment or shadow root is the parent of its top-level elements for :scope",
            "[DOM][SelectorQuery][dom:scope-match-a-selectors-string][!shouldfail]")
  {
    auto document = Parse(u8"<!DOCTYPE html><div id=host></div>");
    auto fragment = document->CreateDocumentFragment();
    auto first = document->CreateElement(u8"i").ReleaseValue();
    REQUIRE_FALSE(first->SetAttribute(u8"id", u8"i1").HasException());
    auto second = document->CreateElement(u8"i").ReleaseValue();
    REQUIRE_FALSE(second->SetAttribute(u8"id", u8"i2").HasException());
    REQUIRE_FALSE(fragment->AppendChild(*first).HasException());
    REQUIRE_FALSE(first->AppendChild(*second).HasException());

    auto shadow = ById(*document, u8"host").AttachShadow({}).ReleaseValue();
    auto inner = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(inner->SetAttribute(u8"id", u8"inner").HasException());
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    CHECK(QueryAllIds(*fragment, u8":scope > i") == u8"i1");
    CHECK(QueryAllIds(*fragment, u8":scope i") == u8"i1 i2");
    CHECK(QueryAllIds(*shadow, u8":scope > p") == u8"inner");
  }

  TEST_CASE("SelectorQuery: matches and closest", "[DOM][SelectorQuery]")
  {
    auto document = Parse(u8"<!DOCTYPE html><section id=s class=c><div id=d><p id=p></p></div></section>");
    auto &p = ById(*document, u8"p");

    auto closestId = [&](DOMStringView selectors)
    {
      auto result = p.Closest(selectors);
      REQUIRE_FALSE(result.HasException());
      return result.Value() ? result.Value()->Id() : DOMString(u8"(none)");
    };
    REQUIRE(closestId(u8"p") == u8"p");
    REQUIRE(closestId(u8"div") == u8"d");
    REQUIRE(closestId(u8".c") == u8"s");
    REQUIRE(closestId(u8"section > div, span") == u8"d");
    REQUIRE(closestId(u8"article") == u8"(none)");
    REQUIRE(closestId(u8":scope") == u8"p");

    auto matches = p.Matches(u8"div > p");
    REQUIRE_FALSE(matches.HasException());
    REQUIRE(matches.Value());
    auto webkitMatches = p.WebkitMatchesSelector(u8"section p:only-child");
    REQUIRE_FALSE(webkitMatches.HasException());
    REQUIRE(webkitMatches.Value());
  }

  TEST_CASE("Scope-match: an invalid selector throws a SyntaxError",
            "[DOM][SelectorQuery][dom:scope-match-a-selectors-string]")
  {
    auto document = Parse(u8"<!DOCTYPE html><p id=p></p>");
    auto &p = ById(*document, u8"p");

    auto requireSyntaxError = [](const auto &result)
    {
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::SyntaxError);
    };
    requireSyntaxError(document->QuerySelector(u8""));
    requireSyntaxError(document->QuerySelector(u8"["));
    requireSyntaxError(document->QuerySelectorAll(u8"p,"));
    requireSyntaxError(document->QuerySelectorAll(u8"ns|p"));
    requireSyntaxError(p.Matches(u8":unknown-pseudo"));
    requireSyntaxError(p.Closest(u8"a >"));
  }

  TEST_CASE("SelectorQuery: queries are cached", "[DOM][SelectorQuery]")
  {
    auto document = Parse(u8"<!DOCTYPE html><p id=p></p>");
    auto first = document->SelectorQueryForString(u8"p > em");
    auto second = document->SelectorQueryForString(u8"p > em");
    REQUIRE(first.HasValue());
    REQUIRE(second.HasValue());
    REQUIRE(&first.Value() == &second.Value());

    // A quirks mode document parses with a different context, so it gets its own query.
    auto quirks = Parse(u8"<p id=p></p>");
    auto other = quirks->SelectorQueryForString(u8"p > em");
    REQUIRE(other.HasValue());
    REQUIRE(&other.Value() != &first.Value());

    SelectorQueryCache::Singleton().Clear();
  }
}
