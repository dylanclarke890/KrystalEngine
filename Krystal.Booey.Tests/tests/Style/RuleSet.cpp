#include "Krystal.Booey/Style/RuleSet.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include <string>

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    KRYS_NODISCARD Ref<css::StyleSheetContents> ParseSheet(utf8_stringview text)
    {
      css::ParserContext context(css::ParserMode::HTMLStandard);
      auto sheet = css::StyleSheetContents::Create(context);
      sheet->ParseString(css::CSSOMString(text));
      return sheet;
    }

    /// @brief The rule set of @p sheet, built for a screen.
    KRYS_NODISCARD Ref<RuleSet> Build(const css::StyleSheetContents &sheet)
    {
      static const css::mq::MediaQueryEvaluator screen {css::CSSOMStringAtom(u8"screen")};
      auto ruleSet = RuleSet::Create();
      {
        RuleSetBuilder builder(*ruleSet, screen);
        builder.AddRulesFromSheet(sheet);
      }
      return ruleSet;
    }

    KRYS_NODISCARD std::string Text(const RuleData &ruleData)
    {
      return ToNarrow(ruleData.Selector()->SelectorText());
    }

    /// @brief The selectors of @p rules, in order.
    KRYS_NODISCARD List<std::string> Texts(const RuleSet::RuleDataVector *rules)
    {
      List<std::string> texts;
      if (rules)
      {
        for (auto &ruleData : *rules)
        {
          texts.push_back(Text(ruleData));
        }
      }
      return texts;
    }

    KRYS_NODISCARD css::CSSOMStringAtom Atom(utf8_stringview string)
    {
      return css::CSSOMStringAtom(string);
    }

    /// @brief The only rule data in @p ruleSet whose selector is @p text, in any bucket.
    KRYS_NODISCARD const RuleData &Find(const RuleSet &ruleSet, utf8_stringview selector)
    {
      const RuleData *found = nullptr;
      auto search = [&](const RuleSet::RuleDataVector *rules)
      {
        if (!rules)
        {
          return;
        }
        for (auto &ruleData : *rules)
        {
          if (ruleData.Selector()->SelectorText() == selector)
          {
            REQUIRE_FALSE(found);
            found = &ruleData;
          }
        }
      };
      search(ruleSet.UniversalRules());
      search(ruleSet.RootElementRules());
      search(ruleSet.LinkPseudoClassRules());
      search(ruleSet.FocusPseudoClassRules());
      search(&ruleSet.HostPseudoClassRules());
      search(&ruleSet.SlottedPseudoElementRules());
      search(&ruleSet.PartPseudoElementRules());
      search(&ruleSet.CuePseudoRules());
      for (auto name : {u8"x", u8"a", u8"b", u8"c", u8"div", u8"span", u8"p", u8"title", u8"cue"})
      {
        search(ruleSet.IdRules(Atom(name)));
        search(ruleSet.ClassRules(Atom(name)));
        search(ruleSet.TagRules(Atom(name), true));
        search(ruleSet.AttributeRules(Atom(name), true));
        search(ruleSet.UserAgentPartRules(Atom(name)));
      }
      REQUIRE(found);
      return *found;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: rules are bucketed by their subject",
                   "[Style][RuleSet]")
  {
    auto sheet =
      ParseSheet(u8"#x {} .a.b {} .b {} [title] {} div {} * {} :link {} :focus {} :focus-visible {}"
                 u8":root {} :host {} ::slotted(p) {} ::part(p) {} ::cue {} ::cue(b) {}"
                 u8"::view-transition-group(x) {} ::view-transition-group(*) {} [style] {}"
                 u8".a span {} p, span {} :scope {} :is(:host) {}");
    auto ruleSet = Build(*sheet);

    REQUIRE(ruleSet->RuleCount() == 23u);

    REQUIRE(Texts(ruleSet->IdRules(Atom(u8"x"))) == List<std::string> {"#x"});
    // The class bucket with fewer rules is chosen: .b had none when .a.b was added, so it went into .a.
    REQUIRE(Texts(ruleSet->ClassRules(Atom(u8"a"))) == List<std::string> {".a.b"});
    REQUIRE(Texts(ruleSet->ClassRules(Atom(u8"b"))) == List<std::string> {".b"});
    REQUIRE(Texts(ruleSet->AttributeRules(Atom(u8"title"), true)) == List<std::string> {"[title]"});
    REQUIRE(Texts(ruleSet->AttributeRules(Atom(u8"title"), false)) == List<std::string> {"[title]"});
    REQUIRE(Texts(ruleSet->TagRules(Atom(u8"div"), true)) == List<std::string> {"div"});
    // Only the subject (rightmost compound) is bucketed.
    REQUIRE(Texts(ruleSet->TagRules(Atom(u8"span"), true)) == List<std::string> {".a span", "span"});
    REQUIRE(Texts(ruleSet->TagRules(Atom(u8"p"), true)) == List<std::string> {"p"});
    REQUIRE(Texts(ruleSet->LinkPseudoClassRules()) == List<std::string> {":link"});
    REQUIRE(Texts(ruleSet->FocusPseudoClassRules()) == List<std::string> {":focus"});
    REQUIRE(Texts(ruleSet->FocusVisiblePseudoClassRules()) == List<std::string> {":focus-visible"});
    REQUIRE(Texts(ruleSet->RootElementRules()) == List<std::string> {":root"});
    REQUIRE(Texts(&ruleSet->HostPseudoClassRules()) == List<std::string> {":host"});
    REQUIRE(Texts(&ruleSet->SlottedPseudoElementRules()) == List<std::string> {"::slotted(p)"});
    REQUIRE(Texts(&ruleSet->PartPseudoElementRules()) == List<std::string> {"::part(p)"});
    REQUIRE(Texts(ruleSet->UserAgentPartRules(Atom(u8"cue"))) == List<std::string> {"::cue"});
    REQUIRE(Texts(&ruleSet->CuePseudoRules()) == List<std::string> {"::cue(b)"});
    REQUIRE(Texts(ruleSet->NamedPseudoElementRules(Atom(u8"x")))
            == List<std::string> {"::view-transition-group(x)"});
    // `[style]` isn't bucketed: the style attribute is synchronised lazily.
    REQUIRE(Texts(ruleSet->UniversalRules())
            == List<std::string> {"*", "::view-transition-group(*)", "[style]", ":scope", ":is(:host)"});

    REQUIRE(ruleSet->HasAttributeRules());
    REQUIRE(ruleSet->HasUserAgentPartRules());
    REQUIRE(ruleSet->HasHostOrScopePseudoClassRulesInUniversalBucket());
    REQUIRE_FALSE(ruleSet->HasHostPseudoClassRulesMatchingInShadowTree());

    SECTION("Rules keep their position and index in their rule")
    {
      auto &p = Find(*ruleSet, u8"p");
      auto &span = *ruleSet->TagRules(Atom(u8"span"), true)->begin();
      REQUIRE(Text(span) == ".a span");
      REQUIRE(p.SelectorListIndex() == 0u);
      REQUIRE(p.SelectorIndex() == 0u);
      auto &secondSpan = (*ruleSet->TagRules(Atom(u8"span"), true))[1];
      REQUIRE(secondSpan.SelectorListIndex() == 1u);
      REQUIRE(&secondSpan.StyleRule() == &p.StyleRule());
      REQUIRE(secondSpan.Position() == p.Position() + 1);
    }

    SECTION(":host followed by a combinator matches in the shadow tree")
    {
      auto hostSheet = ParseSheet(u8":host > p {}");
      REQUIRE(Build(*hostSheet)->HasHostPseudoClassRulesMatchingInShadowTree());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: rule data", "[Style][RuleSet]")
  {
    auto sheet = ParseSheet(u8"* {} div {} #x {} .a {} :link {} div span {} span::marker {} ::cue {}"
                            u8"::cue(b) {} .a > .b span {} :visited {} p::before {}");
    auto ruleSet = Build(*sheet);

    SECTION("Rules that can match on their subject alone are flagged")
    {
      REQUIRE(Find(*ruleSet, u8"*").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::Universal);
      REQUIRE(Find(*ruleSet, u8"div").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::ClassC);
      REQUIRE(Find(*ruleSet, u8"#x").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::ClassA);
      REQUIRE(Find(*ruleSet, u8".a").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::ClassB);
      REQUIRE(Find(*ruleSet, u8":link").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::ClassB);
      REQUIRE(Find(*ruleSet, u8"div span").GetMatchBasedOnRuleHash() == MatchBasedOnRuleHash::None);
    }

    SECTION("Pseudo-element rules")
    {
      REQUIRE(Find(*ruleSet, u8"p::before").CanMatchPseudoElement());
      REQUIRE_FALSE(Find(*ruleSet, u8"div").CanMatchPseudoElement());
      REQUIRE(Find(*ruleSet, u8"span::marker").GetPropertyAllowlist() == PropertyAllowlist::Marker);
      REQUIRE(Find(*ruleSet, u8"::cue").GetPropertyAllowlist() == PropertyAllowlist::Cue);
      REQUIRE(Find(*ruleSet, u8"::cue(b)").GetPropertyAllowlist() == PropertyAllowlist::CueSelector);
      REQUIRE(Find(*ruleSet, u8"div").GetPropertyAllowlist() == PropertyAllowlist::None);
    }

    SECTION("Link match types")
    {
      REQUIRE(Find(*ruleSet, u8":link").LinkMatchType() == css::SelectorChecker::MatchLink);
      REQUIRE(Find(*ruleSet, u8":visited").LinkMatchType() == css::SelectorChecker::MatchVisited);
      REQUIRE(Find(*ruleSet, u8"div").LinkMatchType() == css::SelectorChecker::MatchAll);
    }

    SECTION("Ancestor identifiers are kept for the selector filter")
    {
      REQUIRE(Find(*ruleSet, u8"div").DescendantSelectorIdentifierHashes()[0] == 0u);
      auto &hashes = Find(*ruleSet, u8"div span").DescendantSelectorIdentifierHashes();
      REQUIRE(hashes[0] != 0u);
      REQUIRE(hashes[1] == 0u);
      // Classes are preferred over tags, and every ancestor compound is collected.
      auto &deep = Find(*ruleSet, u8".a > .b span").DescendantSelectorIdentifierHashes();
      REQUIRE(deep[0] != 0u);
      REQUIRE(deep[1] != 0u);
      REQUIRE(deep[2] == 0u);
    }

    SECTION("Rules are enabled")
    {
      REQUIRE(Find(*ruleSet, u8"div").IsEnabled());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: cascade layers", "[Style][RuleSet]")
  {
    auto sheet = ParseSheet(u8"@layer a, b; @layer b { .x {} } @layer a { .y {} @layer c { .z {} } }"
                            u8"@layer { .anon {} } @layer a.c { .w {} } .u {}");
    auto ruleSet = Build(*sheet);

    auto priority = [&](utf8_stringview className)
    {
      auto *rules = ruleSet->ClassRules(Atom(className));
      REQUIRE(rules);
      REQUIRE(rules->size() == 1uz);
      return ruleSet->CascadeLayerPriorityFor(rules->front());
    };

    // Unlayered rules have the highest priority, and later sibling layers beat earlier ones.
    REQUIRE(priority(u8"u") == RuleSet::CascadeLayerPriorityForUnlayered);
    REQUIRE(priority(u8"x") > priority(u8"y"));
    REQUIRE(priority(u8"anon") > priority(u8"x"));
    // A parent layer beats its nested layers.
    REQUIRE(priority(u8"y") > priority(u8"z"));
    // The same layer name is the same layer.
    REQUIRE(priority(u8"w") == priority(u8"z"));
    REQUIRE(priority(u8"z") > RuleSet::CascadeLayerPriorityForPresentationalHints);

    SECTION("Anonymous layers are distinct")
    {
      auto anonymous = ParseSheet(u8"@layer { .a {} } @layer { .b {} }");
      auto anonymousRuleSet = Build(*anonymous);
      auto a = anonymousRuleSet->CascadeLayerPriorityFor(anonymousRuleSet->ClassRules(Atom(u8"a"))->front());
      auto b = anonymousRuleSet->CascadeLayerPriorityFor(anonymousRuleSet->ClassRules(Atom(u8"b"))->front());
      REQUIRE(b > a);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: conditional rules", "[Style][RuleSet]")
  {
    SECTION("Media queries are evaluated")
    {
      auto sheet =
        ParseSheet(u8"@media print { .a {} } @media screen { .b {} } @media (min-width: 1px) { .c {} }");
      auto ruleSet = Build(*sheet);
      REQUIRE_FALSE(ruleSet->ClassRules(Atom(u8"a")));
      REQUIRE(ruleSet->ClassRules(Atom(u8"b")));
      // Without a view, viewport features don't match.
      REQUIRE_FALSE(ruleSet->ClassRules(Atom(u8"c")));
      REQUIRE(ruleSet->HasViewportDependentMediaQueries());
    }

    SECTION("Supports conditions are evaluated")
    {
      auto sheet = ParseSheet(u8"@supports (display: grid) { .a {} } @supports (nonsense: 1) { .b {} }");
      auto ruleSet = Build(*sheet);
      REQUIRE(ruleSet->ClassRules(Atom(u8"a")));
      REQUIRE_FALSE(ruleSet->ClassRules(Atom(u8"b")));
    }

    SECTION("Container queries are recorded")
    {
      auto sheet =
        ParseSheet(u8"@container card (min-width: 1px) { @container (min-height: 1px) { .a {} } } .b {}");
      auto ruleSet = Build(*sheet);
      REQUIRE(ruleSet->HasContainerQueries());
      REQUIRE(ruleSet->ContainerQueryRules().size() == 2uz);
      auto queries = ruleSet->ContainerQueriesFor(ruleSet->ClassRules(Atom(u8"a"))->front());
      REQUIRE(queries.size() == 2uz);
      // Innermost first.
      REQUIRE(queries[0]->Name == css::CSSOMStringAtom::Null());
      REQUIRE(queries[1]->Name == Atom(u8"card"));
      REQUIRE(ruleSet->ContainerQueriesFor(ruleSet->ClassRules(Atom(u8"b"))->front()).empty());
    }

    SECTION("Starting style rules are marked")
    {
      auto sheet = ParseSheet(u8"@starting-style { .a {} } .b {}");
      auto ruleSet = Build(*sheet);
      REQUIRE(
        ruleSet->ClassRules(Atom(u8"a"))->front().UsedRuleTypes().Contains(UsedRuleType::StartingStyle));
      REQUIRE_FALSE(
        ruleSet->ClassRules(Atom(u8"b"))->front().UsedRuleTypes().Contains(UsedRuleType::StartingStyle));
      REQUIRE(ruleSet->Features().HasStartingStyleRules);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: nesting and scopes", "[Style][RuleSet]")
  {
    SECTION("Nested style rules are resolved against their parents")
    {
      auto sheet = ParseSheet(u8".a { color: red; .b { } & > .c { } color: blue; }");
      auto ruleSet = Build(*sheet);
      REQUIRE(Texts(ruleSet->ClassRules(Atom(u8"b"))) == List<std::string> {".a .b"});
      REQUIRE(Texts(ruleSet->ClassRules(Atom(u8"c"))) == List<std::string> {".a > .c"});
      // The parent rule and the declarations after the nested rules both use the parent's selector.
      REQUIRE(Texts(ruleSet->ClassRules(Atom(u8"a"))) == List<std::string> {".a", ".a"});
    }

    SECTION("@scope rules are recorded")
    {
      auto sheet = ParseSheet(u8"@scope (.s) to (.e) { .a {} color: red; } .b {}");
      auto ruleSet = Build(*sheet);
      REQUIRE(ruleSet->HasScopeRules());
      auto scopes = ruleSet->ScopeRulesFor(ruleSet->ClassRules(Atom(u8"a"))->front());
      REQUIRE(scopes.size() == 1uz);
      REQUIRE(ToNarrow(scopes[0]->ScopeStart().SelectorsText()) == ".s");
      REQUIRE(ToNarrow(scopes[0]->ScopeEnd().SelectorsText()) == ".e");
      REQUIRE(ruleSet->ScopeRulesFor(ruleSet->ClassRules(Atom(u8"b"))->front()).empty());
      // Declarations directly in @scope apply to the scoping root.
      REQUIRE(Texts(ruleSet->UniversalRules()) == List<std::string> {":where(:scope)"});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: features", "[Style][RuleSet][RuleFeature]")
  {
    auto sheet = ParseSheet(u8"#i .a [data-x] :hover {} .b:has(> .c) {} :has(.d) .e {} #j > p {}"
                            u8".f::first-line {} :not(.g) {} .h::first-letter {} :host(.k) p {}"
                            u8":has(:is(.m .n)) {}");
    auto ruleSet = Build(*sheet);
    auto &features = ruleSet->Features();

    REQUIRE(features.IdsInRules.contains(Atom(u8"i")));
    REQUIRE(features.IdsMatchingAncestorsInRules.contains(Atom(u8"i")));
    REQUIRE(features.IdsMatchingAncestorsInRules.contains(Atom(u8"j")));
    REQUIRE(features.AttributeLowercaseLocalNamesInRules.contains(Atom(u8"data-x")));
    REQUIRE(features.ClassRules.contains(Atom(u8"a")));
    REQUIRE(features.ClassRules.at(Atom(u8"a"))->front().MatchElement == MatchElement::Ancestor);
    REQUIRE(features.ClassRules.at(Atom(u8"e"))->front().MatchElement == MatchElement::Subject);
    REQUIRE(features.ClassRules.at(Atom(u8"c"))->front().MatchElement == MatchElement::HasChild);
    REQUIRE(features.ClassRules.at(Atom(u8"d"))->front().MatchElement == MatchElement::HasNonSubject);
    REQUIRE(features.ClassRules.at(Atom(u8"g"))->front().IsNegation == IsNegation::Yes);
    REQUIRE(features.ClassRules.at(Atom(u8"k"))->front().MatchElement == MatchElement::Host);
    REQUIRE(features.ClassesAffectingHost.contains(Atom(u8"k")));
    REQUIRE(features.AttributeRules.contains(Atom(u8"data-x")));
    REQUIRE(features.PseudoClasses.contains(css::SelectorPseudoClass::Hover));
    REQUIRE(features.PseudoClassRules.contains(
      MakePseudoClassInvalidationKey(css::SelectorPseudoClass::Hover, InvalidationKeyType::Universal)));
    REQUIRE(features.HasPseudoClassRules.contains(MakePseudoClassInvalidationKey(
      css::SelectorPseudoClass::Has, InvalidationKeyType::Class, Atom(u8"c"))));
    REQUIRE(features.UsesHasPseudoClass());
    REQUIRE(features.UsesMatchElement(MatchElement::HasScopeBreaking));
    REQUIRE(features.ScopeBreakingHasPseudoClassRules.size() == 1uz);
    REQUIRE(features.UsesFirstLineRules);
    REQUIRE(features.UsesFirstLetterRules);

    SECTION("Features add up and clear")
    {
      RuleFeatureSet combined;
      combined.Add(features);
      combined.Add(features);
      REQUIRE(combined.ClassRules.at(Atom(u8"a"))->size() == 2uz);
      REQUIRE(combined.UsesFirstLineRules);

      combined.Clear();
      REQUIRE(combined.ClassRules.empty());
      REQUIRE(combined.IdsInRules.empty());
      REQUIRE_FALSE(combined.UsesFirstLineRules);
    }

    SECTION("Selectors that differ only by a pseudo-element are collected once")
    {
      auto duplicates = ParseSheet(u8".z {} .z::before {} .z::after {}");
      auto duplicateRuleSet = Build(*duplicates);
      REQUIRE(duplicateRuleSet->Features().ClassRules.at(Atom(u8"z"))->size() == 1uz);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RuleSet: resolver mutating rules", "[Style][RuleSet]")
  {
    // Without a resolver, at-rules that change the resolver are skipped.
    auto sheet = ParseSheet(u8"@keyframes k { from {} } @view-transition { navigation: auto; }"
                            u8"@position-try --p { top: 0; } @page { margin: 0; } .a {}");
    auto ruleSet = Build(*sheet);
    REQUIRE_FALSE(ruleSet->GetViewTransitionRule());
    REQUIRE_FALSE(ruleSet->PositionTryRuleForName(Atom(u8"--p")));
    REQUIRE(ruleSet->PageRules().size() == 1uz);
    REQUIRE(ruleSet->RuleCount() == 1u);
  }
}
