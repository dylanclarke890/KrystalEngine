#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/TestParserUtils.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    struct CSSSelectorParserTest
    {
      utf8_string Input;
      utf8_string Output;
      bool ExpectInvalid {false};
    };

    KRYS_NODISCARD List<CSSSelectorParserTest> ParseCSSSelectorParserTest(std::istream &stream) noexcept
    {
      List<CSSSelectorParserTest> tests;
      CSSSelectorParserTest currentTest;

      Maybe<utf8_string> output;

      auto finishTest = [&]()
      {
        if (!currentTest.Input.empty())
        {
          if (output.has_value())
          {
            krys::boo::tests::NormaliseData(*output);
            currentTest.Output = krys::move(*output);
            output.reset();
          }
          else if (!currentTest.ExpectInvalid)
          {
            // If no output is specified for a valid test, assume the output is the same as the input.
            currentTest.Output = currentTest.Input;
          }
          tests.push_back(krys::move(currentTest));
          currentTest = {};
        }
      };

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"input")
        {
          finishTest();
          krys::boo::tests::NormaliseData(data);
          currentTest.Input = krys::move(data);
        }
        else if (sectionName == u8"output")
        {
          krys::boo::tests::NormaliseData(data);
          output = krys::move(data);
        }
        else if (sectionName == u8"expect-invalid")
        {
          currentTest.ExpectInvalid = true;
        }
      };

      krys::boo::tests::ParseTestData(stream, "$", ::krys::move(parse));
      finishTest();

      return tests;
    }

    void ExecuteCSSSelectorParserTest(const CSSSelectorParserTest &test, size_t number, size_t total) noexcept
    {
      auto result = SelectorParser::ParseSelectorList(test.Input, {});

      utf8_string str = u8"--- TEST " + krys::boo::tests::ToUTF8(number + 1uz) + u8" OF "
                        + krys::boo::tests::ToUTF8(total) + u8" ---\n";
      UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + test.Input;
      UTF8_INFO(str);

      if (test.ExpectInvalid)
      {
        str = u8"\n--- EXPECTED OUTPUT ---\n" + krys::boo::tests::ToUTF8("Invalid selector");
        bool isInvalid = !result.has_value();
        CHECK(isInvalid);
      }
      else
      {
        str = u8"--- EXPECTED OUTPUT ---\n" + test.Output;
        UTF8_INFO(str);

        auto output = result ? result->SelectorsText() : u8"";
        str = u8"\n--- ACTUAL OUTPUT ---\n" + output;
        UTF8_INFO(str);

        bool equal = output == test.Output;
        CHECK(equal);
      }
    }

    void RunTest(string filename) noexcept
    {
      static string basedir = KRYS_BOOEY_TESTS_DATA_DIR "/css-selector-parser/";

      auto file = krys::boo::tests::OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseCSSSelectorParserTest(*file);
      REQUIRE(!tests.empty());

      krys::boo::tests::ExecuteTests(tests, ExecuteCSSSelectorParserTest);
    }
  }

