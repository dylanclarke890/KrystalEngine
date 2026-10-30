#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/ElementRuleCollector.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include <string>

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using css::tests::CssText;
  using css::tests::ImmutableDeclarations;
  using Origin = PropertyCascade::Origin;
  using PropertyType = PropertyCascade::PropertyType;

  namespace
  {
    struct BlockOptions
    {
      ScopeOrdinal StyleScopeOrdinal {ScopeOrdinal::Element};
      style::CascadeLayerPriority CascadeLayerPriority {RuleSet::CascadeLayerPriorityForUnlayered};
      style::FromStyleAttribute FromStyleAttribute {style::FromStyleAttribute::No};
      uint8 LinkMatchType {css::SelectorChecker::MatchAll};
      EnumSet<UsedRuleType> UsedRuleTypes {};
      PropertyAllowlist AllowlistType {PropertyAllowlist::None};
      style::IsCacheable IsCacheable {style::IsCacheable::Yes};
    };

    KRYS_NODISCARD MatchedProperties Block(utf8_stringview css, const BlockOptions &options = {})
    {
      return MatchedProperties {.Properties = ImmutableDeclarations(css),
                                .LinkMatchType = options.LinkMatchType,
                                .AllowlistType = options.AllowlistType,
                                .StyleScopeOrdinal = options.StyleScopeOrdinal,
                                .FromStyleAttribute = options.FromStyleAttribute,
                                .CascadeLayerPriority = options.CascadeLayerPriority,
                                .UsedRuleTypes = options.UsedRuleTypes,
                                .IsCacheable = options.IsCacheable};
    }

    KRYS_NODISCARD bool Has(const PropertyCascade &cascade, PropertyId id)
    {
      return id < css::FirstLogicalGroupProperty ? cascade.HasNormalProperty(id)
                                                 : cascade.HasLogicalGroupProperty(id);
    }

    KRYS_NODISCARD const PropertyCascade::Property &Get(const PropertyCascade &cascade, PropertyId id)
    {
      return id < css::FirstLogicalGroupProperty ? cascade.NormalProperty(id)
                                                 : cascade.LogicalGroupProperty(id);
    }

    /// @brief The winning value of a property, or "" if none.
    KRYS_NODISCARD std::string Winner(const PropertyCascade &cascade, PropertyId id,
                                      uint8 linkMatchType = css::SelectorChecker::MatchDefault)
    {
      if (!Has(cascade, id))
      {
        return {};
      }
      auto *value = Get(cascade, id).CssValue[linkMatchType];
      return value ? CssText(*value) : std::string {};
    }

    KRYS_NODISCARD std::string LogicalWinner(const PropertyCascade &cascade, PropertyId id)
    {
      if (!cascade.HasLogicalGroupProperty(id))
      {
        return {};
      }
      return CssText(*cascade.LogicalGroupProperty(id).CssValue[css::SelectorChecker::MatchDefault]);
    }

    KRYS_NODISCARD std::string CustomWinner(const PropertyCascade &cascade, utf8_stringview name)
    {
      auto atom = css::CSSOMStringAtom(css::CSSOMString(name));
      if (!cascade.HasCustomProperty(atom))
      {
        return {};
      }
      return CssText(*cascade.GetCustomProperty(atom).CssValue[css::SelectorChecker::MatchDefault]);
    }

    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      document->StyleScope().FlushPendingUpdate();
      return document;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: origins and importance",
                   "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();
    result->UserAgentDeclarations.push_back(Block(u8"color: red; width: 1px; height: 1px !important"));
    result->UserDeclarations.push_back(Block(u8"color: green; width: 2px !important; margin-top: 3px"));
    result->AuthorDeclarations.push_back(Block(u8"color: blue; width: 3px; height: 3px; opacity: 0.5"));
    result->AuthorDeclarations.push_back(Block(u8"opacity: 0.25; margin-top: 4px !important"));

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
    CHECK_FALSE(cascade.IsEmpty());
    CHECK_FALSE(cascade.ApplyLowPriorityOnly());

    // Normal declarations: author over user over user agent; later over earlier.
    CHECK(Winner(cascade, PropertyId::Color) == "blue");
    CHECK(Get(cascade, PropertyId::Color).Origin == Origin::Author);
    CHECK(Winner(cascade, PropertyId::Opacity) == "0.25");

    // Important declarations: user agent over user over author, and over every normal declaration.
    CHECK(Winner(cascade, PropertyId::Height) == "1px");
    CHECK(Get(cascade, PropertyId::Height).Origin == Origin::UserAgent);
    CHECK(Winner(cascade, PropertyId::Width) == "2px");
    CHECK(Get(cascade, PropertyId::Width).Origin == Origin::User);
    CHECK(Winner(cascade, PropertyId::MarginTop) == "4px");
    CHECK(Get(cascade, PropertyId::MarginTop).Origin == Origin::Author);

    CHECK_FALSE(Has(cascade, PropertyId::Display));
    CHECK(cascade.PropertyIsPresent().test(ToUnderlying(PropertyId::Color)));
    CHECK_FALSE(cascade.PropertyIsPresent().test(ToUnderlying(PropertyId::Display)));

    SECTION("An empty match result gives an empty cascade")
    {
      auto empty = MatchResult::Create();
      PropertyCascade emptyCascade(*empty, PropertyCascade::NormalProperties());
      CHECK(emptyCascade.IsEmpty());
      CHECK(emptyCascade.LogicalGroupPropertyIds().empty());
      CHECK(emptyCascade.CustomProperties().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "PropertyCascade: important declarations across layers and scopes",
                   "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();

    SECTION("Layers: normal by position, important by reversed layer order")
    {
      result->AuthorDeclarations.push_back(
        Block(u8"color: red !important; width: 1px", {.CascadeLayerPriority = 1}));
      result->AuthorDeclarations.push_back(
        Block(u8"color: green !important; width: 2px", {.CascadeLayerPriority = 2}));
      result->AuthorDeclarations.push_back(Block(u8"color: blue !important; width: 3px"));

      PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
      CHECK(Winner(cascade, PropertyId::Width) == "3px");
      CHECK(Winner(cascade, PropertyId::Color) == "red");
      CHECK(Get(cascade, PropertyId::Color).CascadeLayerPriority == 1);
    }

    SECTION("The style attribute's important declarations win over layers")
    {
      result->AuthorDeclarations.push_back(Block(u8"color: red !important", {.CascadeLayerPriority = 1}));
      result->AuthorDeclarations.push_back(
        Block(u8"color: green !important", {.CascadeLayerPriority = RuleSet::CascadeLayerPriorityForUnlayered,
                                            .FromStyleAttribute = FromStyleAttribute::Yes}));
      result->AuthorDeclarations.push_back(Block(u8"color: blue !important", {.CascadeLayerPriority = 0}));

      PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
      CHECK(Winner(cascade, PropertyId::Color) == "green");
      CHECK(Get(cascade, PropertyId::Color).FromStyleAttribute == FromStyleAttribute::Yes);
    }

    SECTION("A later shadow tree's important declarations win")
    {
      // Match results list declarations in reverse tree context order: the slot's shadow tree first.
      result->AuthorDeclarations.push_back(
        Block(u8"color: red !important; width: 1px", {.StyleScopeOrdinal = ScopeOrdinal::FirstSlot}));
      result->AuthorDeclarations.push_back(Block(u8"color: green !important; width: 2px"));
      result->AuthorDeclarations.push_back(
        Block(u8"color: blue !important", {.StyleScopeOrdinal = ScopeOrdinal::ContainingHost}));

      PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
      CHECK(Winner(cascade, PropertyId::Width) == "2px");
      CHECK(Winner(cascade, PropertyId::Color) == "red");
      CHECK(Get(cascade, PropertyId::Color).StyleScopeOrdinal == ScopeOrdinal::FirstSlot);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: link match types", "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create(true);
    result->UserAgentDeclarations.push_back(Block(u8"color: black; width: 1px"));
    result->AuthorDeclarations.push_back(
      Block(u8"color: blue", {.LinkMatchType = css::SelectorChecker::MatchLink}));
    result->AuthorDeclarations.push_back(
      Block(u8"color: purple; width: 2px", {.LinkMatchType = css::SelectorChecker::MatchVisited}));

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());

    auto &color = Get(cascade, PropertyId::Color);
    CHECK(Winner(cascade, PropertyId::Color, css::SelectorChecker::MatchDefault) == "black");
    CHECK(Winner(cascade, PropertyId::Color, css::SelectorChecker::MatchLink) == "blue");
    CHECK(Winner(cascade, PropertyId::Color, css::SelectorChecker::MatchVisited) == "purple");
    CHECK(color.Origins[css::SelectorChecker::MatchDefault] == Origin::UserAgent);
    CHECK(color.Origins[css::SelectorChecker::MatchLink] == Origin::Author);
    CHECK(color.Origins[css::SelectorChecker::MatchVisited] == Origin::Author);
    CHECK(color.Origin == Origin::Author);

    // A property first set for one link state has no value for the others.
    auto result2 = MatchResult::Create(true);
    result2->AuthorDeclarations.push_back(
      Block(u8"color: blue", {.LinkMatchType = css::SelectorChecker::MatchLink}));
    PropertyCascade cascade2(*result2, PropertyCascade::NormalProperties());
    CHECK(Get(cascade2, PropertyId::Color).CssValue[css::SelectorChecker::MatchDefault] == nullptr);
    CHECK(Winner(cascade2, PropertyId::Color, css::SelectorChecker::MatchLink) == "blue");
    CHECK(Get(cascade2, PropertyId::Color).CssValue[css::SelectorChecker::MatchVisited] == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: logical property groups",
                   "[Style][PropertyCascade]")
  {
    const WritingMode ltr;
    const WritingMode rtl(StyleWritingMode::HorizontalTb, TextDirection::RTL, TextOrientation::Mixed);

    auto result = MatchResult::Create();
    result->AuthorDeclarations.push_back(Block(u8"margin-left: 1px; margin-inline-start: 2px"));
    result->AuthorDeclarations.push_back(Block(u8"margin-right: 3px"));
    result->AuthorDeclarations.push_back(Block(u8"margin-left: 4px; color: red"));

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
    CHECK(LogicalWinner(cascade, PropertyId::MarginLeft) == "4px");
    CHECK(LogicalWinner(cascade, PropertyId::MarginInlineStart) == "2px");
    CHECK(LogicalWinner(cascade, PropertyId::MarginRight) == "3px");
    CHECK_FALSE(cascade.HasLogicalGroupProperty(PropertyId::MarginInlineEnd));
    CHECK(Winner(cascade, PropertyId::Color) == "red");

    // In declaration order of each property's last declaration.
    auto ids = cascade.LogicalGroupPropertyIds();
    REQUIRE(ids.size() == 3);
    CHECK(ids[0] == PropertyId::MarginInlineStart);
    CHECK(ids[1] == PropertyId::MarginRight);
    CHECK(ids[2] == PropertyId::MarginLeft);

    // margin-inline-start maps to margin-left in ltr and margin-right in rtl; the later declaration wins.
    auto *winner = cascade.LastPropertyResolvingLogicalPropertyPair(PropertyId::MarginInlineStart, ltr);
    REQUIRE(winner);
    CHECK(winner->Id == PropertyId::MarginLeft);
    winner = cascade.LastPropertyResolvingLogicalPropertyPair(PropertyId::MarginInlineStart, rtl);
    REQUIRE(winner);
    CHECK(winner->Id == PropertyId::MarginRight);
    winner = cascade.LastPropertyResolvingLogicalPropertyPair(PropertyId::MarginLeft, ltr);
    REQUIRE(winner);
    CHECK(winner->Id == PropertyId::MarginLeft);

    // Neither of the pair is declared.
    CHECK(cascade.LastPropertyResolvingLogicalPropertyPair(PropertyId::PaddingLeft, ltr) == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: custom properties", "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();
    result->UserAgentDeclarations.push_back(Block(u8"--a: 1; --b: ua !important"));
    result->AuthorDeclarations.push_back(Block(u8"--a: 2; --b: author; --c: var(--a)"));

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
    CHECK(Has(cascade, PropertyId::Custom));
    CHECK(cascade.CustomProperties().size() == 3);
    CHECK(CustomWinner(cascade, u8"--a") == "2");
    CHECK(CustomWinner(cascade, u8"--b") == "ua");
    CHECK(cascade.GetCustomProperty(css::CSSOMStringAtom(u8"--b")).Origin == Origin::UserAgent);
    CHECK(CustomWinner(cascade, u8"--c") == "var(--a)");
    CHECK(cascade.GetCustomProperty(css::CSSOMStringAtom(u8"--c")).Id == PropertyId::Custom);
    CHECK_FALSE(cascade.HasCustomProperty(css::CSSOMStringAtom(u8"--d")));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: included properties",
                   "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();

    SECTION("Inherited, non-inherited and explicitly inherited properties")
    {
      result->AuthorDeclarations.push_back(Block(u8"color: red; width: 1px; height: inherit; --a: 1"));

      PropertyCascade inherited(*result, {{PropertyType::Inherited}});
      CHECK(Winner(inherited, PropertyId::Color) == "red");
      CHECK_FALSE(Has(inherited, PropertyId::Width));
      CHECK_FALSE(Has(inherited, PropertyId::Height));
      // Custom properties are inherited.
      CHECK(CustomWinner(inherited, u8"--a") == "1");

      PropertyCascade nonInherited(*result, {{PropertyType::NonInherited}});
      CHECK_FALSE(Has(nonInherited, PropertyId::Color));
      CHECK(Winner(nonInherited, PropertyId::Width) == "1px");
      CHECK(Winner(nonInherited, PropertyId::Height) == "inherit");
      CHECK_FALSE(nonInherited.HasCustomProperty(css::CSSOMStringAtom(u8"--a")));

      PropertyCascade explicitlyInherited(*result, {{PropertyType::ExplicitlyInherited}});
      CHECK_FALSE(Has(explicitlyInherited, PropertyId::Color));
      CHECK_FALSE(Has(explicitlyInherited, PropertyId::Width));
      CHECK(Winner(explicitlyInherited, PropertyId::Height) == "inherit");
    }

    SECTION("Ids")
    {
      result->AuthorDeclarations.push_back(Block(u8"color: red; width: 1px; opacity: 0.5"));

      PropertyCascade::IncludedProperties included;
      included.Ids.push_back(PropertyId::Opacity);
      PropertyCascade cascade(*result, krys::move(included));
      CHECK(cascade.ApplyLowPriorityOnly());
      CHECK(Winner(cascade, PropertyId::Opacity) == "0.5");
      CHECK_FALSE(Has(cascade, PropertyId::Color));
      CHECK_FALSE(Has(cascade, PropertyId::Width));
    }

    SECTION("Non-cacheable declarations, and the declarations that may override them")
    {
      result->AuthorDeclarations.push_back(Block(u8"opacity: 0.5", {.IsCacheable = IsCacheable::Partially}));
      result->AuthorDeclarations.push_back(Block(u8"opacity: 0.25; z-index: 3"));
      result->AuthorDeclarations.push_back(Block(u8"color: red; margin-left: 1px"));

      PropertyCascade cascade(*result, {{PropertyType::NonCacheable}});
      CHECK(Winner(cascade, PropertyId::Opacity) == "0.25");
      CHECK_FALSE(Has(cascade, PropertyId::ZIndex));
      CHECK_FALSE(Has(cascade, PropertyId::Color));
      CHECK_FALSE(cascade.HasLogicalGroupProperty(PropertyId::MarginLeft));
    }

    SECTION("Once a logical group property is set, every later one is")
    {
      result->AuthorDeclarations.push_back(
        Block(u8"margin-inline-start: 1px", {.IsCacheable = IsCacheable::Partially}));
      result->AuthorDeclarations.push_back(Block(u8"padding-left: 2px; width: 3px; opacity: 0.5"));

      PropertyCascade cascade(*result, {{PropertyType::NonCacheable}});
      CHECK(LogicalWinner(cascade, PropertyId::MarginInlineStart) == "1px");
      CHECK(LogicalWinner(cascade, PropertyId::PaddingLeft) == "2px");
      CHECK(LogicalWinner(cascade, PropertyId::Width) == "3px");
      CHECK_FALSE(Has(cascade, PropertyId::Opacity));
    }

    SECTION("Starting style")
    {
      result->AuthorDeclarations.push_back(Block(u8"width: 1px"));
      result->AuthorDeclarations.push_back(
        Block(u8"width: 2px; height: 2px", {.UsedRuleTypes = UsedRuleType::StartingStyle}));

      PropertyCascade normal(*result, PropertyCascade::NormalProperties());
      CHECK(Winner(normal, PropertyId::Width) == "1px");
      CHECK_FALSE(Has(normal, PropertyId::Height));

      PropertyCascade startingStyle(*result, {PropertyCascade::StartingStylePropertyTypes()});
      CHECK(Winner(startingStyle, PropertyId::Width) == "2px");
      CHECK(Winner(startingStyle, PropertyId::Height) == "2px");
    }

    SECTION("Allowlists")
    {
      result->AuthorDeclarations.push_back(
        Block(u8"color: red; width: 1px; --a: 1", {.AllowlistType = PropertyAllowlist::Marker}));

      PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
      CHECK(Winner(cascade, PropertyId::Color) == "red");
      CHECK_FALSE(Has(cascade, PropertyId::Width));
      CHECK(CustomWinner(cascade, u8"--a") == "1");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: rollback cascades", "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();
    result->UserAgentDeclarations.push_back(Block(u8"color: black; width: 1px"));
    result->UserDeclarations.push_back(Block(u8"color: green"));
    result->AuthorDeclarations.push_back(
      Block(u8"color: red; width: 2px; height: 1px", {.CascadeLayerPriority = 0}));
    result->AuthorDeclarations.push_back(Block(u8"color: blue; width: 3px", {.CascadeLayerPriority = 1}));
    result->AuthorDeclarations.push_back(Block(u8"color: purple; width: 4px !important"));
    result->AuthorDeclarations.push_back(
      Block(u8"color: orange", {.CascadeLayerPriority = RuleSet::CascadeLayerPriorityForUnlayered,
                                .FromStyleAttribute = FromStyleAttribute::Yes}));

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
    CHECK(Winner(cascade, PropertyId::Color) == "orange");
    CHECK(Winner(cascade, PropertyId::Width) == "4px");

    SECTION("revert in the author origin rolls back to the user origin")
    {
      PropertyCascade rollback(cascade, Origin::User);
      CHECK(Winner(rollback, PropertyId::Color) == "green");
      CHECK(Get(rollback, PropertyId::Color).Origin == Origin::User);
      CHECK(Winner(rollback, PropertyId::Width) == "1px");
      CHECK_FALSE(Has(rollback, PropertyId::Height));
    }

    SECTION("revert in the user origin rolls back to the user agent origin")
    {
      PropertyCascade rollback(cascade, Origin::UserAgent);
      CHECK(Winner(rollback, PropertyId::Color) == "black");
    }

    SECTION("revert-layer rolls back to the layers below")
    {
      // revert-layer in layer 1: layer 0 and the lower origins.
      PropertyCascade rollback(cascade, Origin::Author, ScopeOrdinal::Element, 0);
      CHECK(Winner(rollback, PropertyId::Color) == "red");
      CHECK(Winner(rollback, PropertyId::Width) == "2px");
      CHECK(Winner(rollback, PropertyId::Height) == "1px");

      // revert-layer in the unlayered rules: every layer, but not the style attribute.
      PropertyCascade unlayered(cascade, Origin::Author, ScopeOrdinal::Element,
                                RuleSet::CascadeLayerPriorityForUnlayered - 1);
      CHECK(Winner(unlayered, PropertyId::Color) == "blue");
      CHECK(Winner(unlayered, PropertyId::Width) == "3px");
    }

    SECTION("revert-layer keeps the declarations of later scopes")
    {
      auto scoped = MatchResult::Create();
      scoped->AuthorDeclarations.push_back(
        Block(u8"color: red", {.StyleScopeOrdinal = ScopeOrdinal::FirstSlot, .CascadeLayerPriority = 1}));
      scoped->AuthorDeclarations.push_back(Block(u8"color: blue", {.CascadeLayerPriority = 1}));
      PropertyCascade scopedCascade(*scoped, PropertyCascade::NormalProperties());

      PropertyCascade rollback(scopedCascade, Origin::Author, ScopeOrdinal::Element, 0);
      CHECK(Winner(rollback, PropertyId::Color) == "red");
      PropertyCascade rollbackForSlot(scopedCascade, Origin::Author, ScopeOrdinal::FirstSlot, 0);
      CHECK_FALSE(Has(rollbackForSlot, PropertyId::Color));
    }

    SECTION("Rollback cascades include every property")
    {
      PropertyCascade inheritedOnly(*result, {{PropertyType::Inherited}});
      CHECK_FALSE(Has(inheritedOnly, PropertyId::Width));
      PropertyCascade rollback(inheritedOnly, Origin::User);
      CHECK(Winner(rollback, PropertyId::Width) == "1px");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: position fallback origin",
                   "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();
    result->AuthorDeclarations.push_back(Block(u8"top: 1px; left: 2px !important; width: 3px"));
    auto fallback = ImmutableDeclarations(u8"top: 4px; left: 5px");

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties(), nullptr, fallback.get());
    CHECK(Winner(cascade, PropertyId::Top) == "4px");
    CHECK(Get(cascade, PropertyId::Top).Origin == Origin::PositionFallback);
    CHECK(Winner(cascade, PropertyId::Left) == "2px");
    CHECK(Winner(cascade, PropertyId::Width) == "3px");

    // Reverting in the position fallback origin rolls back to the author origin.
    PropertyCascade rollback(cascade, Origin::Author);
    CHECK(Winner(rollback, PropertyId::Top) == "1px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: properties applied after animations",
                   "[Style][PropertyCascade]")
  {
    auto result = MatchResult::Create();

    SECTION("Important declarations of animated properties win over animations")
    {
      result->AuthorDeclarations.push_back(Block(u8"opacity: 0.5 !important; --a: 1 !important; width: 1px"));
      result->AuthorDeclarations.push_back(Block(u8"height: 2em; margin-top: 1px; --b: 2"));
      HashSet<AnimatableCSSProperty> animated {PropertyId::Opacity, PropertyId::Width,
                                               css::CSSOMStringAtom(u8"--a")};

      PropertyCascade afterAnimation(*result, {{PropertyType::AfterAnimation}}, &animated);
      CHECK(Winner(afterAnimation, PropertyId::Opacity) == "0.5");
      CHECK(CustomWinner(afterAnimation, u8"--a") == "1");
      CHECK_FALSE(Has(afterAnimation, PropertyId::Width));
      CHECK_FALSE(Has(afterAnimation, PropertyId::Height));
      CHECK_FALSE(Has(afterAnimation, PropertyId::MarginTop));
      CHECK_FALSE(afterAnimation.HasCustomProperty(css::CSSOMStringAtom(u8"--b")));
      // Custom properties are recorded as PropertyId::Custom.
      auto overridden = afterAnimation.OverriddenAnimatedProperties();
      CHECK(overridden.size() == 2);
      CHECK(overridden.contains(PropertyId::Opacity));
      CHECK(overridden.contains(PropertyId::Custom));

      // Important declarations don't win over transitions.
      PropertyCascade afterTransition(*result, {{PropertyType::AfterTransition}}, &animated);
      CHECK_FALSE(Has(afterTransition, PropertyId::Opacity));
      CHECK_FALSE(afterTransition.HasCustomProperty(css::CSSOMStringAtom(u8"--a")));
      CHECK(afterTransition.OverriddenAnimatedProperties().empty());
    }

    SECTION("Declarations that depend on an animated font size or line height")
    {
      result->AuthorDeclarations.push_back(
        Block(u8"letter-spacing: 2em; z-index: 1; outline-offset: 2lh; opacity: 0.5"));

      HashSet<AnimatableCSSProperty> fontSize {PropertyId::FontSize};
      PropertyCascade cascade(*result, {{PropertyType::AfterAnimation}}, &fontSize);
      CHECK(Winner(cascade, PropertyId::LetterSpacing) == "2em");
      CHECK_FALSE(Has(cascade, PropertyId::ZIndex));
      // lh depends on the font size as well as the line height.
      CHECK(Winner(cascade, PropertyId::OutlineOffset) == "2lh");
      CHECK_FALSE(Has(cascade, PropertyId::Opacity));
      CHECK(cascade.OverriddenAnimatedProperties().contains(PropertyId::LetterSpacing));

      HashSet<AnimatableCSSProperty> lineHeight {PropertyId::LineHeight};
      PropertyCascade lineHeightCascade(*result, {{PropertyType::AfterAnimation}}, &lineHeight);
      CHECK(Winner(lineHeightCascade, PropertyId::OutlineOffset) == "2lh");
      CHECK_FALSE(Has(lineHeightCascade, PropertyId::LetterSpacing));
    }

    SECTION("Declarations with variable references when custom properties are animated")
    {
      result->AuthorDeclarations.push_back(
        Block(u8"padding-top: var(--b); --c: var(--d); --e: 1; z-index: 1"));

      HashSet<AnimatableCSSProperty> animated {css::CSSOMStringAtom(u8"--x")};
      PropertyCascade cascade(*result, {{PropertyType::AfterAnimation}}, &animated);
      CHECK(Winner(cascade, PropertyId::PaddingTop) == "var(--b)");
      CHECK(CustomWinner(cascade, u8"--c") == "var(--d)");
      CHECK_FALSE(cascade.HasCustomProperty(css::CSSOMStringAtom(u8"--e")));
      CHECK_FALSE(Has(cascade, PropertyId::ZIndex));
    }

    SECTION("Without animated properties nothing is overridden")
    {
      result->AuthorDeclarations.push_back(Block(u8"opacity: 0.5"));
      PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
      CHECK(cascade.OverriddenAnimatedProperties().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PropertyCascade: from matched rules",
                   "[Style][PropertyCascade]")
  {
    html::EventLoop::Current().RunUntilIdle();
    auto document = Parse(u8R"(<!DOCTYPE html><style>
      @layer base { p { color: red !important; margin-left: 1px; } }
      p { color: blue; display: inline; }
      #p { margin-inline-start: 2px; }
    </style><p id=p style="color: green; --a: 1"></p>)");
    auto element = document->GetElementById(u8"p");
    REQUIRE(element);

    ElementRuleCollector collector(*element, Scope::ForNode(*element).GetResolver().RuleSets(), nullptr);
    collector.MatchAllRules(true, false);
    auto result = collector.ReleaseMatchResult();

    PropertyCascade cascade(*result, PropertyCascade::NormalProperties());
    // The layer's important declaration wins over every normal one, including the style attribute.
    CHECK(Winner(cascade, PropertyId::Color) == "red");
    CHECK(Winner(cascade, PropertyId::Display) == "inline");
    CHECK(CustomWinner(cascade, u8"--a") == "1");
    CHECK(cascade.GetCustomProperty(css::CSSOMStringAtom(u8"--a")).FromStyleAttribute
          == FromStyleAttribute::Yes);
    // The user agent sheet's margins, then the author's.
    auto *margin = cascade.LastPropertyResolvingLogicalPropertyPair(PropertyId::MarginLeft, WritingMode());
    REQUIRE(margin);
    CHECK(margin->Id == PropertyId::MarginInlineStart);
    CHECK(CssText(*margin->CssValue[css::SelectorChecker::MatchDefault]) == "2px");
  }
}
