#include "Krystal.Booey/Style/ElementRuleCollector.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/DeclarationOrigin.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/PageRuleCollector.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Booey/Style/SelectorMatchingState.hpp"
#include <string>
#include <vector>

namespace krys::boo::style::tests
{
  using css::tests::ToNarrow;

  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      document->StyleScope().FlushPendingUpdate();
      return document;
    }

    void RunTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    KRYS_NODISCARD dom::Element &ById(dom::Document &document, dom::DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }

    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    KRYS_NODISCARD Ref<html::HTMLStyleElement> CreateStyle(dom::Document &document, dom::DOMString &&text)
    {
      auto style = CreateRef<html::HTMLStyleElement>(document);
      REQUIRE_FALSE(style->TextContent(krys::move(text)).HasException());
      return style;
    }

    KRYS_NODISCARD Ref<dom::ShadowRoot> AttachShadow(dom::Element &host, dom::DOMString &&styleText,
                                                     dom::ShadowRootMode mode = dom::ShadowRootMode::Open)
    {
      auto shadow = host.AttachShadow({.Mode = mode}).ReleaseValue();
      REQUIRE_FALSE(
        shadow->AppendChild(*CreateStyle(host.NodeDocument(), krys::move(styleText))).HasException());
      shadow->StyleScope().FlushPendingUpdate();
      return shadow;
    }

    KRYS_NODISCARD ScopeRuleSets &RuleSetsFor(dom::Element &element)
    {
      return Scope::ForNode(element).GetResolver().RuleSets();
    }

    KRYS_NODISCARD Ref<MatchResult> MatchAll(dom::Element &element,
                                             const Maybe<PseudoElementRequest> &request = {})
    {
      ElementRuleCollector collector(element, RuleSetsFor(element), nullptr);
      collector.SetPseudoElementRequest(request);
      collector.MatchAllRules(true, false);
      return collector.ReleaseMatchResult();
    }

    /// @brief The value of `--n` in each declaration block that sets it, in order.
    KRYS_NODISCARD std::vector<std::string> Names(const List<MatchedProperties> &declarations)
    {
      std::vector<std::string> names;
      for (auto &matchedProperties : declarations)
      {
        auto value = matchedProperties.Properties->GetCustomPropertyValue(u8"--n");
        if (!value.empty())
        {
          names.push_back(ToNarrow(value));
        }
      }
      return names;
    }

    KRYS_NODISCARD const MatchedProperties *Find(const List<MatchedProperties> &declarations,
                                                 utf8_stringview name)
    {
      for (auto &matchedProperties : declarations)
      {
        if (matchedProperties.Properties->GetCustomPropertyValue(u8"--n") == name)
        {
          return &matchedProperties;
        }
      }
      return nullptr;
    }

    using Strings = std::vector<std::string>;
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: declarations in cascade order",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      #b { --n: id; }
      .a { --n: class; }
      p { --n: tag; }
      * { --n: universal; }
      [data-x] { --n: attribute; }
      p.a { --n: tag-class-1; }
      p.a { --n: tag-class-2; }
      div { --n: other; }
      .a { }
    </style><p id=b class=a data-x></p>)");
    auto &p = ById(*document, u8"b");

    auto result = MatchAll(p);
    // Lower specificity first; equal specificity in document order.
    REQUIRE(Names(result->AuthorDeclarations)
            == Strings {"universal", "tag", "class", "attribute", "tag-class-1", "tag-class-2", "id"});
    // The user agent sheet styles p.
    REQUIRE_FALSE(result->UserAgentDeclarations.empty());
    REQUIRE(result->UserDeclarations.empty());
    REQUIRE_FALSE(result->IsForLink);
    REQUIRE_FALSE(result->IsCompletelyNonCacheable);

    for (auto &matchedProperties : result->AuthorDeclarations)
    {
      REQUIRE(matchedProperties.StyleScopeOrdinal == ScopeOrdinal::Element);
      REQUIRE(matchedProperties.CascadeLayerPriority == RuleSet::CascadeLayerPriorityForUnlayered);
      REQUIRE(matchedProperties.FromStyleAttribute == FromStyleAttribute::No);
      REQUIRE(matchedProperties.IsCacheable == IsCacheable::Yes);
      REQUIRE(matchedProperties.AllowlistType == PropertyAllowlist::None);
      REQUIRE(matchedProperties.LinkMatchType == css::SelectorChecker::MatchAll);
    }

    SECTION("Empty rules are skipped unless asked for")
    {
      ElementRuleCollector collector(p, RuleSetsFor(p), nullptr, css::SelectorChecker::Mode::CollectingRules);
      collector.MatchAuthorRules();
      REQUIRE(collector.MatchedRuleList().size() == 7u);

      ElementRuleCollector withEmpty(p, RuleSetsFor(p), nullptr, css::SelectorChecker::Mode::CollectingRules);
      withEmpty.SetIncludeEmptyRules(true);
      withEmpty.MatchAuthorRules();
      REQUIRE(withEmpty.MatchedRuleList().size() == 8u);
    }

    SECTION("Author rules only")
    {
      ElementRuleCollector collector(p, RuleSetsFor(p), nullptr);
      REQUIRE(collector.MatchesAnyAuthorRules());
      collector.MatchAuthorRules();
      REQUIRE(collector.GetMatchResult().UserAgentDeclarations.empty());
      REQUIRE(Names(collector.GetMatchResult().AuthorDeclarations).size() == 7u);

      auto div = Create(*document, u8"div");
      REQUIRE_FALSE(document->Body()->AppendChild(*div).HasException());
      ElementRuleCollector divCollector(*div, RuleSetsFor(*div), nullptr);
      REQUIRE(divCollector.MatchesAnyAuthorRules());

      auto span = Create(*document, u8"span");
      REQUIRE_FALSE(document->Body()->AppendChild(*span).HasException());
      // The universal rule matches every element.
      ElementRuleCollector spanCollector(*span, RuleSetsFor(*span), nullptr);
      spanCollector.MatchAuthorRules();
      REQUIRE(Names(spanCollector.GetMatchResult().AuthorDeclarations) == Strings {"universal"});
    }

    SECTION("Without author and user styles only the user agent rules match")
    {
      ElementRuleCollector collector(p, RuleSetsFor(p), nullptr);
      collector.MatchAllRules(false, false);
      REQUIRE(collector.GetMatchResult().AuthorDeclarations.empty());
      REQUIRE_FALSE(collector.GetMatchResult().UserAgentDeclarations.empty());
    }

    SECTION("A rule set of its own")
    {
      auto &ruleSets = RuleSetsFor(p).AuthorStyle();
      ElementRuleCollector collector(p, ruleSets, nullptr);
      collector.MatchAuthorRules();
      REQUIRE(Names(collector.GetMatchResult().AuthorDeclarations).size() == 7u);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: cascade layers and inline style",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      @layer one, two;
      #p { --n: unlayered; }
      @layer two { #p#p { --n: two; } }
      @layer one { #p#p#p { --n: one; } }
    </style><p id=p style="--n: inline"></p>)");
    auto &p = ById(*document, u8"p");

    auto result = MatchAll(p);
    // Layer order beats specificity; unlayered rules come after the layers and inline style after every rule.
    REQUIRE(Names(result->AuthorDeclarations) == Strings {"one", "two", "unlayered", "inline"});

    auto *one = Find(result->AuthorDeclarations, u8"one");
    auto *two = Find(result->AuthorDeclarations, u8"two");
    REQUIRE(one->CascadeLayerPriority < two->CascadeLayerPriority);
    REQUIRE(two->CascadeLayerPriority < RuleSet::CascadeLayerPriorityForUnlayered);

    auto *inlineStyle = Find(result->AuthorDeclarations, u8"inline");
    REQUIRE(inlineStyle->FromStyleAttribute == FromStyleAttribute::Yes);
    REQUIRE(inlineStyle->CascadeLayerPriority == RuleSet::CascadeLayerPriorityForUnlayered);
    REQUIRE(inlineStyle->Properties.get() == Downcast<dom::StyledElement>(p).InlineStyle());
    // Inline style parsed from the attribute is immutable, so it can be cached.
    REQUIRE(inlineStyle->IsCacheable == IsCacheable::Yes);
    REQUIRE_FALSE(result->IsCompletelyNonCacheable);

    SECTION("Mutable inline style can't be cached")
    {
      REQUIRE(
        Downcast<dom::StyledElement>(p).SetInlineStyleProperty(css::PropertyId::Color, css::ValueId::Red));
      auto mutableResult = MatchAll(p);
      REQUIRE(Find(mutableResult->AuthorDeclarations, u8"inline")->IsCacheable == IsCacheable::No);
      REQUIRE(mutableResult->IsCompletelyNonCacheable);
      REQUIRE(mutableResult->NonCacheablePropertyIds.empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: user agent rules",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();

    SECTION("Quirks mode adds the quirks sheet")
    {
      auto standards = Parse(u8"<!DOCTYPE html><table id=t></table>");
      auto quirks = Parse(u8"<table id=t></table>");
      REQUIRE(quirks->InQuirksMode());
      auto standardsResult = MatchAll(ById(*standards, u8"t"));
      auto quirksResult = MatchAll(ById(*quirks, u8"t"));
      REQUIRE(quirksResult->UserAgentDeclarations.size() > standardsResult->UserAgentDeclarations.size());
    }

    SECTION("Links")
    {
      auto document =
        Parse(u8"<!DOCTYPE html><style>:link { --n: link; }</style><a id=a href=x></a><a id=b></a>");
      auto link = MatchAll(ById(*document, u8"a"));
      REQUIRE(link->IsForLink);
      REQUIRE(Names(link->AuthorDeclarations) == Strings {"link"});
      auto *matched = Find(link->AuthorDeclarations, u8"link");
      REQUIRE(matched->LinkMatchType == css::SelectorChecker::MatchLink);

      auto anchor = MatchAll(ById(*document, u8"b"));
      REQUIRE_FALSE(anchor->IsForLink);
      REQUIRE(Names(anchor->AuthorDeclarations).empty());
    }

    SECTION("dir=auto adds a direction declaration")
    {
      auto document =
        Parse(u8"<!DOCTYPE html><style>p { --n: p; }</style><p id=auto dir=auto></p><p id=ltr dir=ltr></p>");
      auto autoResult = MatchAll(ById(*document, u8"auto"));
      // The presentational hint (unicode-bidi), then the direction, then the author rule.
      REQUIRE(autoResult->AuthorDeclarations.size() == 3u);
      auto &hints = autoResult->AuthorDeclarations[0];
      REQUIRE(hints.Properties->PropertyAsValueId(css::PropertyId::UnicodeBidi) == css::ValueId::Isolate);
      REQUIRE(hints.CascadeLayerPriority == RuleSet::CascadeLayerPriorityForPresentationalHints);
      auto &direction = autoResult->AuthorDeclarations[1];
      REQUIRE(direction.Properties->PropertyAsValueId(css::PropertyId::Direction) == css::ValueId::Ltr);
      REQUIRE(Names(autoResult->AuthorDeclarations) == Strings {"p"});

      // dir=ltr is only a presentational hint.
      auto ltrResult = MatchAll(ById(*document, u8"ltr"));
      REQUIRE(ltrResult->AuthorDeclarations.size() == 2u);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: shadow trees",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      #host { --n: document; }
      #host::part(inner) { --n: part; }
      #slotted { --n: light; }
    </style><div id=host style="--n: inline"><span id=slotted></span></div>)");
    auto &host = ById(*document, u8"host");
    auto mode = GENERATE(dom::ShadowRootMode::Open, dom::ShadowRootMode::Closed);
    auto shadow = AttachShadow(host, u8R"(
      :host { --n: host; }
      ::slotted(span) { --n: slotted; }
      span { --n: shadow; }
    )",
                               mode);
    REQUIRE(shadow->Mode() == mode);
    // Closed shadow roots are hidden from the web-exposed getter but still style their host.
    REQUIRE(static_cast<bool>(host.ShadowRoot()) == (mode == dom::ShadowRootMode::Open));
    auto slot = Create(*document, u8"slot");
    auto inner = Create(*document, u8"span");
    REQUIRE_FALSE(inner->SetAttribute(u8"part", u8"inner").HasException());
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    SECTION(":host rules")
    {
      auto result = MatchAll(host);
      // Rules from the host's shadow tree come before the host's own scope; inline style after both.
      REQUIRE(Names(result->AuthorDeclarations) == Strings {"host", "document", "inline"});
      REQUIRE(Find(result->AuthorDeclarations, u8"host")->StyleScopeOrdinal == ScopeOrdinal::Shadow);
    }

    SECTION("::slotted() rules")
    {
      auto result = MatchAll(ById(*document, u8"slotted"));
      REQUIRE(Names(result->AuthorDeclarations) == Strings {"slotted", "light"});
      REQUIRE(Find(result->AuthorDeclarations, u8"slotted")->StyleScopeOrdinal == ScopeOrdinal::FirstSlot);
    }

    SECTION("::part() rules")
    {
      auto result = MatchAll(*inner);
      // Rules from the host's scope override the shadow tree's own.
      REQUIRE(Names(result->AuthorDeclarations) == Strings {"shadow", "part"});
      REQUIRE(Find(result->AuthorDeclarations, u8"part")->StyleScopeOrdinal == ScopeOrdinal::ContainingHost);
      REQUIRE(Find(result->AuthorDeclarations, u8"shadow")->StyleScopeOrdinal == ScopeOrdinal::Element);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: @scope", "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      @scope (.outer) { p { --n: outer; } }
      @scope (.inner) { p { --n: inner; } }
      @scope (.s) to (.e) { p { --n: donut; } }
      @scope (.inner) { :scope { --n: root; } }
    </style>
    <div class="inner" id=i1><div class="outer"><p id=a></p></div></div>
    <div class="outer"><div class="inner" id=i2><p id=b></p></div></div>
    <div class="s"><p id=c></p><div class="e"><p id=d></p></div></div>
    <div><style>@scope { p { --n: implicit; } }</style><p id=e></p></div>
    <p id=f></p>)");

    // With equal specificity, the rule with the nearer scoping root wins.
    REQUIRE(Names(MatchAll(ById(*document, u8"a"))->AuthorDeclarations) == Strings {"inner", "outer"});
    REQUIRE(Names(MatchAll(ById(*document, u8"b"))->AuthorDeclarations) == Strings {"outer", "inner"});

    // Scoping limits.
    REQUIRE(Names(MatchAll(ById(*document, u8"c"))->AuthorDeclarations) == Strings {"donut"});
    REQUIRE(Names(MatchAll(ById(*document, u8"d"))->AuthorDeclarations).empty());

    // The implicit scoping root is the parent of the sheet's owner node.
    REQUIRE(Names(MatchAll(ById(*document, u8"e"))->AuthorDeclarations) == Strings {"implicit"});
    REQUIRE(Names(MatchAll(ById(*document, u8"f"))->AuthorDeclarations).empty());

    // :scope matches the scoping root.
    REQUIRE(Names(MatchAll(ById(*document, u8"i1"))->AuthorDeclarations) == Strings {"root"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: @container rules don't match yet",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    // FIXME: Container queries are unknown until there is a container query evaluator.
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      #c { container-type: inline-size; }
      @container (width >= 0px) { p { --n: container; } }
      p { --n: plain; }
    </style><div id=c><p id=p></p></div>)");
    REQUIRE(Names(MatchAll(ById(*document, u8"p"))->AuthorDeclarations) == Strings {"plain"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: pseudo-elements",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      p { --n: element; }
      p::before { --n: before; }
      p::marker { --n: marker; }
      p::highlight(one) { --n: one; }
      p::highlight(two) { --n: two; }
    </style><p id=p></p>)");
    auto &p = ById(*document, u8"p");

    SECTION("Without a request, the pseudo-elements that have rules are reported")
    {
      ElementRuleCollector collector(p, RuleSetsFor(p), nullptr);
      collector.MatchAllRules(true, false);
      REQUIRE(Names(collector.GetMatchResult().AuthorDeclarations) == Strings {"element"});
      REQUIRE(collector.MatchedPseudoElements().Contains(PseudoElementType::Before));
      REQUIRE(collector.MatchedPseudoElements().Contains(PseudoElementType::Marker));
      REQUIRE(collector.MatchedPseudoElements().Contains(PseudoElementType::Highlight));
      REQUIRE_FALSE(collector.MatchedPseudoElements().Contains(PseudoElementType::After));
    }

    SECTION("A requested pseudo-element")
    {
      auto before = MatchAll(p, PseudoElementRequest(PseudoElementType::Before));
      REQUIRE(Names(before->AuthorDeclarations) == Strings {"before"});

      auto marker = MatchAll(p, PseudoElementRequest(PseudoElementType::Marker));
      REQUIRE(Names(marker->AuthorDeclarations) == Strings {"marker"});
      REQUIRE(Find(marker->AuthorDeclarations, u8"marker")->AllowlistType == PropertyAllowlist::Marker);

      auto two =
        MatchAll(p, PseudoElementRequest(PseudoElementType::Highlight, css::CSSOMStringAtom(u8"two")));
      REQUIRE(Names(two->AuthorDeclarations) == Strings {"two"});
    }

    SECTION("Keyframes")
    {
      auto keyframes = css::Parser::ParseKeyframeRule(u8"from { color: red }", css::ParserContext(*document));
      REQUIRE(keyframes);

      ElementRuleCollector collector(p, RuleSetsFor(p), nullptr);
      collector.SetPseudoElementRequest(PseudoElementRequest(PseudoElementType::Marker));
      collector.AddAuthorKeyframeRules(*keyframes);
      auto &declarations = collector.GetMatchResult().AuthorDeclarations;
      REQUIRE(declarations.size() == 1u);
      REQUIRE(declarations.front().Properties.get() == &keyframes->Properties());
      REQUIRE(declarations.front().AllowlistType == PropertyAllowlist::Marker);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ElementRuleCollector: matching state and relations",
                   "[Style][ElementRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      .x p { --n: descendant; }
      .y p { --n: rejected; }
      p:nth-child(2) { --n: second; }
    </style><div class=x><span></span><p id=p></p></div>)");
    auto &p = ById(*document, u8"p");

    SelectorMatchingState state;
    state.SelectorFilter.PushParentInitializingIfNeeded(*p.ParentElement());
    ElementRuleCollector collector(p, RuleSetsFor(p), &state);
    collector.MatchAllRules(true, false);
    REQUIRE(Names(collector.GetMatchResult().AuthorDeclarations) == Strings {"descendant", "second"});
    REQUIRE_FALSE(collector.StyleRelations().empty());
  }

  TEST_CASE("MatchResult: cacheable properties", "[Style][MatchResult]")
  {
    auto a = css::MutableStyleProperties::Create();
    auto b = css::MutableStyleProperties::Create();

    auto first = MatchResult::Create();
    auto second = MatchResult::Create();
    REQUIRE(first->IsEmpty());
    REQUIRE(first->CacheablePropertiesEqual(*second));
    REQUIRE(DefaultHash<MatchResult>::Hash(*first) == DefaultHash<MatchResult>::Hash(*second));

    first->AuthorDeclarations.push_back({ShareRef<const css::StyleProperties>(*a)});
    REQUIRE_FALSE(first->IsEmpty());
    REQUIRE_FALSE(first->CacheablePropertiesEqual(*second));

    second->AuthorDeclarations.push_back({ShareRef<const css::StyleProperties>(*a)});
    REQUIRE(first->CacheablePropertiesEqual(*second));
    REQUIRE(DefaultHash<MatchResult>::Hash(*first) == DefaultHash<MatchResult>::Hash(*second));

    SECTION("Different declaration blocks")
    {
      second->AuthorDeclarations.back().Properties = ShareRef<const css::StyleProperties>(*b);
      REQUIRE_FALSE(first->CacheablePropertiesEqual(*second));
      REQUIRE_FALSE(first->AuthorDeclarations.back() == second->AuthorDeclarations.back());
    }

    SECTION("Partially cacheable declaration blocks compare equal")
    {
      first->AuthorDeclarations.back().IsCacheable = IsCacheable::Partially;
      second->AuthorDeclarations.back().IsCacheable = IsCacheable::Partially;
      second->AuthorDeclarations.back().Properties = ShareRef<const css::StyleProperties>(*b);
      REQUIRE(first->CacheablePropertiesEqual(*second));
      REQUIRE(DefaultHash<MatchResult>::Hash(*first) == DefaultHash<MatchResult>::Hash(*second));

      first->NonCacheablePropertyIds.push_back(css::PropertyId::Color);
      REQUIRE_FALSE(first->CacheablePropertiesEqual(*second));
      second->NonCacheablePropertyIds.push_back(css::PropertyId::Color);
      REQUIRE(first->CacheablePropertiesEqual(*second));

      // Custom properties never compare equal.
      first->NonCacheablePropertyIds.push_back(css::PropertyId::Custom);
      second->NonCacheablePropertyIds.push_back(css::PropertyId::Custom);
      REQUIRE_FALSE(first->CacheablePropertiesEqual(*second));
    }

    SECTION("User agent declarations must be identical")
    {
      first->UserAgentDeclarations.push_back({ShareRef<const css::StyleProperties>(*a)});
      REQUIRE_FALSE(first->CacheablePropertiesEqual(*second));
    }

    SECTION("Links")
    {
      auto link = MatchResult::Create(true);
      link->AuthorDeclarations.push_back({ShareRef<const css::StyleProperties>(*a)});
      REQUIRE(link->IsForLink);
      REQUIRE_FALSE(first->CacheablePropertiesEqual(*link));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PageRuleCollector: @page rules", "[Style][PageRuleCollector]")
  {
    RunTasks();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      @page :first { margin-top: 2px; }
      @page { margin-top: 1px; }
      @page :left { margin-top: 3px; }
      @page :right { margin-top: 4px; }
      @page :left { }
    </style>)");
    auto &ruleSets = document->StyleScope().GetResolver().RuleSets();

    auto margins = [&](const MatchResult &result)
    {
      Strings values;
      for (auto &matchedProperties : result.AuthorDeclarations)
      {
        values.push_back(
          ToNarrow(matchedProperties.Properties->GetPropertyValue(css::PropertyId::MarginTop)));
      }
      return values;
    };

    SECTION("Left-to-right: the first page is a right page")
    {
      PageRuleCollector first(ruleSets, WritingMode());
      first.MatchAllPageRules(0);
      // The user agent's print sheet first, then the author rules sorted by specificity: :first beats :left
      // and :right.
      REQUIRE(margins(first.GetMatchResult()) == Strings {"auto", "1px", "4px", "2px"});

      PageRuleCollector second(ruleSets, WritingMode());
      second.MatchAllPageRules(1);
      REQUIRE(margins(second.GetMatchResult()) == Strings {"auto", "1px", "3px"});
    }

    SECTION("Right-to-left: the first page is a left page")
    {
      WritingMode rightToLeft(StyleWritingMode::HorizontalTb, TextDirection::RTL, TextOrientation::Mixed);
      PageRuleCollector first(ruleSets, rightToLeft);
      first.MatchAllPageRules(0);
      REQUIRE(margins(first.GetMatchResult()) == Strings {"auto", "1px", "3px", "2px"});
    }
  }
}
