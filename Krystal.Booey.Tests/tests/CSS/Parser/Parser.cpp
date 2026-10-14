#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD ParserContext SheetContext()
    {
      auto context = DeclarationTestContext();
      context.PropertySettings.crossDocumentViewTransitionsEnabled = true;
      context.PropertySettings.cssFunctionAtRuleEnabled = true;
      return context;
    }

    KRYS_NODISCARD Ref<StyleSheetContents> ParseSheet(utf8_stringview css,
                                                      const ParserContext &context = SheetContext())
    {
      auto sheet = StyleSheetContents::Create(context);
      Parser::ParseStyleSheet(css, context, *sheet);
      return sheet;
    }

    KRYS_NODISCARD List<RuleType> RuleTypes(const List<Ref<Rule>> &rules)
    {
      List<RuleType> types;
      for (const auto &rule : rules)
      {
        types.push_back(rule->Type());
      }
      return types;
    }

    KRYS_NODISCARD List<RuleType> RuleTypes(const StyleSheetContents &sheet)
    {
      List<RuleType> types;
      for (uint32 i = 0; i < sheet.RuleCount(); ++i)
      {
        types.push_back(sheet.RuleAt(i)->Type());
      }
      return types;
    }

    KRYS_NODISCARD std::string MediaText(const mq::MediaQueryList &list)
    {
      CSSOMString text;
      mq::Serialise(text, list);
      return ToNarrow(text);
    }

    KRYS_NODISCARD std::string LayerText(const CascadeLayerName &name)
    {
      CSSOMString text;
      for (const auto &part : name)
      {
        if (!text.empty())
        {
          text += u8'.';
        }
        text += part.View();
      }
      return ToNarrow(text);
    }

    KRYS_NODISCARD const StyleRule &StyleAt(const StyleSheetContents &sheet, uint32 index)
    {
      auto *rule = sheet.RuleAt(index);
      REQUIRE(rule);
      REQUIRE(rule->IsStyleRule());
      return Downcast<StyleRule>(*rule);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: style rules", "[CSS][Parser]")
  {
    auto sheet = ParseSheet(u8"a, b > c { color: red; margin: 0 }\n.x{color:blue!important;color:green}");

    REQUIRE(sheet->RuleCount() == 2);
    CHECK(sheet->HasSyntacticallyValidCSSHeader());

    auto &first = StyleAt(*sheet, 0);
    CHECK(first.Type() == RuleType::Style);
    CHECK(SelectorsText(first.GetSelectorList()) == "a, b > c");
    CHECK(AsText(first.Properties()) == "color: red; margin: 0px;");

    // The last declaration wins unless an earlier one is important.
    auto &second = StyleAt(*sheet, 1);
    CHECK(SelectorsText(second.GetSelectorList()) == ".x");
    CHECK(AsText(second.Properties()) == "color: blue !important;");
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: error recovery", "[CSS][Parser]")
  {
    SECTION("Invalid declarations are dropped")
    {
      auto sheet = ParseSheet(u8"a { color: ; foo: bar; margin: 1px; width: 10qq; padding: 2px }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(AsText(StyleAt(*sheet, 0).Properties()) == "margin: 1px; padding: 2px;");
    }

    SECTION("Invalid selectors drop the rule")
    {
      auto sheet = ParseSheet(u8"a:unknown-pseudo { color: red } b { color: blue }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(SelectorsText(StyleAt(*sheet, 0).GetSelectorList()) == "b");
      CHECK_FALSE(sheet->HasSyntacticallyValidCSSHeader());
    }

    SECTION("Unknown at-rules are skipped")
    {
      auto sheet = ParseSheet(u8"@unknown foo; @unknown { a { color: red } } b { color: blue }");
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Style});
    }

    SECTION("A rule cut off by EOF is kept")
    {
      auto sheet = ParseSheet(u8"a { color: red");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(AsText(StyleAt(*sheet, 0).Properties()) == "color: red;");
    }

    SECTION("A prelude without a block is dropped")
    {
      auto sheet = ParseSheet(u8"a { color: red } b");
      CHECK(sheet->RuleCount() == 1);
    }

    SECTION("A top-level rule that looks like a custom property is dropped")
    {
      auto sheet = ParseSheet(u8"--foo: bar { color: red } a { color: blue }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(SelectorsText(StyleAt(*sheet, 0).GetSelectorList()) == "a");
    }

    SECTION("CDO and CDC are ignored at the top level")
    {
      auto sheet = ParseSheet(u8"<!-- a { color: red } -->");
      CHECK(sheet->RuleCount() == 1);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: rule order", "[CSS][Parser]")
  {
    SECTION("@charset is accepted first and dropped")
    {
      auto sheet = ParseSheet(u8"@charset \"utf-8\"; @import \"a.css\";");
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Import});
      CHECK(sheet->HasSyntacticallyValidCSSHeader());
    }

    SECTION("@import and @namespace after a style rule are dropped")
    {
      auto sheet = ParseSheet(u8"@import \"a.css\"; a {} @import \"b.css\"; @namespace x \"y\";");
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Import, RuleType::Style});
    }

    SECTION("@import after @namespace is dropped")
    {
      auto sheet = ParseSheet(u8"@namespace x \"y\"; @import \"a.css\";");
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Namespace});
    }

    SECTION("@layer statements may come before @import")
    {
      auto sheet = ParseSheet(u8"@layer a, b.c; @import \"a.css\";");
      REQUIRE(sheet->LayerRulesBeforeImportRules().size() == 1);
      auto &names = sheet->LayerRulesBeforeImportRules()[0]->NameList();
      REQUIRE(names.size() == 2);
      CHECK(LayerText(names[0]) == "a");
      CHECK(LayerText(names[1]) == "b.c");
      CHECK(sheet->ImportRules().size() == 1);
    }

    SECTION("shouldIgnoreImportRules")
    {
      auto context = SheetContext();
      context.shouldIgnoreImportRules = true;
      auto sheet = ParseSheet(u8"@import \"a.css\"; a {}", context);
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Style});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: @import", "[CSS][Parser]")
  {
    SECTION("String, layer, supports and media")
    {
      auto sheet = ParseSheet(u8"@import url(\"x.css\") layer(foo.bar) supports(display: grid) screen;");
      REQUIRE(sheet->ImportRules().size() == 1);
      auto &rule = *sheet->ImportRules()[0];
      CHECK(ToNarrow(rule.Href()) == "x.css");
      REQUIRE(rule.GetCascadeLayerName());
      CHECK(LayerText(*rule.GetCascadeLayerName()) == "foo.bar");
      CHECK(ToNarrow(rule.SupportsText()) == "display: grid");
      CHECK(rule.SupportsMatches());
      CHECK(MediaText(rule.MediaQueries()) == "screen");
    }

    SECTION("URL token and anonymous layer")
    {
      auto sheet = ParseSheet(u8"@import url(y.css) layer;");
      REQUIRE(sheet->ImportRules().size() == 1);
      auto &rule = *sheet->ImportRules()[0];
      CHECK(ToNarrow(rule.Href()) == "y.css");
      REQUIRE(rule.GetCascadeLayerName());
      CHECK(rule.GetCascadeLayerName()->empty());
    }

    SECTION("Unsupported supports() condition")
    {
      // supports() is parsed as CSS.supports(), which is never invalid, so the rule is always kept.
      for (auto css : {u8"@import \"x.css\" supports(foo: bar);", u8"@import \"x.css\" supports(;"})
      {
        auto sheet = ParseSheet(css);
        REQUIRE(sheet->ImportRules().size() == 1);
        CHECK_FALSE(sheet->ImportRules()[0]->SupportsMatches());
      }
    }

    SECTION("Invalid")
    {
      CHECK(ParseSheet(u8"@import;")->IsEmpty());
      CHECK(ParseSheet(u8"@import foo;")->IsEmpty());
      CHECK(ParseSheet(u8"@import \"x.css\" {}")->IsEmpty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: @namespace", "[CSS][Parser]")
  {
    auto sheet =
      ParseSheet(u8"@namespace \"http://default\"; @namespace svg url(http://www.w3.org/2000/svg); "
                 u8"svg|a { color: red }");
    REQUIRE(sheet->NamespaceRules().size() == 2);
    CHECK(sheet->NamespaceRules()[0]->Prefix() == CSSOMStringAtom::Null());
    CHECK(sheet->NamespaceRules()[0]->Uri() == u8"http://default");
    CHECK(sheet->NamespaceRules()[1]->Prefix() == u8"svg");
    CHECK(sheet->DefaultNamespace() == u8"http://default");
    REQUIRE(sheet->ChildRules().size() == 1);
    CHECK(SelectorsText(Downcast<StyleRule>(*sheet->ChildRules()[0]).GetSelectorList()) == "svg|a");

    CHECK(ParseSheet(u8"@namespace a b;")->IsEmpty());
    CHECK(ParseSheet(u8"@namespace \"a\" \"b\";")->IsEmpty());
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: conditional group rules", "[CSS][Parser]")
  {
    SECTION("@media")
    {
      auto sheet =
        ParseSheet(u8"@media screen and (min-width: 10px) { a { color: red } @media print { b {} } }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &media = Downcast<MediaRule>(*sheet->RuleAt(0));
      CHECK(MediaText(media.MediaQueries()) == "screen and (min-width: 10px)");
      CHECK(RuleTypes(media.ChildRules()) == List<RuleType> {RuleType::Style, RuleType::Media});
    }

    SECTION("@supports")
    {
      auto sheet = ParseSheet(u8"@supports (display: grid) and (not (foo: bar)) { a {} } "
                              u8"@supports (foo: bar) { b {} } @supports foo { c {} }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &supported = Downcast<SupportsRule>(*sheet->RuleAt(0));
      CHECK(ToNarrow(supported.ConditionText()) == "(display: grid) and (not (foo: bar))");
      CHECK(supported.ConditionIsSupported());
      CHECK(supported.ChildRules().size() == 1);
      auto &unsupported = Downcast<SupportsRule>(*sheet->RuleAt(1));
      CHECK_FALSE(unsupported.ConditionIsSupported());
    }

    SECTION("@container")
    {
      auto sheet = ParseSheet(u8"@container sidebar (min-width: 400px) { a {} } @container { b {} }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &container = Downcast<ContainerRule>(*sheet->RuleAt(0));
      CSSOMString text;
      cq::Serialise(text, container.ContainerQuery());
      CHECK(ToNarrow(text) == "sidebar (min-width: 400px)");
      CHECK(container.ChildRules().size() == 1);
    }

    SECTION("@layer")
    {
      auto sheet = ParseSheet(u8"@layer a.b { x {} } @layer { y {} } @layer a b { } @layer c, d { }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &named = Downcast<LayerRule>(*sheet->RuleAt(0));
      CHECK_FALSE(named.IsStatement());
      CHECK(LayerText(named.Name()) == "a.b");
      CHECK(named.ChildRules().size() == 1);
      auto &anonymous = Downcast<LayerRule>(*sheet->RuleAt(1));
      CHECK(anonymous.Name().empty());
    }

    SECTION("@scope")
    {
      auto sheet = ParseSheet(u8"@scope (.a) to (.b) { c {} } @scope { d {} } @scope (.a) foo (.b) { }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &scope = Downcast<ScopeRule>(*sheet->RuleAt(0));
      CHECK(SelectorsText(scope.OriginalScopeStart()) == ".a");
      CHECK(SelectorsText(scope.OriginalScopeEnd()) == ".b");
      CHECK(scope.GetStyleSheetContents().get() == sheet.get());
      auto &implicit = Downcast<ScopeRule>(*sheet->RuleAt(1));
      CHECK(implicit.OriginalScopeStart().IsEmpty());
    }

    SECTION("@starting-style")
    {
      auto sheet = ParseSheet(u8"@starting-style { a { opacity: 0 } } @starting-style foo { }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(Downcast<StartingStyleRule>(*sheet->RuleAt(0)).ChildRules().size() == 1);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: descriptor rules", "[CSS][Parser]")
  {
    SECTION("@font-face")
    {
      auto sheet = ParseSheet(u8"@font-face { font-family: Foo; font-weight: bold; color: red } "
                              u8"@font-face { font-display: swap !important } @font-face x { }");
      REQUIRE(sheet->RuleCount() == 2);
      CHECK(AsText(Downcast<FontFaceRule>(*sheet->RuleAt(0)).Properties())
            == "font-family: Foo; font-weight: bold;");
      // !important isn't allowed in @font-face.
      CHECK(Downcast<FontFaceRule>(*sheet->RuleAt(1)).Properties().PropertyCount() == 0);
    }

    SECTION("@page")
    {
      auto sheet = ParseSheet(u8"@page :first { margin: 1px } @page { margin: 2px } @page :foo { }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &page = Downcast<PageRule>(*sheet->RuleAt(0));
      CHECK(AsText(page.Properties()) == "margin: 1px;");
      REQUIRE(page.Selector());
      CHECK(ToNarrow(page.Selector()->SelectorText()) == ":first");
    }

    SECTION("@keyframes")
    {
      auto sheet = ParseSheet(u8"@keyframes spin { from { opacity: 0 } 50%, 75% { opacity: 0.5 } "
                              u8"to { opacity: 1 } foo { } } @keyframes none { } @keyframes \"quoted\" { }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &keyframes = Downcast<KeyframesRule>(*sheet->RuleAt(0));
      CHECK(keyframes.Name() == u8"spin");
      REQUIRE(keyframes.Keyframes().size() == 3);
      CHECK(ToNarrow(keyframes.Keyframes()[1]->KeyText()) == "50%,75%");
      CHECK(AsText(keyframes.Keyframes()[1]->Properties()) == "opacity: 0.5;");
      CHECK(Downcast<KeyframesRule>(*sheet->RuleAt(1)).Name() == u8"quoted");
    }

    SECTION("@counter-style")
    {
      auto sheet = ParseSheet(u8"@counter-style thumbs { system: cyclic; symbols: \"a\"; suffix: \" \" } "
                              u8"@counter-style none { system: cyclic; symbols: \"a\" }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &counterStyle = Downcast<CounterStyleRule>(*sheet->RuleAt(0));
      CHECK(counterStyle.Name() == u8"thumbs");
      CHECK(ToNarrow(counterStyle.System()) == "cyclic");
      CHECK(ToNarrow(counterStyle.Suffix()) == "\" \"");
    }

    SECTION("@property")
    {
      auto sheet = ParseSheet(u8"@property --a { syntax: '<length>'; inherits: false; initial-value: 1px } "
                              u8"@property --b { syntax: '*'; inherits: true } "
                              u8"@property --c { syntax: '<length>'; initial-value: 1px } "
                              u8"@property --d { syntax: '<length>'; inherits: false } "
                              u8"@property --e { inherits: false } "
                              u8"@property --f { syntax: '<length>'; inherits: false; initial-value: 1em } "
                              u8"@property g { syntax: '*'; inherits: false }");
      REQUIRE(sheet->RuleCount() == 2);
      auto &length = Downcast<PropertyRule>(*sheet->RuleAt(0)).GetDescriptor();
      CHECK(length.Name == u8"--a");
      CHECK(ToNarrow(length.Syntax) == "<length>");
      CHECK(length.Inherits == false);
      REQUIRE(length.InitialValue);
      auto &universal = Downcast<PropertyRule>(*sheet->RuleAt(1)).GetDescriptor();
      CHECK(universal.Inherits == true);
      CHECK_FALSE(universal.InitialValue);
    }

    SECTION("@view-transition")
    {
      auto sheet = ParseSheet(u8"@view-transition { navigation: auto } @view-transition x { }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(Downcast<ViewTransitionRule>(*sheet->RuleAt(0)).Navigation() == ViewTransitionNavigation::Auto);

      auto context = SheetContext();
      context.PropertySettings.crossDocumentViewTransitionsEnabled = false;
      CHECK(ParseSheet(u8"@view-transition { navigation: auto }", context)->IsEmpty());
    }

    SECTION("@position-try")
    {
      auto sheet = ParseSheet(u8"@position-try --top { top: 0; --x: 1 } @position-try top { }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &positionTry = Downcast<PositionTryRule>(*sheet->RuleAt(0));
      CHECK(positionTry.Name() == u8"--top");
      // Custom properties aren't allowed in @position-try.
      CHECK(AsText(positionTry.Properties()) == "top: 0px;");
    }

    SECTION("@font-palette-values")
    {
      auto sheet = ParseSheet(u8"@font-palette-values --p { font-family: A, B; base-palette: 2; "
                              u8"override-colors: 0 red, 3 #00f } @font-palette-values p { }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &palette = Downcast<FontPaletteValuesRule>(*sheet->RuleAt(0));
      CHECK(palette.Name() == u8"--p");
      CHECK(palette.FontFamilies() == List<CSSOMStringAtom> {u8"A", u8"B"});
      REQUIRE(palette.BasePalette());
      CHECK(palette.BasePalette()->IndexType == FontPaletteIndex::Type::Integer);
      CHECK(palette.BasePalette()->Integer == 2);
      REQUIRE(palette.OverrideColors().size() == 2);
      CHECK(palette.OverrideColors()[0].first == 0);
      CHECK(palette.OverrideColors()[1].first == 3);
      CHECK(palette.OverrideColors()[1].second == boo::Color(boo::Color::blue));
    }

    SECTION("@font-feature-values")
    {
      auto sheet = ParseSheet(u8"@font-feature-values Foo, \"Bar Baz\" { @styleset { nice: 1 2 3; } "
                              u8"@swash { fancy: 1; bad: 1 2; } @character-variant { x: 1 2 } junk; } "
                              u8"@styleset { a: 1 }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &values = Downcast<FontFeatureValuesRule>(*sheet->RuleAt(0));
      CHECK(values.FontFamilies() == List<CSSOMStringAtom> {u8"Foo", u8"Bar Baz"});
      auto featureValues = values.Value();
      CHECK(featureValues->Styleset().size() == 1);
      CHECK(featureValues->Swash().size() == 1);
      CHECK(featureValues->CharacterVariant().size() == 1);
    }

    SECTION("@function")
    {
      auto sheet = ParseSheet(u8"@function --f(--a <length>: 1px, --b) returns <length> { result: var(--a); "
                              u8"@media screen { result: 2px } } "
                              u8"@function --g(--a <length>: red) { } @function --h(a) { } "
                              u8"@function --i(--a: 1 !important) { }");
      REQUIRE(sheet->RuleCount() == 1);
      auto &function = Downcast<FunctionRule>(*sheet->RuleAt(0));
      CHECK(function.Name() == u8"--f");
      REQUIRE(function.Parameters().size() == 2);
      CHECK(function.Parameters()[0].Name == u8"--a");
      CHECK(function.Parameters()[0].DefaultValue);
      CHECK(function.Parameters()[1].Name == u8"--b");
      CHECK(function.Parameters()[1].Type.IsUniversal());
      CHECK_FALSE(function.ReturnType().IsUniversal());
      CHECK(RuleTypes(function.ChildRules())
            == List<RuleType> {RuleType::FunctionDeclarations, RuleType::Media});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: nesting", "[CSS][Parser]")
  {
    SECTION("Nested style rules and trailing declarations")
    {
      auto sheet = ParseSheet(u8"a { color: red; &:hover { color: blue } b { } margin: 0 }");
      REQUIRE(sheet->RuleCount() == 1);
      REQUIRE(sheet->RuleAt(0)->IsStyleRuleWithNesting());
      auto &rule = Downcast<StyleRuleWithNesting>(*sheet->RuleAt(0));
      CHECK(AsText(rule.Properties()) == "color: red;");
      // Nested style rules are always nesting rules.
      CHECK(RuleTypes(rule.NestedRules())
            == List<RuleType> {RuleType::StyleWithNesting, RuleType::StyleWithNesting,
                               RuleType::NestedDeclarations});
      CHECK(SelectorsText(Downcast<StyleRuleWithNesting>(*rule.NestedRules()[0]).OriginalSelectorList())
            == "&:hover");
      // A nested selector without `&` is relative to the parent.
      CHECK(SelectorsText(Downcast<StyleRuleWithNesting>(*rule.NestedRules()[1]).OriginalSelectorList())
            == "& b");
      CHECK(AsText(Downcast<NestedDeclarationsRule>(*rule.NestedRules()[2]).Properties()) == "margin: 0px;");
      CHECK(sheet->HasNestingRules());
    }

    SECTION("A plain style rule isn't a nesting rule")
    {
      auto sheet = ParseSheet(u8"a { color: red }");
      CHECK_FALSE(sheet->RuleAt(0)->IsStyleRuleWithNesting());
    }

    SECTION("Nested group rules keep their declarations")
    {
      auto sheet = ParseSheet(u8"a { @media screen { color: red; b { } } @font-face { } }");
      auto &rule = Downcast<StyleRuleWithNesting>(*sheet->RuleAt(0));
      // Only group rules are allowed inside a style rule.
      REQUIRE(RuleTypes(rule.NestedRules()) == List<RuleType> {RuleType::Media});
      auto &media = Downcast<MediaRule>(*rule.NestedRules()[0]);
      CHECK(RuleTypes(media.ChildRules())
            == List<RuleType> {RuleType::NestedDeclarations, RuleType::StyleWithNesting});
    }

    SECTION("An invalid nested selector stops at the semicolon")
    {
      auto sheet = ParseSheet(u8"a { b:hover; color: red }");
      REQUIRE(sheet->RuleCount() == 1);
      CHECK(AsText(StyleAt(*sheet, 0).Properties()) == "color: red;");
    }

    SECTION("Rule list nesting is limited")
    {
      utf8_string css;
      for (int i = 0; i < 130; ++i)
      {
        css += u8"@media screen {";
      }
      css += u8"a {}";
      auto sheet = ParseSheet(css);
      REQUIRE(sheet->RuleCount() == 1);
      const Rule *rule = sheet->RuleAt(0);
      uint32 depth = 0;
      while (rule && rule->Type() == RuleType::Media)
      {
        auto &children = Downcast<MediaRule>(*rule).ChildRules();
        rule = children.empty() ? nullptr : children[0].get();
        ++depth;
      }
      // The 129th @media is kept, but its rule list is past the limit and dropped.
      CHECK(depth == 129);
      CHECK(rule == nullptr);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseStyleSheet: style-based editability", "[CSS][Parser]")
  {
    CHECK_FALSE(ParseSheet(u8"a { color: red }")->UsesStyleBasedEditability());
    CHECK(ParseSheet(u8"a { -WEBKIT-user-modify: read-write }")->UsesStyleBasedEditability());
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::ParseString", "[CSS][Parser]")
  {
    auto sheet = StyleSheetContents::Create(SheetContext());
    CHECK(sheet->ParseString(u8"@import \"a.css\"; a { color: red }"));
    CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Import, RuleType::Style});
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseRule", "[CSS][Parser]")
  {
    auto context = SheetContext();
    auto sheet = StyleSheetContents::Create(context);

    auto rule =
      Parser::ParseRule(u8"  a { color: red }  ", context, sheet.get(), Parser::AllowedRules::RegularRules);
    REQUIRE(rule);
    CHECK(rule->IsStyleRule());

    CHECK_FALSE(Parser::ParseRule(u8"  ", context, sheet.get(), Parser::AllowedRules::RegularRules));
    CHECK_FALSE(Parser::ParseRule(u8"a {} b {}", context, sheet.get(), Parser::AllowedRules::RegularRules));
    CHECK_FALSE(
      Parser::ParseRule(u8"@import \"a\";", context, sheet.get(), Parser::AllowedRules::RegularRules));
    CHECK(Parser::ParseRule(u8"@import \"a\";", context, sheet.get(), Parser::AllowedRules::ImportRules));

    SECTION("Nested context")
    {
      auto nested = Parser::ParseRule(u8"&:hover {}", context, sheet.get(),
                                      Parser::AllowedRules::RegularRules, NestedParserContextType::Style);
      REQUIRE(nested);
      CHECK(SelectorsText(Downcast<StyleRuleWithNesting>(*nested).OriginalSelectorList()) == "&:hover");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseKeyframeRule", "[CSS][Parser]")
  {
    auto context = SheetContext();
    auto keyframe = Parser::ParseKeyframeRule(u8"from, 50% { opacity: 0 }", context);
    REQUIRE(keyframe);
    CHECK(ToNarrow(keyframe->KeyText()) == "0%,50%");
    CHECK_FALSE(Parser::ParseKeyframeRule(u8"a { opacity: 0 }", context));
    // !important isn't allowed in a keyframe.
    auto important = Parser::ParseKeyframeRule(u8"from { opacity: 0 !important }", context);
    REQUIRE(important);
    CHECK(important->Properties().PropertyCount() == 0);
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParseNestedDeclarations", "[CSS][Parser]")
  {
    auto context = SheetContext();
    auto rule = Parser::ParseNestedDeclarations(context, u8"color: red; margin: 0");
    REQUIRE(rule);
    CHECK(AsText(rule->Properties()) == "color: red; margin: 0px;");
    CHECK_FALSE(Parser::ParseNestedDeclarations(context, u8"foo: bar"));
  }

  TEST_CASE_METHOD(ValueFixture, "Parser::ParsePageSelector", "[CSS][Parser]")
  {
    auto sheet = StyleSheetContents::Create(SheetContext());
    auto selectors = [&](utf8_stringview text)
    {
      Tokens tokens {text};
      return SelectorsText(Parser::ParsePageSelector(tokens.Tokenizer.Tokens(), sheet.get()));
    };

    CHECK(selectors(u8":left") == ":left");
    CHECK(selectors(u8"name") == "name");
    CHECK(selectors(u8"name:first") == "name:first");
    CHECK(SelectorsText(Parser::ParsePageSelector(TokenRange({}), sheet.get())) == "");
    CHECK(selectors(u8":hover").empty());
    CHECK(selectors(u8"a b").empty());

    // Without a sheet, the type selector is in any namespace.
    Tokens tokens {u8"name"};
    CHECK(SelectorsText(Parser::ParsePageSelector(tokens.Tokenizer.Tokens(), nullptr)) == "name");
  }

  TEST_CASE("Parser::ConsumeTrailingImportantAndWhitespace", "[CSS][Parser]")
  {
    auto consume = [](utf8_stringview text) -> Pair<bool, std::string>
    {
      Tokens tokens {text};
      auto range = tokens.Tokenizer.Tokens();
      auto important = Parser::ConsumeTrailingImportantAndWhitespace(range);
      return {static_cast<bool>(important), ToNarrow(range.Serialise(TokenSerialisationMode::Normal))};
    };

    CHECK(consume(u8"red !important  ") == Pair<bool, std::string> {true, "red"});
    CHECK(consume(u8"red ! IMPORTANT") == Pair<bool, std::string> {true, "red"});
    CHECK(consume(u8"red  ") == Pair<bool, std::string> {false, "red"});
    CHECK(consume(u8"important") == Pair<bool, std::string> {false, "important"});
    CHECK(consume(u8"red important") == Pair<bool, std::string> {false, "red important"});
  }

  TEST_CASE_METHOD(ValueFixture, "MutableStyleProperties::SetCustomProperty keeps whitespace",
                   "[CSS][Parser]")
  {
    auto context = SheetContext();
    auto properties = MutableStyleProperties::Create(ParserMode::HTMLStandard);
    CHECK(properties->SetCustomProperty(CSSOMString(u8"--x"), CSSOMString(u8"1px  2px"), context));
    CHECK(ToNarrow(properties->GetCustomPropertyValue(u8"--x")) == "1px  2px");
  }

  TEST_CASE_METHOD(ValueFixture, "MutableStyleProperties: string setters", "[CSS][Parser]")
  {
    auto context = SheetContext();
    auto properties = MutableStyleProperties::Create(ParserMode::HTMLStandard);

    SECTION("SetProperty")
    {
      bool failed = true;
      CHECK(properties->SetProperty(PropertyId::Color, CSSOMString(u8"red"), context, IsImportant(false),
                                    &failed));
      CHECK_FALSE(failed);
      CHECK(AsText(*properties) == "color: red;");

      // The fast path and the full parser both report an unchanged value.
      CHECK_FALSE(properties->SetProperty(PropertyId::Color, CSSOMString(u8"red"), context));

      CHECK(properties->SetProperty(PropertyId::Margin, CSSOMString(u8"1px 2px"), context));
      CHECK(AsText(*properties) == "color: red; margin: 1px 2px;");

      CHECK_FALSE(properties->SetProperty(PropertyId::Color, CSSOMString(u8"nope"), context,
                                          IsImportant(false), &failed));
      CHECK(failed);

      // An empty value removes the property.
      CHECK(properties->SetProperty(PropertyId::Color, CSSOMString(), context));
      CHECK(AsText(*properties) == "margin: 1px 2px;");

      CHECK(properties->SetProperty(PropertyId::Color, CSSOMString(u8"blue"), IsImportant(true)));
      CHECK(AsText(*properties) == "margin: 1px 2px; color: blue !important;");
    }

    SECTION("SetCustomProperty")
    {
      CHECK(properties->SetCustomProperty(CSSOMString(u8"--x"), CSSOMString(u8" 1px 2px "), context));
      CHECK(ToNarrow(properties->GetCustomPropertyValue(u8"--x")) == "1px 2px");
      CHECK(properties->SetCustomProperty(CSSOMString(u8"--x"), CSSOMString(), context));
      CHECK(properties->PropertyCount() == 0);
    }

    SECTION("ParseDeclaration")
    {
      CHECK(properties->ParseDeclaration(
        CSSOMString(u8"color: red; color: blue !important; color: green; --a: 1; --a: 2; foo: bar"),
        context));
      CHECK(AsText(*properties) == "--a: 2; color: blue !important;");
      CHECK_FALSE(properties->ParseDeclaration(
        CSSOMString(u8"color: red; color: blue !important; color: green; --a: 1; --a: 2"), context));
      CHECK(properties->ParseDeclaration(CSSOMString(u8""), context));
      CHECK(properties->PropertyCount() == 0);
    }
  }
}
