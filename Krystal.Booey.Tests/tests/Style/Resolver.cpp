#include "Krystal.Booey/Style/Resolver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
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
      css::ParserContext context(css::ParserMode::HTMLStandard);
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

  namespace
  {
    struct ElementStyleFixture : ResolverFixture
    {
      ElementStyleFixture() noexcept
          : ResolverFixture(u8R"(<!DOCTYPE html><style>
              html { font-size: 20px }
              .a { width: 2rem; opacity: 0.5 }
              #b { margin-top: 1em }
              div:first-child { height: 3px }
              p::before { content: "x"; width: 3px }
              #anchor::before { content: attr(data-x) }
              @page { margin-top: 7px }
              @page :first { margin-left: 9px }
            </style><body><div id=a class=a></div><div id=b class=a style="height: 4px"></div>
            <a id=link href="x"></a><a id=self href=""></a><a id=anchor></a><p id=p></p>
            <svg><a id=svglink xlink:href="x"></a><svg id=inner></svg></svg>)")
      {
      }
    };
  }

  TEST_CASE_METHOD(ElementStyleFixture, "Resolver: element styles", "[Style][Resolver]")
  {
    SECTION("The document element inherits from the initial containing block")
    {
      auto resolved = Resolve(*Document->DocumentElement());
      REQUIRE(resolved.Style);
      CHECK(resolved.Style->ComputedFontSize() == 20);
      CHECK(resolved.Style->Display() == DisplayType::Block);
      CHECK(resolved.MatchResult);
    }

    SECTION("Rules from every origin, inline style, root-relative units and inheritance")
    {
      auto a = Resolve(ById(u8"a"));
      CHECK(Serialised(*a.Style, a.Style->Width()) == "40px");
      CHECK(a.Style->GetOpacity() == Opacity {0.5f});
      CHECK(a.Style->Display() == DisplayType::Block);
      CHECK(a.Style->ComputedFontSize() == 20);
      CHECK(Serialised(*a.Style, a.Style->Height()) == "3px");

      auto b = Resolve(ById(u8"b"));
      CHECK(Serialised(*b.Style, b.Style->MarginTop()) == "20px");
      // The inline style wins over the author rule.
      CHECK(Serialised(*b.Style, b.Style->Height()) == "4px");
      REQUIRE(b.MatchResult);
      CHECK(b.MatchResult->AuthorDeclarations.size() >= 3u);

      auto body = Resolve(*Document->Body());
      CHECK(Serialised(*body.Style, body.Style->MarginTop()) == "8px");
    }

    SECTION("Only user agent rules")
    {
      auto a = Resolve(ById(u8"a"), RuleMatchingBehavior::MatchOnlyUserAgentRules);
      CHECK(Serialised(*a.Style, a.Style->Width()) == "auto");
      CHECK(a.Style->Display() == DisplayType::Block);
    }

    SECTION("Relations")
    {
      // `:first-child` matched: the element's own style records it.
      auto a = Resolve(ById(u8"a"));
      CHECK(a.Style->FirstChildState());
      REQUIRE(a.Relations);
      CHECK(std::ranges::none_of(*a.Relations, [](const Relation &relation)
                                 { return relation.RelationType == Relation::FirstChild; }));

      // Failing `:first-child` still makes the parent's children depend on it.
      auto b = Resolve(ById(u8"b"));
      CHECK_FALSE(b.Style->FirstChildState());
      REQUIRE(b.Relations);
      CHECK(std::ranges::any_of(*b.Relations,
                                [&](const Relation &relation)
                                {
                                  return relation.RelationType == Relation::ChildrenAffectedByFirstChildRules
                                         && relation.Element == Document->Body().get();
                                }));
    }

    SECTION("Links")
    {
      auto link = Resolve(ById(u8"link"));
      CHECK(link.Style->IsLink());
      CHECK(link.Style->GetInsideLink() == InsideLink::InsideUnvisited);

      // A link to the document itself is visited.
      auto self = Resolve(ById(u8"self"));
      CHECK(self.Style->GetInsideLink() == InsideLink::InsideVisited);

      auto anchor = Resolve(ById(u8"anchor"));
      CHECK_FALSE(anchor.Style->IsLink());
      CHECK(anchor.Style->GetInsideLink() == InsideLink::NotInside);

      auto svgLink = Resolve(ById(u8"svglink"));
      CHECK(svgLink.Style->GetInsideLink() == InsideLink::InsideUnvisited);
    }

    SECTION("SVG zoom rules")
    {
      CHECK(Resolve(ById(u8"svglink")).Style->UseSVGZoomRulesForLength());
      // Only an `<svg>` without a parent uses them.
      CHECK_FALSE(Resolve(ById(u8"inner")).Style->UseSVGZoomRulesForLength());
      CHECK_FALSE(Resolve(ById(u8"a")).Style->UseSVGZoomRulesForLength());
    }

    SECTION("Pseudo-elements")
    {
      auto p = Resolve(ById(u8"p"));
      CHECK(p.Style->HasPseudoStyle(PseudoElementType::Before));
      CHECK_FALSE(p.Style->HasPseudoStyle(PseudoElementType::After));

      auto before = GetResolver().StyleForPseudoElement(
        ById(u8"p"), PseudoElementRequest {PseudoElementType::Before}, {.ParentStyle = p.Style.get()});
      REQUIRE(before);
      CHECK(Serialised(*before->Style, before->Style->Width()) == "3px");
      REQUIRE(before->Style->GetPseudoElementIdentifier());
      CHECK(before->Style->GetPseudoElementIdentifier()->Type == PseudoElementType::Before);

      // No rules match ::after (the user agent's ::after rules need `content`).
      auto after = GetResolver().StyleForPseudoElement(
        ById(u8"p"), PseudoElementRequest {PseudoElementType::After}, {.ParentStyle = p.Style.get()});
      CHECK_FALSE(after);
    }

    SECTION("Pages")
    {
      // Without a resolved document element style there's no writing mode to resolve pages against.
      auto unresolved = GetResolver().StyleForPage(0);
      CHECK(Serialised(*unresolved, unresolved->MarginTop()) == "0px");

      Document->UpdateStyleIfNeeded();
      auto first = GetResolver().StyleForPage(0);
      CHECK(Serialised(*first, first->MarginTop()) == "7px");
      CHECK(Serialised(*first, first->MarginLeft()) == "9px");
      // Inherited from the initial containing block.
      CHECK(first->ComputedFontSize() == 16);

      auto second = GetResolver().StyleForPage(1);
      CHECK(Serialised(*second, second->MarginTop()) == "7px");
      // From the user agent's @page rule.
      CHECK(Serialised(*second, second->MarginLeft()) == "auto");
    }

    SECTION("Matched rules")
    {
      auto &a = ById(u8"a");
      auto author = GetResolver().StyleRulesForElement(&a, Resolver::AuthorCSSRules);
      CHECK(author.size() == 2u);
      auto all = GetResolver().StyleRulesForElement(&a);
      CHECK(all.size() > author.size());
      CHECK(GetResolver().StyleRulesForElement(nullptr).empty());

      auto before = GetResolver().PseudoStyleRulesForElement(
        &ById(u8"p"), PseudoElementRequest {PseudoElementType::Before}, Resolver::AuthorCSSRules);
      CHECK(before.size() == 1u);
    }

    SECTION("The root default style only has user agent rules")
    {
      // The resolver was created after the document was parsed.
      Document->StyleScope().ClearResolver();
      auto *root = GetResolver().RootDefaultStyle();
      REQUIRE(root);
      CHECK(root->ComputedFontSize() == 16);
      CHECK(root->Display() == DisplayType::Block);
    }

    SECTION("Viewport units are recorded on the document")
    {
      CHECK_FALSE(Document->HasStyleWithViewportUnits());
      REQUIRE_FALSE(ById(u8"a").SetAttribute(u8"style", u8"width: 10vw").HasException());
      (void)Resolve(ById(u8"a"));
      CHECK(Document->HasStyleWithViewportUnits());
    }

    SECTION("Content attributes of ::before and ::after are registered as rule features")
    {
      auto &anchor = ById(u8"anchor");
      auto anchorStyle = Resolve(anchor);
      CHECK_FALSE(GetResolver().RuleSets().Features().ContentAttributeNamesInRules.contains(
        css::CSSOMStringAtom(u8"data-x")));
      auto before = GetResolver().StyleForPseudoElement(
        anchor, PseudoElementRequest {PseudoElementType::Before}, {.ParentStyle = anchorStyle.Style.get()});
      REQUIRE(before);
      CHECK(GetResolver().RuleSets().Features().ContentAttributeNamesInRules.contains(
        css::CSSOMStringAtom(u8"data-x")));
    }
  }

  TEST_CASE_METHOD(ElementStyleFixture, "Resolver: view transition styles", "[Style][Resolver]")
  {
    auto &resolver = GetResolver();
    auto *dynamicStyle = resolver.RuleSets().DynamicViewTransitionsStyle();
    REQUIRE(dynamicStyle);
    auto before = dynamicStyle->RuleCount();

    auto properties = css::MutableStyleProperties::Create(css::ParserMode::HTMLStandard);
    REQUIRE(properties->SetProperty(css::PropertyId::Width, u8"5px"));
    resolver.SetViewTransitionStyles(css::SelectorPseudoElement::ViewTransitionGroup,
                                     css::CSSOMStringAtom(u8"x"), properties);
    CHECK(dynamicStyle->RuleCount() == before + 1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Resolver: random() base values", "[Style][Resolver]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html><div id=a></div><div id=b></div>");
    auto a = document->GetElementById(u8"a");
    auto b = document->GetElementById(u8"b");
    css::calc::RandomCachingKey key {
      Variant<css::calc::Random::SharingOptions::Auto, css::CSSOMStringAtom> {css::CSSOMStringAtom(u8"x")}};

    CHECK_FALSE(a->HasRandomCachingKeyMap());
    auto value = a->LookupCSSRandomBaseValue({}, key);
    CHECK(a->HasRandomCachingKeyMap());
    CHECK(value >= 0.0);
    CHECK(value < 1.0);
    // Values are cached per element and pseudo-element.
    CHECK(a->LookupCSSRandomBaseValue({}, key) == value);
    CHECK_FALSE(b->HasRandomCachingKeyMap());

    auto shared = document->LookupCSSRandomBaseValue(key);
    CHECK(document->LookupCSSRandomBaseValue(key) == shared);
  }
}
