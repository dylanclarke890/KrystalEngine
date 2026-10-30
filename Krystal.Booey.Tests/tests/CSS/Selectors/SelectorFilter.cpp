#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/HasSelectorFilter.hpp"
#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/Style/SelectorMatchingState.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD dom::Element &ById(dom::Document &document, dom::DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    KRYS_NODISCARD SelectorList ParseSelector(const dom::Document &document, CSSOMStringView text)
    {
      auto list = SelectorParser::ParseSelectorList(text, SelectorParserContext(document));
      REQUIRE(list);
      return krys::move(*list);
    }

    KRYS_NODISCARD uint32 HashCount(const SelectorFilter::Hashes &hashes) noexcept
    {
      uint32 count = 0;
      while (count < hashes.size() && hashes[count])
      {
        ++count;
      }
      return count;
    }
  }

  TEST_CASE("SelectorFilter: hashes of ancestor identifiers", "[CSS][Selectors][SelectorFilter]")
  {
    auto document = Parse(u8"<!DOCTYPE html>");
    auto hashes = [&](CSSOMStringView text)
    {
      return SelectorFilter::CollectHashes(*ParseSelector(*document, text).FirstSelectorComponent());
    };

    // The subject's own compound isn't collected.
    REQUIRE(HashCount(hashes(u8"div.a#b[c]")) == 0u);
    REQUIRE(HashCount(hashes(u8"div span")) == 1u);
    REQUIRE(HashCount(hashes(u8"div.a > span")) == 2u);
    // Siblings aren't ancestors, but their ancestors are.
    REQUIRE(HashCount(hashes(u8"div + span")) == 0u);
    REQUIRE(HashCount(hashes(u8"section div + span")) == 1u);
    // Universal selectors and the class, id and style attributes aren't collected.
    REQUIRE(HashCount(hashes(u8"* span")) == 0u);
    REQUIRE(HashCount(hashes(u8"[class] span")) == 0u);
    // Single-argument :is() and :where() are looked into.
    REQUIRE(HashCount(hashes(u8":is(.a) span")) == 1u);
    REQUIRE(HashCount(hashes(u8":is(.a, .b) span")) == 0u);
    // At most four, preferring IDs, attributes and classes over tags.
    REQUIRE(HashCount(hashes(u8"a b c d e span")) == 4u);
    REQUIRE(hashes(u8"#x .y span") == hashes(u8".y #x span"));
    // Duplicates are dropped.
    REQUIRE(HashCount(hashes(u8".a .a span")) == 1u);
  }

  TEST_CASE("SelectorFilter: fast rejection", "[CSS][Selectors][SelectorFilter]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><div id=outer class='a b' data-x><section><p id=leaf></p></section></div>");
    auto &outer = ById(*document, u8"outer");
    auto &section = *outer.FirstElementChild();
    auto &leaf = ById(*document, u8"leaf");

    auto rejects = [&](const SelectorFilter &filter, CSSOMStringView text)
    {
      auto list = ParseSelector(*document, text);
      return filter.FastRejectSelector(SelectorFilter::CollectHashes(*list.FirstSelectorComponent()));
    };

    SelectorFilter filter;
    REQUIRE(filter.ParentStackIsEmpty());
    filter.PushParentInitializingIfNeeded(section);
    REQUIRE_FALSE(filter.ParentStackIsEmpty());
    REQUIRE(filter.ParentStackIsConsistent(&section));
    REQUIRE_FALSE(filter.ParentStackIsConsistent(&outer));

    REQUIRE_FALSE(rejects(filter, u8"div p"));
    REQUIRE_FALSE(rejects(filter, u8"#outer p"));
    REQUIRE_FALSE(rejects(filter, u8".a > section > p"));
    REQUIRE_FALSE(rejects(filter, u8"[data-x] p"));
    REQUIRE_FALSE(rejects(filter, u8"html body p"));
    REQUIRE(rejects(filter, u8".missing p"));
    REQUIRE(rejects(filter, u8"article p"));
    REQUIRE(rejects(filter, u8"#leaf p"));
    // A class isn't a tag.
    REQUIRE(rejects(filter, u8".section p"));
    // Nothing to reject on.
    REQUIRE_FALSE(rejects(filter, u8"p"));

    filter.PushParent(&leaf);
    REQUIRE_FALSE(rejects(filter, u8"#leaf span"));
    filter.PopParent();
    REQUIRE(rejects(filter, u8"#leaf span"));

    filter.PopParentsUntil(&outer);
    REQUIRE(filter.ParentStackIsConsistent(&outer));
    REQUIRE(rejects(filter, u8"section p"));

    filter.PopParentsUntil(nullptr);
    REQUIRE(filter.ParentStackIsEmpty());
    REQUIRE(filter.ParentStackIsConsistent(document.get()));
    REQUIRE(rejects(filter, u8"div p"));
  }

  TEST_CASE("SelectorFilter: quirks mode folds class and ID case", "[CSS][Selectors][SelectorFilter]")
  {
    auto document = Parse(u8"<div id=Outer class=Big><p></p></div>");
    REQUIRE(document->InQuirksMode());
    SelectorFilter filter;
    filter.PushParentInitializingIfNeeded(ById(*document, u8"Outer"));
    auto rejects = [&](CSSOMStringView text)
    {
      auto list = ParseSelector(*document, text);
      return filter.FastRejectSelector(SelectorFilter::CollectHashes(*list.FirstSelectorComponent()));
    };
    REQUIRE_FALSE(rejects(u8".big p"));
    REQUIRE_FALSE(rejects(u8"#outer p"));
  }

  TEST_CASE("HasSelectorFilter: children and descendants", "[CSS][Selectors][SelectorFilter][Style]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><div id=root><span class=child><b class=grandchild></b></span></div>");
    auto &root = ById(*document, u8"root");

    REQUIRE(style::HasSelectorFilter::TypeForMatchElement(style::MatchElement::HasChild)
            == style::HasSelectorFilter::Type::Children);
    REQUIRE(style::HasSelectorFilter::TypeForMatchElement(style::MatchElement::HasDescendant)
            == style::HasSelectorFilter::Type::Descendants);
    REQUIRE_FALSE(style::HasSelectorFilter::TypeForMatchElement(style::MatchElement::HasSibling));

    auto rejects = [&](const style::HasSelectorFilter &filter, CSSOMStringView text)
    {
      // The argument of :has() is the selector inside it.
      auto list = ParseSelector(*document, text);
      auto *has = list.FirstSelectorComponent();
      return filter.Reject(*has->SubSelectors()->FirstSelectorComponent());
    };

    style::HasSelectorFilter children(root, style::HasSelectorFilter::Type::Children);
    REQUIRE(children.GetType() == style::HasSelectorFilter::Type::Children);
    REQUIRE_FALSE(rejects(children, u8":has(> .child)"));
    REQUIRE(rejects(children, u8":has(> .grandchild)"));
    REQUIRE(rejects(children, u8":has(> .nope)"));

    style::HasSelectorFilter descendants(root, style::HasSelectorFilter::Type::Descendants);
    REQUIRE_FALSE(rejects(descendants, u8":has(.grandchild)"));
    REQUIRE_FALSE(rejects(descendants, u8":has(b)"));
    REQUIRE(rejects(descendants, u8":has(i)"));
    // Nothing to filter on.
    REQUIRE_FALSE(rejects(descendants, u8":has(*)"));
  }

  TEST_CASE("SelectorChecker: :has() match cache and filters", "[CSS][Selectors][SelectorChecker][Style]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><div id=a><p class=x></p></div><div id=b><section><p></p></section></div>");
    auto &a = ById(*document, u8"a");
    auto &b = ById(*document, u8"b");
    SelectorChecker checker(*document);
    style::SelectorMatchingState state;

    // The cache is keyed by selector, so the selectors outlive it.
    auto hasX = ParseSelector(*document, u8":has(.x)");
    auto hasY = ParseSelector(*document, u8":has(.y)");
    auto hasP = ParseSelector(*document, u8":has(p)");

    auto match = [&](const dom::Element &element, const SelectorList &list)
    {
      SelectorChecker::CheckingContext context(SelectorChecker::Mode::ResolvingStyle);
      context.SelectorMatchingState = &state;
      return checker.Match(*list.FirstSelectorComponent(), element, context);
    };

    REQUIRE(match(a, hasX));
    REQUIRE_FALSE(match(b, hasX));
    REQUIRE(state.HasPseudoClassMatchCache.size() == 2uz);
    // Cached results are reused.
    REQUIRE(match(a, hasX));
    REQUIRE_FALSE(match(b, hasX));
    REQUIRE(state.HasPseudoClassMatchCache.size() == 2uz);

    // A second selector for the same element builds a filter, which rejects .y without matching.
    REQUIRE_FALSE(match(b, hasY));
    auto filterKey = style::MakeHasPseudoClassFilterKey(b, style::HasSelectorFilter::Type::Descendants);
    REQUIRE(state.HasPseudoClassSelectorFilters.contains(filterKey));
    REQUIRE(state.HasPseudoClassSelectorFilters.at(filterKey) != nullptr);
    REQUIRE(state.HasPseudoClassMatchCache.size() == 2uz);
    REQUIRE(match(b, hasP));
    REQUIRE(state.HasPseudoClassMatchCache.size() == 3uz);

    // Results are the same without the cache.
    SelectorChecker::CheckingContext uncached(SelectorChecker::Mode::ResolvingStyle);
    auto list = ParseSelector(*document, u8":has(.x)");
    REQUIRE(checker.Match(*list.FirstSelectorComponent(), a, uncached));
    REQUIRE_FALSE(checker.Match(*list.FirstSelectorComponent(), b, uncached));
  }

  TEST_CASE("SelectorChecker: style relations", "[CSS][Selectors][SelectorChecker][Style]")
  {
    auto document =
      Parse(u8"<!DOCTYPE html><ul id=list><li id=first></li><li id=second></li><li id=third></li></ul>"
            u8"<p id=empty></p>");
    auto &list = ById(*document, u8"list");
    auto &first = ById(*document, u8"first");
    auto &second = ById(*document, u8"second");
    auto &third = ById(*document, u8"third");
    SelectorChecker checker(*document);

    auto relations = [&](const dom::Element &element, CSSOMStringView text,
                         SelectorChecker::Mode mode = SelectorChecker::Mode::ResolvingStyle)
    {
      SelectorChecker::CheckingContext context(mode);
      auto selectors = ParseSelector(*document, text);
      (void)checker.Match(*selectors.FirstSelectorComponent(), element, context);
      List<std::tuple<const dom::Element *, style::Relation::Type, uint32>> result;
      for (auto &relation : context.StyleRelations)
      {
        result.emplace_back(relation.Element, relation.RelationType, relation.Value);
      }
      return result;
    };

    using R = style::Relation;
    using Relations = List<std::tuple<const dom::Element *, R::Type, uint32>>;

    REQUIRE(relations(first, u8":first-child")
            == Relations {{&list, R::ChildrenAffectedByFirstChildRules, 1}, {&first, R::FirstChild, 1}});
    REQUIRE(relations(second, u8":first-child")
            == Relations {{&list, R::ChildrenAffectedByFirstChildRules, 1}});
    REQUIRE(relations(third, u8":last-child")
            == Relations {{&list, R::ChildrenAffectedByLastChildRules, 1}, {&third, R::LastChild, 1}});
    REQUIRE(
      relations(second, u8":nth-child(2)")
      == Relations {{&list, R::ChildrenAffectedByForwardPositionalRules, 1}, {&second, R::NthChildIndex, 2}});
    REQUIRE(relations(ById(*document, u8"empty"), u8":empty")
            == Relations {{&ById(*document, u8"empty"), R::AffectedByEmpty, 1}});
    // Consecutive siblings that affect the next sibling are merged.
    REQUIRE(relations(third, u8"#nope ~ li")
            == Relations {{&third, R::AffectedByPreviousSibling, 1}, {&first, R::AffectsNextSibling, 2}});
    REQUIRE(relations(second, u8"li + li")
            == Relations {{&second, R::AffectedByPreviousSibling, 1}, {&first, R::AffectsNextSibling, 1}});
    // Positions of elements other than the subject affect descendants.
    const dom::Element *body = document->Body().get();
    REQUIRE(relations(first, u8":first-of-type > *")
            == Relations {{body, R::DescendantsAffectedByForwardPositionalRules, 1}});
    // Only resolving style records relations.
    REQUIRE(relations(first, u8":first-child", SelectorChecker::Mode::QueryingRules) == Relations {});

    SECTION(":has() forwards positional relations as affecting :has()")
    {
      REQUIRE(relations(list, u8":has(> :first-child)")
              == Relations {{&list, R::ChildrenAffectedByFirstChildRules, 1}});
      REQUIRE(relations(list, u8":has(> :nth-child(2))")
              == Relations {{&list, R::AffectedByHasWithPositionalPseudoClass, 1},
                            {&list, R::AffectedByHasWithPositionalPseudoClass, 1}});
    }
  }
}
