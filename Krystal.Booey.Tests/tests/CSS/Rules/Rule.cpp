#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD List<Ref<Rule>> Rules(std::initializer_list<Ref<Rule>> rules)
    {
      return List<Ref<Rule>>(rules);
    }

    KRYS_NODISCARD std::string MediaText(const mq::MediaQueryList &list)
    {
      CSSOMString result;
      mq::Serialise(result, list);
      return ToNarrow(result);
    }

    KRYS_NODISCARD cq::ContainerQuery ParseContainerQuery(utf8_stringview text)
    {
      Tokens tokens {text};
      auto range = tokens.Tokenizer.Tokens();
      auto query = cq::ContainerQueryParser::ConsumeContainerQuery(
        range, MediaQueryParserContext(StrictParserContext()));
      REQUIRE(query);
      return krys::move(*query);
    }
  }

  TEST_CASE("RuleType", "[CSS][Rules][Rule]")
  {
    // The CSSOM CSSRule.type constants.
    CHECK(static_cast<int>(RuleType::Style) == 1);
    CHECK(static_cast<int>(RuleType::Charset) == 2);
    CHECK(static_cast<int>(RuleType::Import) == 3);
    CHECK(static_cast<int>(RuleType::Media) == 4);
    CHECK(static_cast<int>(RuleType::FontFace) == 5);
    CHECK(static_cast<int>(RuleType::Page) == 6);
    CHECK(static_cast<int>(RuleType::Keyframes) == 7);
    CHECK(static_cast<int>(RuleType::Keyframe) == 8);
    CHECK(static_cast<int>(RuleType::Margin) == 9);
    CHECK(static_cast<int>(RuleType::Namespace) == 10);
    CHECK(static_cast<int>(RuleType::CounterStyle) == 11);
    CHECK(static_cast<int>(RuleType::Supports) == 12);
    CHECK(static_cast<int>(RuleType::FontFeatureValues) == 14);
    CHECK(FirstUnexposedRuleType == RuleType::ViewTransition);
    CHECK(static_cast<int>(RuleType::FunctionDeclarations) == 28);
  }

  TEST_CASE_METHOD(ValueFixture, "StyleRule", "[CSS][Rules][Rule]")
  {
    auto rule = CreateStyleRule(u8".a, .b > .c", u8"color: red");

    SECTION("Type")
    {
      CHECK(rule->Type() == RuleType::Style);
      CHECK(rule->IsStyleRule());
      CHECK_FALSE(rule->IsStyleRuleWithNesting());
      CHECK_FALSE(rule->IsGroupRule());
      CHECK(Is<StyleRule>(static_cast<Rule &>(*rule)));
      CHECK_FALSE(Is<GroupRule>(static_cast<Rule &>(*rule)));
    }

    SECTION("Selectors and properties")
    {
      CHECK(SelectorsText(rule->GetSelectorList()) == ".a, .b > .c");
      CHECK(AsText(rule->Properties()) == "color: red;");
      CHECK_FALSE(rule->Properties().IsMutable());
      CHECK_FALSE(rule->HasDocumentSecurityOrigin());
      CHECK(StyleRule::Create(ImmutableDeclarations(u8"color: red"), true, ParseSelectors(u8".a"))
              ->HasDocumentSecurityOrigin());
    }

    SECTION("MutableProperties makes the declarations mutable once")
    {
      auto &mutableProperties = rule->MutableProperties();
      CHECK(rule->Properties().IsMutable());
      CHECK(&rule->MutableProperties() == &mutableProperties);
      AddDeclaration(mutableProperties, u8"width: 1px", DeclarationTestContext());
      CHECK(AsText(rule->Properties()) == "color: red; width: 1px;");
    }

    SECTION("SetProperties and AdoptSelectorList")
    {
      rule->SetProperties(ImmutableDeclarations(u8"width: 2px"));
      CHECK(AsText(rule->Properties()) == "width: 2px;");
      rule->WrapperAdoptSelectorList(ParseSelectors(u8"#x"));
      CHECK(SelectorsText(rule->GetSelectorList()) == "#x");
    }

    SECTION("Copy is deep")
    {
      auto copy = rule->Copy();
      CHECK(copy->Type() == RuleType::Style);
      CHECK(SelectorsText(copy->GetSelectorList()) == ".a, .b > .c");
      CHECK(&copy->Properties() != &rule->Properties());
      CHECK(copy->Properties().IsMutable());
      AddDeclaration(copy->MutableProperties(), u8"width: 1px", DeclarationTestContext());
      CHECK(AsText(rule->Properties()) == "color: red;");

      Ref<Rule> base = rule;
      auto baseCopy = base->Copy();
      CHECK(baseCopy->Type() == RuleType::Style);
      CHECK(SelectorsText(Downcast<StyleRule>(*baseCopy).GetSelectorList()) == ".a, .b > .c");
    }

    SECTION("Split flags")
    {
      CHECK_FALSE(rule->IsSplitRule());
      rule->MarkAsSplitRule();
      rule->MarkAsLastRuleInSplitRule();
      CHECK(rule->IsSplitRule());
      CHECK(rule->IsLastRuleInSplitRule());
    }

    CHECK(StyleRule::AverageSizeInBytes() > sizeof(StyleRule));
  }

  TEST_CASE_METHOD(ValueFixture, "StyleRule::SplitIntoMultipleRulesWithMaximumSelectorComponentCount",
                   "[CSS][Rules][Rule]")
  {
    // 1 + 3 + 1 + 2 components.
    auto rule = CreateStyleRule(u8".a, .b .c .d, .e, .f > .g", u8"color: red");
    REQUIRE(rule->GetSelectorList().ComponentCount() == 7);

    auto rules = rule->SplitIntoMultipleRulesWithMaximumSelectorComponentCount(4);
    REQUIRE(rules.size() == 2);
    CHECK(SelectorsText(rules[0]->GetSelectorList()) == ".a, .b .c .d");
    CHECK(SelectorsText(rules[1]->GetSelectorList()) == ".e, .f > .g");
    for (auto &split : rules)
    {
      CHECK(split->IsSplitRule());
      // The declarations are shared.
      CHECK(&split->Properties() == &rule->Properties());
    }
    CHECK_FALSE(rules[0]->IsLastRuleInSplitRule());
    CHECK(rules[1]->IsLastRuleInSplitRule());

    // A complex selector longer than the maximum isn't split.
    auto single = rule->SplitIntoMultipleRulesWithMaximumSelectorComponentCount(2);
    REQUIRE(single.size() == 4);
    CHECK(SelectorsText(single[1]->GetSelectorList()) == ".b .c .d");
  }

  TEST_CASE_METHOD(ValueFixture, "StyleRuleWithNesting", "[CSS][Rules][Rule]")
  {
    auto child =
      StyleRuleWithNesting::Create(ImmutableDeclarations(u8"width: 1px"), false, ParseSelectors(u8".c"), {});
    auto declarations = NestedDeclarationsRule::Create(ImmutableDeclarations(u8"height: 1px"));
    auto rule = StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                             ParseSelectors(u8".a"), Rules({child, declarations}));

    CHECK(rule->Type() == RuleType::StyleWithNesting);
    CHECK(rule->IsStyleRule());
    CHECK(rule->IsStyleRuleWithNesting());
    CHECK(Is<StyleRule>(static_cast<Rule &>(*rule)));
    // The selectors are resolved when the rule set is built.
    CHECK(rule->GetSelectorList().IsEmpty());
    CHECK(SelectorsText(rule->OriginalSelectorList()) == ".a");
    CHECK(rule->NestedRules().size() == 2);

    CHECK(declarations->Type() == RuleType::NestedDeclarations);
    CHECK(declarations->IsStyleRule());
    CHECK(declarations->IsNestedDeclarationsRule());
    CHECK(declarations->GetSelectorList().IsEmpty());

    SECTION("Copy copies the nested rules")
    {
      auto copy = rule->Copy();
      REQUIRE(copy->NestedRules().size() == 2);
      CHECK(copy->NestedRules()[0].get() != child.get());
      CHECK(copy->NestedRules()[0]->Type() == RuleType::StyleWithNesting);
      CHECK(copy->NestedRules()[1]->Type() == RuleType::NestedDeclarations);
      CHECK(SelectorsText(copy->OriginalSelectorList()) == ".a");
    }

    SECTION("WrapperAdoptOriginalSelectorList invalidates the resolved selectors")
    {
      rule->AdoptSelectorList(ParseSelectors(u8".a"));
      child->AdoptSelectorList(ParseSelectors(u8".a .c"));
      declarations->AdoptSelectorList(ParseSelectors(u8".a"));

      rule->WrapperAdoptOriginalSelectorList(ParseSelectors(u8".b"));
      CHECK(SelectorsText(rule->OriginalSelectorList()) == ".b");
      CHECK(rule->GetSelectorList().IsEmpty());
      CHECK(child->GetSelectorList().IsEmpty());
      CHECK(declarations->GetSelectorList().IsEmpty());
      CHECK(SelectorsText(child->OriginalSelectorList()) == ".c");
    }

    SECTION("Create from a style rule")
    {
      auto styleRule = CreateStyleRule(u8".x", u8"color: blue");
      auto withNesting = StyleRuleWithNesting::Create(krys::move(*styleRule));
      CHECK(withNesting->Type() == RuleType::StyleWithNesting);
      CHECK(SelectorsText(withNesting->GetSelectorList()) == ".x");
      CHECK(SelectorsText(withNesting->OriginalSelectorList()) == ".x");
      CHECK(withNesting->NestedRules().empty());
      CHECK(AsText(withNesting->Properties()) == "color: blue;");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Group rules", "[CSS][Rules][Rule]")
  {
    SECTION("MediaRule")
    {
      auto queries =
        mq::MediaQueryParser::Parse(u8"screen and (min-width: 10px), print", StrictParserContext());
      auto rule = MediaRule::Create(krys::move(queries), Rules({CreateStyleRule(u8".a", u8"color: red")}));
      CHECK(rule->Type() == RuleType::Media);
      CHECK(rule->IsGroupRule());
      CHECK(Is<GroupRule>(static_cast<Rule &>(*rule)));
      CHECK(MediaText(rule->MediaQueries()) == "screen and (min-width: 10px), print");

      auto copy = rule->Copy();
      REQUIRE(copy->ChildRules().size() == 1);
      CHECK(copy->ChildRules()[0].get() != rule->ChildRules()[0].get());
      CHECK(MediaText(copy->MediaQueries()) == "screen and (min-width: 10px), print");

      rule->SetMediaQueries(mq::MediaQueryParser::Parse(u8"print", StrictParserContext()));
      CHECK(MediaText(rule->MediaQueries()) == "print");
      CHECK(MediaText(copy->MediaQueries()) == "screen and (min-width: 10px), print");
    }

    SECTION("WrapperInsertRule and WrapperRemoveRule")
    {
      auto rule = StartingStyleRule::Create(Rules({CreateStyleRule(u8".a", u8"color: red")}));
      rule->WrapperInsertRule(0, CreateStyleRule(u8".b", u8"color: red"));
      rule->WrapperInsertRule(2, CreateStyleRule(u8".c", u8"color: red"));
      REQUIRE(rule->ChildRules().size() == 3);
      CHECK(SelectorsText(Downcast<StyleRule>(*rule->ChildRules()[0]).GetSelectorList()) == ".b");
      CHECK(SelectorsText(Downcast<StyleRule>(*rule->ChildRules()[1]).GetSelectorList()) == ".a");
      CHECK(SelectorsText(Downcast<StyleRule>(*rule->ChildRules()[2]).GetSelectorList()) == ".c");
      rule->WrapperRemoveRule(1);
      REQUIRE(rule->ChildRules().size() == 2);
      CHECK(SelectorsText(Downcast<StyleRule>(*rule->ChildRules()[1]).GetSelectorList()) == ".c");
      CHECK(rule->Type() == RuleType::StartingStyle);
      CHECK(rule->IsStartingStyleRule());
    }

    SECTION("SupportsRule")
    {
      auto rule = SupportsRule::Create(u8"(display: grid)", true, {});
      CHECK(rule->Type() == RuleType::Supports);
      CHECK(rule->IsGroupRule());
      CHECK(rule->ConditionText() == u8"(display: grid)");
      CHECK(rule->ConditionIsSupported());
      CHECK(rule->Copy()->ConditionText() == u8"(display: grid)");
    }

    SECTION("LayerRule")
    {
      auto statement = LayerRule::CreateStatement(
        {{CSSOMStringAtom(u8"a")}, {CSSOMStringAtom(u8"b"), CSSOMStringAtom(u8"c")}});
      CHECK(statement->Type() == RuleType::LayerStatement);
      CHECK(statement->IsLayerRule());
      CHECK(statement->IsStatement());
      // A statement is a group rule type-wise only as a block.
      CHECK_FALSE(statement->IsGroupRule());
      REQUIRE(statement->NameList().size() == 2);
      CHECK(statement->NameList()[1].size() == 2);
      CHECK(statement->NameList()[1][1] == CSSOMStringAtom(u8"c"));
      CHECK(statement->ChildRules().empty());

      auto block =
        LayerRule::CreateBlock({CSSOMStringAtom(u8"a")}, Rules({CreateStyleRule(u8".a", u8"color: red")}));
      CHECK(block->Type() == RuleType::LayerBlock);
      CHECK(block->IsGroupRule());
      CHECK_FALSE(block->IsStatement());
      CHECK(block->Name() == CascadeLayerName {CSSOMStringAtom(u8"a")});
      CHECK(block->Copy()->ChildRules().size() == 1);
    }

    SECTION("ContainerRule")
    {
      auto rule = ContainerRule::Create(ParseContainerQuery(u8"card (min-width: 10px)"), {});
      CHECK(rule->Type() == RuleType::Container);
      CHECK(rule->IsContainerRule());
      CHECK(rule->IsGroupRule());
      CHECK(rule->ContainerQuery().Name == CSSOMStringAtom(u8"card"));
      CHECK(rule->Copy()->ContainerQuery().Name == CSSOMStringAtom(u8"card"));
    }

    SECTION("ScopeRule")
    {
      auto sheet = StyleSheetContents::Create();
      auto rule = ScopeRule::Create(ParseSelectors(u8".start"), ParseSelectors(u8".end"),
                                    Rules({StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"),
                                                                        false, ParseSelectors(u8".a"), {})}));
      CHECK(rule->Type() == RuleType::Scope);
      CHECK(rule->IsScopeRule());
      CHECK(rule->IsGroupRule());
      CHECK(SelectorsText(rule->OriginalScopeStart()) == ".start");
      CHECK(SelectorsText(rule->OriginalScopeEnd()) == ".end");
      CHECK(rule->ScopeStart().IsEmpty());
      CHECK(rule->ScopeEnd().IsEmpty());

      CHECK_FALSE(rule->GetStyleSheetContents());
      rule->SetStyleSheetContents(*sheet);
      CHECK(rule->GetStyleSheetContents().get() == sheet.get());

      rule->SetScopeStart(ParseSelectors(u8".start"));
      rule->SetScopeEnd(ParseSelectors(u8".end"));
      auto copy = rule->Copy();
      CHECK(SelectorsText(copy->ScopeStart()) == ".start");
      CHECK(SelectorsText(copy->OriginalScopeEnd()) == ".end");
      CHECK(copy->GetStyleSheetContents().get() == sheet.get());
      CHECK(copy->ChildRules()[0].get() != rule->ChildRules()[0].get());
    }

    SECTION("Invalidation reaches the children of group rules")
    {
      auto nested = StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                                 ParseSelectors(u8".n"), {});
      nested->AdoptSelectorList(ParseSelectors(u8".resolved"));
      auto scope = ScopeRule::Create(ParseSelectors(u8".s"), {}, Rules({nested}));
      scope->SetScopeStart(ParseSelectors(u8".s"));
      auto media =
        MediaRule::Create(mq::MediaQueryParser::Parse(u8"print", StrictParserContext()), Rules({scope}));
      auto parent = StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                                 ParseSelectors(u8".p"), Rules({media}));

      parent->WrapperAdoptOriginalSelectorList(ParseSelectors(u8".q"));
      CHECK(scope->ScopeStart().IsEmpty());
      CHECK(nested->GetSelectorList().IsEmpty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Declaration rules", "[CSS][Rules][Rule]")
  {
    SECTION("FontFaceRule")
    {
      auto rule =
        FontFaceRule::Create(ParseDescriptors(u8"font-family: foo; font-display: swap", RuleType::FontFace)
                               ->ImmutableCopyIfNeeded());
      CHECK(rule->Type() == RuleType::FontFace);
      CHECK(rule->IsFontFaceRule());
      CHECK(AsText(rule->Properties()) == "font-family: foo; font-display: swap;");
      CHECK_FALSE(rule->Properties().IsMutable());
      CHECK(rule->MutableProperties().IsMutable());

      auto copy = rule->Copy();
      CHECK(&copy->Properties() != &rule->Properties());
      CHECK(AsText(copy->Properties()) == "font-family: foo; font-display: swap;");
    }

    SECTION("PageRule")
    {
      auto rule = PageRule::Create(ImmutableDeclarations(u8"margin: 1px"), {});
      CHECK(rule->Type() == RuleType::Page);
      CHECK(rule->IsPageRule());
      CHECK(rule->Selector() == nullptr);
      CHECK_FALSE(rule->Properties().IsMutable());
      // Copy-on-write: the immutable declarations are replaced, not cast.
      AddDeclaration(rule->MutableProperties(), u8"color: red", DeclarationTestContext());
      CHECK(rule->Properties().IsMutable());
      CHECK(AsText(rule->Properties()) == "margin: 1px; color: red;");
      CHECK(AsText(rule->Copy()->Properties()) == "margin: 1px; color: red;");
    }

    SECTION("PropertyRule")
    {
      auto rule =
        PropertyRule::Create({.Name = CSSOMStringAtom(u8"--x"), .Syntax = u8"<length>", .Inherits = false});
      CHECK(rule->Type() == RuleType::Property);
      CHECK(rule->IsPropertyRule());
      CHECK(rule->GetDescriptor().Name == CSSOMStringAtom(u8"--x"));
      CHECK(rule->GetDescriptor().Syntax == u8"<length>");
      CHECK(rule->GetDescriptor().Inherits == false);
      CHECK_FALSE(rule->GetDescriptor().InitialValue);
      CHECK(rule->Copy()->GetDescriptor().Syntax == u8"<length>");
    }

    SECTION("FontPaletteValuesRule")
    {
      List<FontPaletteValues::OverriddenColor> colors;
      colors.push_back({1u, boo::Color::black});
      auto rule =
        FontPaletteValuesRule::Create(CSSOMStringAtom(u8"--p"), {CSSOMStringAtom(u8"Bixa")},
                                      FontPaletteIndex(FontPaletteIndex::Type::Dark), krys::move(colors));
      CHECK(rule->Type() == RuleType::FontPaletteValues);
      CHECK(rule->IsFontPaletteValuesRule());
      CHECK(rule->Name() == CSSOMStringAtom(u8"--p"));
      CHECK(rule->FontFamilies().size() == 1);
      CHECK(rule->BasePalette() == FontPaletteIndex(FontPaletteIndex::Type::Dark));
      REQUIRE(rule->OverrideColors().size() == 1);
      CHECK(rule->OverrideColors()[0].first == 1u);
      CHECK(rule->Copy()->FontPaletteValues() == rule->FontPaletteValues());
    }

    SECTION("FontFeatureValuesRule")
    {
      auto values = FontFeatureValues::Create();
      values->UpdateOrInsertForType(FontFeatureValuesType::Swash, {{u8"fancy", {2u}}});
      auto rule = FontFeatureValuesRule::Create({CSSOMStringAtom(u8"Font One")}, krys::move(values));
      CHECK(rule->Type() == RuleType::FontFeatureValues);
      CHECK(rule->IsFontFeatureValuesRule());
      CHECK(rule->FontFamilies().size() == 1);
      CHECK(rule->Value()->Swash().at(u8"fancy") == List<uint32> {2u});
      // The values are shared by copies.
      CHECK(rule->Copy()->Value().get() == rule->Value().get());

      auto block =
        FontFeatureValuesBlockRule::Create(FontFeatureValuesType::Styleset, {{u8"nice", {1u, 3u}}});
      CHECK(block->Type() == RuleType::FontFeatureValuesBlock);
      CHECK(block->IsFontFeatureValuesBlockRule());
      CHECK(block->FontFeatureValuesType() == FontFeatureValuesType::Styleset);
      CHECK(block->Copy()->Tags() == block->Tags());
    }

    SECTION("CharsetRule and NamespaceRule")
    {
      auto charset = CharsetRule::Create();
      CHECK(charset->Type() == RuleType::Charset);
      CHECK(charset->IsCharsetRule());
      CHECK(charset->Copy()->IsCharsetRule());

      auto rule =
        NamespaceRule::Create(CSSOMStringAtom(u8"svg"), CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));
      CHECK(rule->Type() == RuleType::Namespace);
      CHECK(rule->IsNamespaceRule());
      CHECK(rule->Prefix() == CSSOMStringAtom(u8"svg"));
      CHECK(rule->Copy()->Uri() == CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));
    }
  }
}
