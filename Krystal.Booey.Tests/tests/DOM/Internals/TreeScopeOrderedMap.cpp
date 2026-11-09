#include "Krystal.Booey/DOM/Internals/TreeScopeOrderedMap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  namespace
  {
    using Strings = List<std::string>;

    KRYS_NODISCARD Ref<HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    /// @brief The ID, or else the class, of each element matching @p selectors.
    template <typename TRoot>
    KRYS_NODISCARD Strings Query(TRoot &root, DOMStringView selectors)
    {
      auto list = root.QuerySelectorAll(selectors).ReleaseValue();
      Strings names;
      for (size_t i = 0uz; i < list->Length(); ++i)
      {
        auto &element = Downcast<Element>(*list->Item(i));
        auto name = element.Id();
        if (name.empty())
        {
          name = element.ClassName();
        }
        names.emplace_back(name.begin(), name.end());
      }
      return names;
    }

    KRYS_NODISCARD Ref<Element> Create(Document &document, DOMStringView id)
    {
      auto element = document.CreateElement(u8"div").ReleaseValue();
      element->Id(DOMString(id));
      return element;
    }
  }

  TEST_CASE("TreeScopeOrderedMap: the first element with an ID in tree order", "[DOM][TreeScopeOrderedMap]")
  {
    auto document = Parse(u8"<!DOCTYPE html><body><div id=a></div><div id=b></div>");
    auto &body = *document->Body();
    auto a = document->GetElementById(u8"a");
    auto b = document->GetElementById(u8"b");
    REQUIRE(a);
    REQUIRE(b);

    SECTION("an element that comes first takes over, and the next follows when it goes")
    {
      auto second = Create(*document, u8"a");
      REQUIRE_FALSE(body.AppendChild(*second).HasException());
      CHECK(document->GetElementById(u8"a") == a);

      auto first = Create(*document, u8"a");
      REQUIRE_FALSE(body.InsertBefore(*first, a.get()).HasException());
      CHECK(document->GetElementById(u8"a") == first);

      REQUIRE_FALSE(body.RemoveChild(*first).HasException());
      CHECK(document->GetElementById(u8"a") == a);
      REQUIRE_FALSE(body.RemoveChild(*a).HasException());
      CHECK(document->GetElementById(u8"a") == second);
      REQUIRE_FALSE(body.RemoveChild(*second).HasException());
      CHECK(document->GetElementById(u8"a") == nullptr);
    }

    SECTION("changing and removing the attribute")
    {
      a->Id(u8"c");
      CHECK(document->GetElementById(u8"a") == nullptr);
      CHECK(document->GetElementById(u8"c") == a);

      // Taking an ID another element has comes in tree order.
      a->Id(u8"b");
      CHECK(document->GetElementById(u8"b") == a);
      a->RemoveAttribute(u8"id");
      CHECK(document->GetElementById(u8"b") == b);

      // No element has the empty string as its ID.
      a->Id(u8"");
      CHECK(document->GetElementById(u8"") == nullptr);
      a->Id(u8"a");
      CHECK(document->GetElementById(u8"a") == a);
    }

    SECTION("a subtree enters and leaves as one")
    {
      auto parent = Create(*document, u8"parent");
      auto child = Create(*document, u8"child");
      REQUIRE_FALSE(parent->AppendChild(*child).HasException());
      CHECK(document->GetElementById(u8"child") == nullptr);

      REQUIRE_FALSE(body.AppendChild(*parent).HasException());
      CHECK(document->GetElementById(u8"parent") == parent);
      CHECK(document->GetElementById(u8"child") == child);

      // An attribute changed while it's outside the tree counts when it comes back.
      REQUIRE_FALSE(body.RemoveChild(*parent).HasException());
      CHECK(document->GetElementById(u8"child") == nullptr);
      child->Id(u8"renamed");
      REQUIRE_FALSE(body.AppendChild(*parent).HasException());
      CHECK(document->GetElementById(u8"child") == nullptr);
      CHECK(document->GetElementById(u8"renamed") == child);
    }

    SECTION("moving an element changes which is first")
    {
      auto second = Create(*document, u8"a");
      REQUIRE_FALSE(body.AppendChild(*second).HasException());
      CHECK(document->GetElementById(u8"a") == a);
      REQUIRE_FALSE(body.MoveBefore(*second, a.get()).HasException());
      CHECK(document->GetElementById(u8"a") == second);
    }

    SECTION("a fragment and an element's subtree aren't in the map")
    {
      auto fragment = document->CreateDocumentFragment();
      auto inFragment = Create(*document, u8"f");
      REQUIRE_FALSE(fragment->AppendChild(*inFragment).HasException());
      CHECK(fragment->GetElementById(u8"f") == inFragment);
      CHECK(document->GetElementById(u8"f") == nullptr);
    }

    SECTION("an element can't be found from the document it left")
    {
      auto other = CreateRef<HTMLDocument>();
      REQUIRE_FALSE(other->AdoptNode(*b).HasException());
      CHECK(document->GetElementById(u8"b") == nullptr);
    }
  }

  TEST_CASE("TreeScopeOrderedMap: each shadow tree has its own IDs", "[DOM][TreeScopeOrderedMap]")
  {
    auto document = Parse(u8"<!DOCTYPE html><body><div id=host></div><div id=outside></div>");
    auto host = document->GetElementById(u8"host");
    auto outside = document->GetElementById(u8"outside");
    REQUIRE(host);
    REQUIRE(outside);
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();

    auto inside = Create(*document, u8"inside");
    REQUIRE_FALSE(shadow->AppendChild(*inside).HasException());
    CHECK(shadow->GetElementById(u8"inside") == inside);
    CHECK(document->GetElementById(u8"inside") == nullptr);
    CHECK(shadow->GetElementById(u8"outside") == nullptr);

    SECTION("the host coming and going changes nothing")
    {
      REQUIRE_FALSE(document->Body()->RemoveChild(*host).HasException());
      CHECK(shadow->GetElementById(u8"inside") == inside);
      REQUIRE_FALSE(document->Body()->AppendChild(*host).HasException());
      CHECK(shadow->GetElementById(u8"inside") == inside);

      // A tree whose host isn't connected has IDs of its own too.
      REQUIRE_FALSE(document->Body()->RemoveChild(*host).HasException());
      auto added = Create(*document, u8"added");
      REQUIRE_FALSE(shadow->AppendChild(*added).HasException());
      CHECK(shadow->GetElementById(u8"added") == added);
    }

    SECTION("a move across trees moves the ID")
    {
      REQUIRE_FALSE(shadow->MoveBefore(*outside, nullptr).HasException());
      CHECK(document->GetElementById(u8"outside") == nullptr);
      CHECK(shadow->GetElementById(u8"outside") == outside);

      REQUIRE_FALSE(document->Body()->MoveBefore(*inside, nullptr).HasException());
      CHECK(document->GetElementById(u8"inside") == inside);
      CHECK(shadow->GetElementById(u8"inside") == nullptr);
    }

    SECTION("selectors look in the scope they run in")
    {
      CHECK(Query(*shadow, u8"#inside") == Strings {"inside"});
      CHECK(Query(*shadow, u8"#outside").empty());
      CHECK(Query(*document, u8"#inside").empty());
    }
  }

  TEST_CASE("SelectorDataList: finding elements by ID", "[DOM][TreeScopeOrderedMap][SelectorQuery]")
  {
    auto document = Parse(u8"<!DOCTYPE html><body>"
                          u8"<div id=a class=x><p id=b class=y></p><p class=in-a></p></div>"
                          u8"<p id=c class=y></p><span class=after></span>"
                          u8"<div id=empty-attr></div><div id='' class=blank></div>"
                          u8"<p id=dup class=first-dup></p><div><p id=dup class=second-dup></p></div>");
    auto a = document->GetElementById(u8"a");
    REQUIRE(a);

    SECTION("in the rightmost compound selector")
    {
      CHECK(Query(*document, u8"#a") == Strings {"a"});
      CHECK(Query(*document, u8"[id=a]") == Strings {"a"});
      CHECK(Query(*document, u8"div#a") == Strings {"a"});
      CHECK(Query(*document, u8"p#a").empty());
      CHECK(Query(*document, u8"#a.x") == Strings {"a"});
      CHECK(Query(*document, u8"#a.y").empty());
      CHECK(Query(*document, u8"#missing").empty());
      CHECK(Query(*document, u8"body #b") == Strings {"b"});
      CHECK(Query(*document, u8"p #b").empty());
    }

    SECTION("the element has to be below the root")
    {
      CHECK(Query(*a, u8"#b") == Strings {"b"});
      CHECK(Query(*a, u8"#c").empty());
      CHECK(Query(*a, u8"#a").empty());
    }

    SECTION("an ID several elements share")
    {
      CHECK(Query(*document, u8"#dup") == Strings {"dup", "dup"});
      CHECK(Query(*document, u8"div #dup") == Strings {"dup"});

      // They come in tree order, which changes as they do.
      auto classes = [&]
      {
        auto list = document->QuerySelectorAll(u8"#dup").ReleaseValue();
        Strings names;
        for (size_t i = 0uz; i < list->Length(); ++i)
        {
          auto name = Downcast<Element>(*list->Item(i)).ClassName();
          names.emplace_back(name.begin(), name.end());
        }
        return names;
      };
      auto first = document->QuerySelector(u8".first-dup").ReleaseValue();
      auto second = document->QuerySelector(u8".second-dup").ReleaseValue();
      REQUIRE(first);
      REQUIRE(second);
      CHECK(classes() == Strings {"first-dup", "second-dup"});
      CHECK(document->QuerySelector(u8"#dup").ReleaseValue() == first);

      REQUIRE_FALSE(document->Body()->MoveBefore(*second, first.get()).HasException());
      CHECK(classes() == Strings {"second-dup", "first-dup"});
      CHECK(document->QuerySelector(u8"#dup").ReleaseValue() == second);

      auto third = document->CreateElement(u8"p").ReleaseValue();
      third->Id(u8"dup");
      third->ClassName(u8"third-dup");
      REQUIRE_FALSE(document->Body()->InsertBefore(*third, second.get()).HasException());
      CHECK(classes() == Strings {"third-dup", "second-dup", "first-dup"});

      REQUIRE_FALSE(document->Body()->RemoveChild(*second).HasException());
      CHECK(classes() == Strings {"third-dup", "first-dup"});

      // Only those below the root.
      auto holder = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(document->Body()->AppendChild(*holder).HasException());
      REQUIRE_FALSE(holder->AppendChild(*first).HasException());
      CHECK(Query(*holder, u8"#dup") == Strings {"dup"});

      // Down to one again, the list isn't needed.
      REQUIRE_FALSE(document->Body()->RemoveChild(*third).HasException());
      CHECK(classes() == Strings {"first-dup"});
    }

    SECTION("an empty ID isn't in the map, but the attribute can still match")
    {
      CHECK(Query(*document, u8"[id='']") == Strings {"blank"});
    }

    SECTION("in a compound selector to the left")
    {
      CHECK(Query(*document, u8"#a .y") == Strings {"b"});
      CHECK(Query(*document, u8"#a > .y") == Strings {"b"});
      CHECK(Query(*document, u8"#a .y, #c") == Strings {"b", "c"});
      CHECK(Query(*document, u8"#a + .y") == Strings {"c"});
      CHECK(Query(*document, u8"#a ~ .after") == Strings {"after"});
      CHECK(Query(*document, u8"#a + .y + .after") == Strings {"after"});
      CHECK(Query(*document, u8"#a ~ .y .nothing").empty());
      CHECK(Query(*document, u8"#missing .y").empty());
      // What's past a sibling of the ID isn't below it.
      CHECK(Query(*document, u8"body > #a ~ .y") == Strings {"c"});
    }

    SECTION("in a compound selector to the left, from an element")
    {
      // The ID is the root, or above it, so it can't narrow the search.
      CHECK(Query(*a, u8"#a .y") == Strings {"b"});
      CHECK(Query(*a, u8"body #b") == Strings {"b"});
      CHECK(Query(*a, u8":scope > .y") == Strings {"b"});
      // Outside the root altogether.
      CHECK(Query(*a, u8"#c + .after").empty());
    }

    SECTION("outside a tree scope, elements are still found")
    {
      auto fragment = document->CreateDocumentFragment();
      auto inFragment = Create(*document, u8"f");
      REQUIRE_FALSE(fragment->AppendChild(*inFragment).HasException());
      CHECK(Query(*fragment, u8"#f") == Strings {"f"});

      REQUIRE_FALSE(document->Body()->RemoveChild(*a).HasException());
      CHECK(Query(*a, u8"#b") == Strings {"b"});
      CHECK(Query(*document, u8"#b").empty());
    }
  }

  TEST_CASE("SelectorDataList: IDs match without case in quirks mode",
            "[DOM][TreeScopeOrderedMap][SelectorQuery]")
  {
    auto quirks = Parse(u8"<body><div id=Mixed><p class=y></p></div>");
    REQUIRE(quirks->InQuirksMode());
    CHECK(Query(*quirks, u8"#Mixed") == Strings {"Mixed"});
    CHECK(Query(*quirks, u8"#mixed") == Strings {"Mixed"});
    CHECK(Query(*quirks, u8"#MIXED .y") == Strings {"y"});

    // The attribute's value and getElementById keep their case.
    CHECK(Query(*quirks, u8"[id=mixed]").empty());
    CHECK(quirks->GetElementById(u8"mixed") == nullptr);

    auto noQuirks = Parse(u8"<!DOCTYPE html><body><div id=Mixed><p class=y></p></div>");
    CHECK(Query(*noQuirks, u8"#mixed").empty());
    CHECK(Query(*noQuirks, u8"#mixed .y").empty());
  }
}