#define EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE(FileName)                                                      \
  TEST_CASE("CSSSelectorParser(" FileName ")", "[CSS][CSSSelectorParser]")                                   \
  {                                                                                                          \
    RunTest(FileName);                                                                                       \
  }

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("an-plus-b-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("an-plus-b-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("attribute-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("child-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("class-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("descendant-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("focus-visible-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("has-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("has-02.dat");

  // FIXME: :heading and :heading() (Selectors 5) aren't supported.
  TEST_CASE("CSSSelectorParser(heading-01.dat)", "[CSS][CSSSelectorParser][!shouldfail]")
  {
    RunTest("heading-01.dat");
  }
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("heading-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("id-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("not-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("not-02.dat");

  // FIXME: A complex selector inside :not() inside :host() should be invalid (WPT parse-not.html).
  TEST_CASE("CSSSelectorParser(not-03.dat)", "[CSS][CSSSelectorParser][!shouldfail]")
  {
    RunTest("not-03.dat");
  }

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("part-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("part-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("sibling-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("slotted-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("slotted-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("state-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("state-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("universal-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("where-01.dat");
}

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD CSSOMString Parse(CSSOMStringView input, const SelectorParserContext &context = {},
                                     StyleSheetContents *styleSheet = nullptr,
                                     NestedContext nested = {}) noexcept
    {
      auto list = SelectorParser::ParseSelectorList(input, context, styleSheet, nested);
      return list ? list->SelectorsText() : CSSOMString(u8"<invalid>");
    }

    KRYS_NODISCARD SelectorList ParseValid(CSSOMStringView input, const SelectorParserContext &context = {},
                                           StyleSheetContents *styleSheet = nullptr,
                                           NestedContext nested = {}) noexcept
    {
      auto list = SelectorParser::ParseSelectorList(input, context, styleSheet, nested);
      REQUIRE(list);
      return krys::move(*list);
    }

    KRYS_NODISCARD bool Supports(CSSOMStringView input) noexcept
    {
      InputStream inputStream {CSSOMString(input)};
      Tokenizer tokenizer(inputStream);
      std::ignore = tokenizer.PumpTokenizer();
      return SelectorParser::SupportsComplexSelector(tokenizer.Tokens(), {});
    }

    KRYS_NODISCARD SelectorParserContext AllEnabled() noexcept
    {
      SelectorParserContext context;
      return context;
    }
  }

  TEST_CASE("SelectorParser(namespaces)", "[CSS][Selectors][SelectorParser]")
  {
    auto sheet = StyleSheetContents::Create();
    sheet->ParserAddNamespace(CSSOMStringAtom(u8"svg"), CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));
    CHECK(sheet->NamespaceURIFromPrefix(CSSOMStringAtom(u8"svg"))
          == CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));
    CHECK(sheet->NamespaceURIFromPrefix(CSSOMStringAtom(u8"foo")) == CSSOMStringAtom::Null());
    CHECK(sheet->DefaultNamespace() == dom::StarAtom());

    CHECK(Parse(u8"svg|rect", {}, sheet.get()) == u8"svg|rect");
    CHECK(Parse(u8"svg|*.a", {}, sheet.get()) == u8"svg|*.a");
    CHECK(Parse(u8"foo|rect", {}, sheet.get()) == u8"<invalid>");
    CHECK(Parse(u8"svg|rect") == u8"<invalid>");
    // Without a default namespace, `*|` is the default and is dropped.
    CHECK(Parse(u8"*|rect") == u8"rect");
    CHECK(Parse(u8"*|*") == u8"*");
    CHECK(Parse(u8"|rect") == u8"|rect");
    CHECK(Parse(u8"[svg|href]", {}, sheet.get()) == u8"[svg|href]");
    CHECK(Parse(u8"[*|href]") == u8"[*|href]");
    CHECK(Parse(u8"[|href]") == u8"[href]");
    CHECK(Parse(u8"[foo|href]", {}, sheet.get()) == u8"<invalid>");

    auto rect = ParseValid(u8"svg|rect", {}, sheet.get());
    CHECK(rect.FirstSelectorComponent()->TagQualifiedName().NamespaceURI()
          == CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));

    // A default namespace adds an implicit, unserialised universal selector.
    sheet->ParserAddNamespace(CSSOMStringAtom::Null(), CSSOMStringAtom(u8"http://www.w3.org/1999/xhtml"));
    CHECK(sheet->DefaultNamespace() == CSSOMStringAtom(u8"http://www.w3.org/1999/xhtml"));
    auto withDefault = ParseValid(u8".a", {}, sheet.get());
    CHECK(withDefault.ComponentCount() == 2);
    CHECK(withDefault.SelectorsText() == u8".a");
    CHECK(Parse(u8"div", {}, sheet.get()) == u8"div");

    // ...except for the subject compound inside :is(), :not(), :has() without a type selector.
    auto is = ParseValid(u8":is(.a)", {}, sheet.get());
    CHECK(is.ComponentCount() == 2);
    const auto *isSelector = is.SelectorComponentAt(0)->Match() == SelectorMatch::PseudoClass
                               ? is.SelectorComponentAt(0)
                               : is.SelectorComponentAt(1);
    REQUIRE(isSelector->SubSelectors());
    CHECK(isSelector->SubSelectors()->ComponentCount() == 1);
    CHECK(is.SelectorsText() == u8":is(.a)");
  }

  TEST_CASE("SelectorParser(nesting)", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Parse(u8".a", {}, nullptr, NestedParserContextType::Style) == u8"& .a");
    CHECK(Parse(u8"> .a", {}, nullptr, NestedParserContextType::Style) == u8"& > .a");
    CHECK(Parse(u8"~ .a, + .b", {}, nullptr, NestedParserContextType::Style) == u8"& ~ .a, & + .b");
    CHECK(Parse(u8"&.b", {}, nullptr, NestedParserContextType::Style) == u8"&.b");
    CHECK(Parse(u8".c &", {}, nullptr, NestedParserContextType::Style) == u8".c &");
    CHECK(Parse(u8".c :is(&)", {}, nullptr, NestedParserContextType::Style) == u8".c :is(&)");
    CHECK(Parse(u8"> .a") == u8"<invalid>");

    CHECK(Parse(u8".a", {}, nullptr, NestedParserContextType::Scope) == u8".a");
    CHECK(Parse(u8"> .a", {}, nullptr, NestedParserContextType::Scope) == u8"> .a");
    CHECK(Parse(u8":scope .a", {}, nullptr, NestedParserContextType::Scope) == u8":scope .a");
    auto scoped = ParseValid(u8".a", {}, nullptr, NestedParserContextType::Scope);
    CHECK(scoped.FirstSelectorComponent()->HasScope());

    CHECK(Parse(u8".a", {}, nullptr, NestedParserContextType::Function) == u8".a");
  }

  TEST_CASE("SelectorParser::ResolveNestingParent", "[CSS][Selectors][SelectorParser]")
  {
    auto resolve = [](CSSOMStringView nested, Maybe<CSSOMStringView> parent, bool parentRuleIsScope = false)
    {
      auto nestedList = ParseValid(nested, {}, nullptr, NestedParserContextType::Style);
      Maybe<SelectorList> parentList;
      if (parent)
      {
        parentList = ParseValid(*parent);
      }
      return SelectorParser::ResolveNestingParent(nestedList, parentList ? &*parentList : nullptr,
                                                  parentRuleIsScope)
        .SelectorsText();
    };

    CHECK(resolve(u8".c", u8".p") == u8".p .c");
    CHECK(resolve(u8"&.c", u8".p") == u8".p.c");
    CHECK(resolve(u8".c", u8".p, .q") == u8":is(.p, .q) .c");
    CHECK(resolve(u8".c", u8".p::before") == u8":is(.p::before) .c");
    CHECK(resolve(u8"span&", u8"div") == u8"span:is(div)");
    CHECK(resolve(u8".b &", u8".p.q") == u8".b .p.q");
    CHECK(resolve(u8".b &", u8".p .q") == u8".b :is(.p .q)");
    CHECK(resolve(u8":has(&)", u8".p") == u8":has(.p)");
    CHECK(resolve(u8".c", {}) == u8":where(:scope) .c");
    CHECK(resolve(u8".c", u8".p", true) == u8":where(:scope) .c");

    // The parent's specificity is kept when it's inlined.
    auto nestedList = ParseValid(u8".c", {}, nullptr, NestedParserContextType::Style);
    auto parentList = ParseValid(u8"#p");
    auto resolved = SelectorParser::ResolveNestingParent(nestedList, &parentList);
    CHECK(resolved.FirstSelectorComponent()->ComputeSpecificity() == 0x10100);
  }

  TEST_CASE("SelectorParser::SupportsComplexSelector", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Supports(u8"a > b"));
    CHECK(Supports(u8" .a:hover::before"));
    CHECK(Supports(u8":is(.a, .b)"));
    CHECK(!Supports(u8":is(:unknown, .b)"));
    CHECK(!Supports(u8":where(.a, ::nope)"));
    CHECK(!Supports(u8"a, b"));
    CHECK(!Supports(u8"::-webkit-foo"));
    CHECK(!Supports(u8""));
  }

  TEST_CASE("SelectorParser::ParsePseudoElement", "[CSS][Selectors][SelectorParser]")
  {
    using Identifier = style::PseudoElementIdentifier;
    SelectorParserContext context;

    CHECK(SelectorParser::ParsePseudoElement(u8"::before", context)
          == Pair<bool, Maybe<Identifier>> {true, Identifier {PseudoElementType::Before}});
    CHECK(SelectorParser::ParsePseudoElement(u8":after", context)
          == Pair<bool, Maybe<Identifier>> {true, Identifier {PseudoElementType::After}});
    CHECK(SelectorParser::ParsePseudoElement(u8"::details-content", context)
          == Pair<bool, Maybe<Identifier>> {true, {}});
    CHECK(SelectorParser::ParsePseudoElement(u8"::highlight(x)", context)
          == Pair<bool, Maybe<Identifier>> {
            true, Identifier {PseudoElementType::Highlight, CSSOMStringAtom(u8"x")}});
    CHECK(!SelectorParser::ParsePseudoElement(u8"::highlight", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8"::highlight(x y)", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8":hover", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8"::before.a", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8"::part(x)", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8"before", context).first);
    CHECK(!SelectorParser::ParsePseudoElement(u8"", context).first);

    auto enabled = AllEnabled();
    CHECK(SelectorParser::ParsePseudoElement(u8"::view-transition-group(x)", enabled)
          == Pair<bool, Maybe<Identifier>> {
            true, Identifier {PseudoElementType::ViewTransitionGroup, CSSOMStringAtom(u8"x")}});
    CHECK(!SelectorParser::ParsePseudoElement(u8"::view-transition-group(*)", enabled).first);
    CHECK(style::IsNamedViewTransitionPseudoElement(
      SelectorParser::ParsePseudoElement(u8"::view-transition-new(x)", enabled).second));
  }

  TEST_CASE("SelectorParser(forgiving lists)", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Parse(u8":is(.a, :unknown, .b)") == u8":is(.a, :unknown, .b)");
    CHECK(Parse(u8":where(.a, .b c d $, .e)") == u8":where(.a, .b c d $, .e)");
    CHECK(Parse(u8":is()") == u8":is()");
    CHECK(Parse(u8":is(:unknown)") == u8":is(:unknown)");
    CHECK(Parse(u8":not(:unknown)") == u8"<invalid>");
    CHECK(Parse(u8":not(.a, :unknown)") == u8"<invalid>");

    auto list = ParseValid(u8":is(:unknown, & .a!)");
    const auto *is = list.FirstSelectorComponent();
    REQUIRE(is->SubSelectors());
    CHECK(is->SubSelectors()->SelectorComponentAt(0)->Match() == SelectorMatch::ForgivingUnknown);
    CHECK(is->SubSelectors()->SelectorComponentAt(1)->Match()
          == SelectorMatch::ForgivingUnknownNestContaining);
    CHECK(list.HasExplicitNestingParent());
  }

  TEST_CASE("SelectorParser(simple selectors)", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Parse(u8"#a.b[c]") == u8"#a.b[c]");
    CHECK(Parse(u8"#1") == u8"<invalid>");
    CHECK(Parse(u8". a") == u8"<invalid>");
    CHECK(Parse(u8"[a|=b]") == u8"[a|=\"b\"]");
    CHECK(Parse(u8"[ a ~= \"b\" ]") == u8"[a~=\"b\"]");
    CHECK(Parse(u8"[a~ =b]") == u8"<invalid>");
    CHECK(Parse(u8"[a=\"b\" I]") == u8"[a=\"b\" i]");
    CHECK(Parse(u8"[a=b x]") == u8"<invalid>");
    CHECK(Parse(u8"[a=1]") == u8"<invalid>");
    CHECK(Parse(u8"[a=b c]") == u8"<invalid>");
    CHECK(Parse(u8"[]") == u8"<invalid>");
    CHECK(Parse(u8"a b ") == u8"a b");
    CHECK(Parse(u8"a > ") == u8"<invalid>");
    CHECK(Parse(u8"a,") == u8"<invalid>");

    SelectorParserContext quirks;
    quirks.Mode = ParserMode::HTMLQuirks;
    auto list = ParseValid(u8".FooBar#Baz", quirks);
    CHECK(list.FirstSelectorComponent()->Value() == CSSOMStringAtom(u8"foobar"));
    CHECK(list.SelectorComponentAt(1)->Value() == CSSOMStringAtom(u8"baz"));
    CHECK(list.SelectorsText() == u8".FooBar#Baz");
    CHECK(ParseValid(u8".FooBar").FirstSelectorComponent()->Value() == CSSOMStringAtom(u8"FooBar"));
  }

  TEST_CASE("SelectorParser(functional pseudo-classes)", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Parse(u8":nth-child(odd of .a, b)") == u8":nth-child(2n+1 of .a, b)");
    CHECK(Parse(u8":nth-of-type(2n of .a)") == u8"<invalid>");
    CHECK(Parse(u8":nth-child(2n of)") == u8"<invalid>");
    CHECK(Parse(u8":nth-child(2n from .a)") == u8"<invalid>");
    CHECK(Parse(u8":lang(en, \"fr\")") == u8":lang(en, \"fr\")");
    CHECK(Parse(u8":lang(--x)") == u8"<invalid>");
    CHECK(Parse(u8":lang()") == u8"<invalid>");
    CHECK(Parse(u8":dir(rtl)") == u8":dir(rtl)");
    CHECK(Parse(u8":dir(rtl ltr)") == u8"<invalid>");
    CHECK(Parse(u8":host(.a)") == u8":host(.a)");
    CHECK(Parse(u8":host(.a .b)") == u8"<invalid>");
    CHECK(Parse(u8":has(> .a, + .b)") == u8":has(> .a, + .b)");
    CHECK(Parse(u8":has(:has(.a))") == u8"<invalid>");
    CHECK(Parse(u8":is(:has(.a))") == u8":is(:has(.a))");
    CHECK(Parse(u8":hover()") == u8"<invalid>");
    CHECK(Parse(u8":not") == u8"<invalid>");
    CHECK(Parse(u8":is(::before)") == u8":is(::before)");
    CHECK(Parse(u8":not(::before)") == u8"<invalid>");

    CHECK(Parse(u8":active-view-transition-type(a, b)", AllEnabled())
          == u8":active-view-transition-type(a, b)");
    CHECK(Parse(u8":active-view-transition-type(a,)", AllEnabled()) == u8"<invalid>");
    CHECK(Parse(u8":active-view-transition-type(initial)", AllEnabled()) == u8"<invalid>");
  }

  TEST_CASE("SelectorParser(pseudo-elements)", "[CSS][Selectors][SelectorParser]")
  {
    CHECK(Parse(u8"p::before") == u8"p::before");
    CHECK(Parse(u8"p:before") == u8"p::before");
    CHECK(Parse(u8"::before:hover") == u8"<invalid>");
    CHECK(Parse(u8"::before.a") == u8"<invalid>");
    CHECK(Parse(u8"::before::after") == u8"<invalid>");
    CHECK(Parse(u8"::before .a") == u8"<invalid>");
    CHECK(Parse(u8"::before:is(.a)") == u8"::before:is(.a)");
    CHECK(Parse(u8"::part(a b):hover") == u8"::part(a b):hover");
    CHECK(Parse(u8"::part(a)::before") == u8"::part(a)::before");
    CHECK(Parse(u8"::part(a):first-child") == u8"<invalid>");
    CHECK(Parse(u8"::part(a)::part(b)") == u8"<invalid>");
    CHECK(Parse(u8"::part()") == u8"<invalid>");
    CHECK(Parse(u8"::slotted(.a)::before") == u8"::slotted(.a)::before");
    CHECK(Parse(u8"::slotted(.a)::selection") == u8"<invalid>");
    CHECK(Parse(u8"::slotted(.a .b)") == u8"<invalid>");
    CHECK(Parse(u8"::details-content::marker") == u8"::details-content::marker");
    CHECK(Parse(u8"::details-content:hover") == u8"::details-content:hover");
    CHECK(Parse(u8"::details-content:empty") == u8"<invalid>");
    CHECK(Parse(u8"::highlight(x)") == u8"::highlight(x)");
    CHECK(Parse(u8"::highlight") == u8"<invalid>");
    CHECK(Parse(u8"::cue") == u8"::cue");
    CHECK(Parse(u8"video::cue(b, i)") == u8"video::cue(b, i)");
    CHECK(Parse(u8"::cue(b c)") == u8"<invalid>");
    CHECK(Parse(u8"::target-text", AllEnabled()) == u8"::target-text");

    auto enabled = AllEnabled();
    CHECK(Parse(u8"::view-transition-group(*)", enabled) == u8"::view-transition-group(*)");
    CHECK(Parse(u8"::view-transition-group(.a)", enabled) == u8"::view-transition-group(*.a)");
    CHECK(Parse(u8"::view-transition-old(x.a.b)", enabled) == u8"::view-transition-old(x.a.b)");
    CHECK(Parse(u8"::view-transition-new(x):only-child", enabled) == u8"::view-transition-new(x):only-child");
    CHECK(Parse(u8"::view-transition-new(x):hover", enabled) == u8"<invalid>");
    CHECK(Parse(u8"::view-transition-new(x .a)", enabled) == u8"<invalid>");
    CHECK(Parse(u8"::view-transition-new(none)", enabled) == u8"::view-transition-new(none)");
    CHECK(Parse(u8"::view-transition-new(inherit)", enabled) == u8"<invalid>");

    // User agent parts and ::part() cross into a shadow tree, so they start a new compound.
    auto part = ParseValid(u8"input.a::details-content");
    CHECK(part.FirstSelectorComponent()->Match() == SelectorMatch::PseudoElement);
    CHECK(part.FirstSelectorComponent()->Relation() == SelectorRelation::ShadowDescendant);
    CHECK(part.SelectorsText() == u8"input.a::details-content");
    CHECK(ParseValid(u8"::part(a)").FirstSelectorComponent()->Relation()
          == SelectorRelation::ShadowPartDescendant);
    CHECK(ParseValid(u8"::part(a)::before").SelectorComponentAt(1)->Relation()
          == SelectorRelation::ShadowPartDescendant);
    CHECK(ParseValid(u8"::slotted(.a)::before").SelectorComponentAt(1)->Relation()
          == SelectorRelation::ShadowSlotted);
  }
}
