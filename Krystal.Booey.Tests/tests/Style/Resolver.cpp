#include "Krystal.Booey/Style/Resolver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/DeclarationOrigin.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    void RunTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    Ref<html::HTMLStyleElement> CreateStyle(dom::Document &document, dom::DOMString &&text) noexcept
    {
      auto style = CreateRef<html::HTMLStyleElement>(document);
      REQUIRE_FALSE(style->TextContent(krys::move(text)).HasException());
      return style;
    }

    KRYS_NODISCARD css::CSSOMStringAtom Atom(utf8_stringview string)
    {
      return css::CSSOMStringAtom(string);
    }

    KRYS_NODISCARD bool HasClassRule(const Resolver &resolver, utf8_stringview className)
    {
      return resolver.RuleSets().AuthorStyle().ClassRules(Atom(className)) != nullptr;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Resolver: the document's resolver", "[Style][Resolver]")
  {
    RunTasks();
    auto document =
      Parse(u8"<!DOCTYPE html><style>.a {} #b {}</style><style>@media screen { .c {} }</style>");
    auto &scope = document->StyleScope();
    scope.FlushPendingUpdate();
    REQUIRE_FALSE(scope.ResolverIfExists());

    auto &resolver = scope.GetResolver();
    REQUIRE(scope.ResolverIfExists() == &resolver);
    REQUIRE(&resolver.Document() == document.get());
    REQUIRE(resolver.GetScopeType() == Resolver::ScopeType::Document);
    REQUIRE_FALSE(resolver.IsSharedBetweenShadowTrees());
    REQUIRE(UserAgentStyle::DefaultStyle);

    auto &ruleSets = resolver.RuleSets();
    REQUIRE(ruleSets.IsAuthorStyleDefined());
    REQUIRE(ruleSets.AuthorStyle().RuleCount() == 2u);
    // Without a view, only the media type of the evaluator is unknown, so `screen` doesn't match.
    REQUIRE_FALSE(HasClassRule(resolver, u8"c"));
    REQUIRE(ruleSets.StyleForDeclarationOrigin(DeclarationOrigin::Author) == &ruleSets.AuthorStyle());
    REQUIRE_FALSE(ruleSets.StyleForDeclarationOrigin(DeclarationOrigin::User));
    REQUIRE(ruleSets.StyleForDeclarationOrigin(DeclarationOrigin::UserAgent)
            == ruleSets.UserAgentMediaQueryStyle());

    SECTION("Selectors in the rules are looked up")
    {
      REQUIRE(resolver.HasSelectorForId(Atom(u8"b")));
      REQUIRE_FALSE(resolver.HasSelectorForId(Atom(u8"a")));
      // The features include the default style's.
      REQUIRE(ruleSets.Features().PseudoClasses.contains(css::SelectorPseudoClass::FocusVisible));
      REQUIRE(ruleSets.Features().ClassRules.contains(Atom(u8"a")));
      REQUIRE_FALSE(ruleSets.HasScopeRules());
      REQUIRE_FALSE(ruleSets.HasContainerQueries());
    }

    SECTION("Sheets added at the end are appended to the resolver")
    {
      REQUIRE_FALSE(document->Body()->AppendChild(*CreateStyle(*document, u8".d {}")).HasException());
      scope.FlushPendingUpdate();
      REQUIRE(scope.ResolverIfExists() == &resolver);
      REQUIRE(HasClassRule(resolver, u8"d"));
      REQUIRE(ruleSets.AuthorStyle().RuleCount() == 3u);
    }

    SECTION("Sheets inserted before others reset the author style")
    {
      RefPtr<dom::Element> head = document->DocumentElement()->FirstElementChild();
      REQUIRE_FALSE(
        head->InsertBefore(*CreateStyle(*document, u8".e {}"), head->FirstChild()).HasException());
      scope.FlushPendingUpdate();
      REQUIRE(scope.ResolverIfExists() == &resolver);
      REQUIRE(HasClassRule(resolver, u8"e"));
      REQUIRE(resolver.RuleSets().AuthorStyle().RuleCount() == 3u);
      // The inserted rule comes first.
      REQUIRE(resolver.RuleSets().AuthorStyle().ClassRules(Atom(u8"e"))->front().Position() == 0u);
    }

    SECTION("Removing a sheet reconstructs the resolver")
    {
      RefPtr<dom::Element> head = document->DocumentElement()->FirstElementChild();
      REQUIRE_FALSE(head->RemoveChild(*head->FirstChild()).HasException());
      scope.FlushPendingUpdate();
      REQUIRE_FALSE(scope.ResolverIfExists());
      REQUIRE_FALSE(HasClassRule(scope.GetResolver(), u8"a"));
    }

    SECTION("Changing a sheet's contents clears the resolver")
    {
      scope.DidChangeStyleSheetContents();
      REQUIRE_FALSE(scope.ResolverIfExists());
    }

    SECTION("Releasing memory clears the resolver")
    {
      scope.ReleaseMemory();
      REQUIRE_FALSE(scope.ResolverIfExists());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Resolver: at-rules that change the resolver",
                   "[Style][Resolver]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><style>"
                          u8"@counter-style thumbs { system: cyclic; symbols: x y; }"
                          u8"@keyframes spin { from {} to {} }"
                          u8"@font-face { font-family: f; src: local(f); }"
                          u8"</style>");
    auto &scope = document->StyleScope();
    scope.FlushPendingUpdate();
    auto &resolver = scope.GetResolver();

    REQUIRE(ToNarrow(scope.GetCounterStyleRegistry().ResolvedCounterStyle(Atom(u8"thumbs"))->Text(2, {}))
            == "y");
    REQUIRE(resolver.IsAnimationNameValid(u8"spin"));
    REQUIRE_FALSE(resolver.IsAnimationNameValid(u8"nope"));
    REQUIRE_FALSE(resolver.GetViewTransitionRule());

    SECTION("@view-transition and @position-try")
    {
      // FIXME: Author sheets can't have @view-transition and @position-try rules until parser contexts have
      // settings (crossDocumentViewTransitionsEnabled and cssAnchorPositioningEnabled are on by default).
      css::ParserContext context(css::ParserMode::HTMLStandard);
      context.PropertySettings.crossDocumentViewTransitionsEnabled = true;
      context.PropertySettings.cssAnchorPositioningEnabled = true;
      auto sheet = css::StyleSheetContents::Create(context);
      sheet->ParseString(u8"@view-transition { navigation: auto; } @position-try --p { top: 0; }"
                         u8"@layer a { @position-try --q { top: 0; } } @position-try --p { top: 1px; }");
      REQUIRE(sheet->ChildRules().size() == 4uz);
      auto ruleSet = RuleSet::Create();
      {
        RuleSetBuilder builder(*ruleSet, resolver.MediaQueryEvaluator(), &resolver);
        builder.AddRulesFromSheet(*sheet);
      }
      REQUIRE(ruleSet->GetViewTransitionRule());
      REQUIRE(ruleSet->RuleCount() == 0u);
      REQUIRE(ruleSet->PositionTryRuleForName(Atom(u8"--q")));
      // The last rule with a name wins.
      auto positionTry = ruleSet->PositionTryRuleForName(Atom(u8"--p"));
      REQUIRE(positionTry);
      REQUIRE(positionTry.get() == sheet->ChildRules().back().get());
    }

    SECTION("Clearing the resolver clears the author counter styles")
    {
      scope.ClearResolver();
      REQUIRE_FALSE(scope.GetCounterStyleRegistry().HasAuthorCounterStyles());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Resolver: shadow trees share resolvers", "[Style][Resolver]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><div id=one></div><div id=two></div><div id=three></div>");
    auto attach = [&](utf8_stringview id, dom::DOMString &&text)
    {
      auto host = document->GetElementById(id);
      auto shadow = host->AttachShadow({}).ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*CreateStyle(*document, krys::move(text))).HasException());
      shadow->StyleScope().FlushPendingUpdate();
      return shadow;
    };
    auto one = attach(u8"one", u8".s {} @counter-style shadow { system: cyclic; symbols: z; }");
    auto two = attach(u8"two", u8".s {} @counter-style shadow { system: cyclic; symbols: z; }");
    auto three = attach(u8"three", u8".t {}");

    auto &first = one->StyleScope().GetResolver();
    REQUIRE(first.GetScopeType() == Resolver::ScopeType::ShadowTree);
    REQUIRE(HasClassRule(first, u8"s"));
    REQUIRE_FALSE(first.IsSharedBetweenShadowTrees());

    auto &second = two->StyleScope().GetResolver();
    REQUIRE(&second == &first);
    REQUIRE(first.IsSharedBetweenShadowTrees());

    auto &third = three->StyleScope().GetResolver();
    REQUIRE(&third != &first);
    REQUIRE(HasClassRule(third, u8"t"));

    // Shadow trees can't add counter styles.
    REQUIRE_FALSE(document->StyleScope().GetCounterStyleRegistry().HasAuthorCounterStyles());
    REQUIRE_FALSE(one->StyleScope().GetCounterStyleRegistry().HasAuthorCounterStyles());

    SECTION("A changed shadow tree gets its own resolver")
    {
      REQUIRE_FALSE(two->AppendChild(*CreateStyle(*document, u8".u {}")).HasException());
      two->StyleScope().FlushPendingUpdate();
      REQUIRE_FALSE(two->StyleScope().ResolverIfExists());
      auto &changed = two->StyleScope().GetResolver();
      REQUIRE(&changed != &first);
      REQUIRE(HasClassRule(changed, u8"u"));
      REQUIRE(&one->StyleScope().GetResolver() == &first);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Resolver: dynamic media queries", "[Style][Resolver]")
  {
    RunTasks();
    css::mq::MediaEnvironment environment;
    environment.LayoutSize = IntSize {800, 600};
    css::mq::SetMediaEnvironment(environment);

    auto document = Parse(u8"<!DOCTYPE html><style>@media (min-width: 1000px) { .wide {} } .always {}</style>"
                          u8"<style>@media screen { .screen {} }</style>");
    auto &scope = document->StyleScope();
    scope.FlushPendingUpdate();
    auto &resolver = scope.GetResolver();

    REQUIRE(resolver.HasViewportDependentMediaQueries());
    REQUIRE(HasClassRule(resolver, u8"screen"));
    // Rules under queries that can change are added, and enabled while the query matches.
    auto &wide = resolver.RuleSets().AuthorStyle().ClassRules(Atom(u8"wide"))->front();
    REQUIRE_FALSE(wide.IsEnabled());
    REQUIRE_FALSE(resolver.EvaluateDynamicMediaQueries());

    environment.LayoutSize = IntSize {1'200, 600};
    css::mq::SetMediaEnvironment(environment);
    auto changes = resolver.EvaluateDynamicMediaQueries();
    REQUIRE(changes);
    REQUIRE(changes->ChangeType == DynamicMediaQueryEvaluationChanges::Type::InvalidateStyle);
    REQUIRE(changes->InvalidationRuleSets.size() == 1uz);
    REQUIRE(changes->InvalidationRuleSets[0].RuleSet->ClassRules(Atom(u8"wide")));
    REQUIRE(wide.IsEnabled());

    // The invalidation rule set is reused for the same change.
    environment.LayoutSize = IntSize {800, 600};
    css::mq::SetMediaEnvironment(environment);
    auto back = resolver.EvaluateDynamicMediaQueries();
    REQUIRE(back);
    REQUIRE(back->InvalidationRuleSets[0].RuleSet == changes->InvalidationRuleSets[0].RuleSet);
    REQUIRE_FALSE(wide.IsEnabled());

    css::mq::SetMediaEnvironment(null);
  }
}
