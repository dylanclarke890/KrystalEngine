#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD CounterStyleDescriptors Descriptors(utf8_stringview css)
    {
      return CounterStyleDescriptors::Create(CSSOMStringAtom(u8"foo"),
                                             *ParseDescriptors(css, RuleType::CounterStyle));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CounterStyleDescriptors::Create", "[CSS][Rules][CounterStyleRule]")
  {
    SECTION("Defaults")
    {
      auto descriptors = Descriptors(u8"");
      CHECK(descriptors.Name == CSSOMStringAtom(u8"foo"));
      CHECK(descriptors.System == CounterStyleSystem::Symbolic);
      CHECK(descriptors.NegativeSymbols == CounterStyleNegativeSymbols {});
      CHECK(descriptors.NegativeSymbols.Prefix.Text == u8"-");
      CHECK(descriptors.Prefix == CounterStyleSymbol {});
      CHECK(descriptors.Suffix == CounterStyleSymbol {false, u8". "});
      CHECK(descriptors.Ranges.empty());
      CHECK(descriptors.FallbackName == CSSOMStringAtom(u8"decimal"));
      CHECK(descriptors.ExtendsName == CSSOMStringAtom(u8"decimal"));
      CHECK(descriptors.FixedSystemFirstSymbolValue == 1);
      CHECK(descriptors.SpeakAs == CounterStyleSpeakAs::Auto);
      CHECK(descriptors.ExplicitlySetDescriptors == CounterStyleDescriptor::None);
      // `symbolic` needs at least one symbol.
      CHECK_FALSE(descriptors.IsValid());

      // Nothing was set explicitly, so nothing serialises.
      CHECK(descriptors.SystemCSSText().empty());
      CHECK(descriptors.SymbolsCSSText().empty());
      CHECK(descriptors.SuffixCSSText().empty());
      CHECK(descriptors.FallbackCSSText().empty());
      CHECK(descriptors.RangesCSSText().empty());
      CHECK(ToNarrow(descriptors.NameCSSText()) == "foo");
    }

    SECTION("Every descriptor")
    {
      auto descriptors = Descriptors(u8"system: fixed 3; symbols: a \"b\" c; prefix: \"[\"; suffix: \")\"; "
                                     u8"range: 1 10, 20 infinite; pad: 3 \"0\"; negative: \"(\" \")\"; "
                                     u8"fallback: lower-roman");
      CHECK(descriptors.System == CounterStyleSystem::Fixed);
      CHECK(descriptors.FixedSystemFirstSymbolValue == 3);
      REQUIRE(descriptors.Symbols.size() == 3);
      CHECK(descriptors.Symbols[0] == CounterStyleSymbol {true, u8"a"});
      CHECK(descriptors.Symbols[1] == CounterStyleSymbol {false, u8"b"});
      CHECK(descriptors.IsValid());

      CHECK(ToNarrow(descriptors.SystemCSSText()) == "fixed 3");
      CHECK(ToNarrow(descriptors.SymbolsCSSText()) == "a \"b\" c");
      CHECK(ToNarrow(descriptors.PrefixCSSText()) == "\"[\"");
      CHECK(ToNarrow(descriptors.SuffixCSSText()) == "\")\"");
      CHECK(ToNarrow(descriptors.RangesCSSText()) == "1 10, 20 infinite");
      CHECK(ToNarrow(descriptors.PadCSSText()) == "3 \"0\"");
      CHECK(ToNarrow(descriptors.NegativeCSSText()) == "\"(\" \")\"");
      CHECK(ToNarrow(descriptors.FallbackCSSText()) == "lower-roman");
      CHECK(descriptors.AdditiveSymbolsCSSText().empty());
    }

    SECTION("Systems")
    {
      CHECK(Descriptors(u8"system: cyclic").System == CounterStyleSystem::Cyclic);
      CHECK(Descriptors(u8"system: numeric").System == CounterStyleSystem::Numeric);
      CHECK(Descriptors(u8"system: alphabetic").System == CounterStyleSystem::Alphabetic);
      CHECK(Descriptors(u8"system: fixed").FixedSystemFirstSymbolValue == 1);
      CHECK(ToNarrow(Descriptors(u8"system: fixed").SystemCSSText()) == "fixed 1");

      auto extends = Descriptors(u8"system: extends lower-alpha");
      CHECK(extends.System == CounterStyleSystem::Extends);
      CHECK(extends.ExtendsName == CSSOMStringAtom(u8"lower-alpha"));
      CHECK(ToNarrow(extends.SystemCSSText()) == "extends lower-alpha");
      CHECK(extends.IsValid());
    }

    SECTION("Additive symbols")
    {
      auto descriptors = Descriptors(u8"system: additive; additive-symbols: 10 X, 5 \"V\", 1 I");
      CHECK(descriptors.System == CounterStyleSystem::Additive);
      REQUIRE(descriptors.AdditiveSymbols.size() == 3);
      CHECK(descriptors.AdditiveSymbols[1].second == 5u);
      CHECK(descriptors.IsValid());
      CHECK(ToNarrow(descriptors.AdditiveSymbolsCSSText()) == "10 X, 5 \"V\", 1 I");
    }

    SECTION("range: auto")
    {
      auto descriptors = Descriptors(u8"range: auto");
      CHECK(descriptors.Ranges.empty());
      CHECK(ToNarrow(descriptors.RangesCSSText()) == "auto");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CounterStyleDescriptors setters", "[CSS][Rules][CounterStyleRule]")
  {
    auto descriptors = Descriptors(u8"system: alphabetic; symbols: a b");

    SECTION("Setters mark the descriptor as explicitly set")
    {
      descriptors.SetPrefix({false, u8"<"});
      CHECK(HasFlag(descriptors.ExplicitlySetDescriptors, CounterStyleDescriptor::Prefix));
      CHECK(ToNarrow(descriptors.PrefixCSSText()) == "\"<\"");

      descriptors.SetRanges({{1, 2}});
      CHECK(ToNarrow(descriptors.RangesCSSText()) == "1 2");

      descriptors.SetPad({2u, {true, u8"z"}});
      CHECK(ToNarrow(descriptors.PadCSSText()) == "2 z");

      descriptors.SetFallbackName(CSSOMStringAtom(u8"disc"));
      CHECK(ToNarrow(descriptors.FallbackCSSText()) == "disc");

      descriptors.SetNegative({{false, u8"-"}, {false, u8"!"}});
      CHECK(ToNarrow(descriptors.NegativeCSSText()) == "\"-\" \"!\"");
    }

    SECTION("Symbols must stay valid for the system")
    {
      descriptors.SetSymbols({{true, u8"x"}});
      CHECK(descriptors.Symbols.size() == 2);
      descriptors.SetSymbols({{true, u8"x"}, {true, u8"y"}, {true, u8"z"}});
      CHECK(ToNarrow(descriptors.SymbolsCSSText()) == "x y z");
      descriptors.SetAdditiveSymbols({{{true, u8"x"}, 1u}});
      CHECK(descriptors.AdditiveSymbols.size() == 1);
    }

    SECTION("SetName ignores null names")
    {
      descriptors.SetName(CSSOMStringAtom::Null());
      CHECK(descriptors.Name == CSSOMStringAtom(u8"foo"));
      descriptors.SetName(CSSOMStringAtom(u8"bar"));
      CHECK(descriptors.Name == CSSOMStringAtom(u8"bar"));
    }

    SECTION("SetSystemData")
    {
      descriptors.SetSystemData({CSSOMStringAtom(u8"upper-alpha"), 4});
      CHECK(descriptors.ExtendsName == CSSOMStringAtom(u8"upper-alpha"));
      CHECK(descriptors.FixedSystemFirstSymbolValue == 4);
    }

    SECTION("IsExtendedResolved")
    {
      descriptors.IsExtendedResolved = true;
      CHECK(ToNarrow(descriptors.SystemCSSText()) == "extends decimal");
    }

    CHECK(CounterStyleDescriptors::AreSymbolsValidForSystem(CounterStyleSystem::DisclosureOpen, {}, {}));
    CHECK_FALSE(
      CounterStyleDescriptors::AreSymbolsValidForSystem(CounterStyleSystem::Extends, {{true, u8"a"}}, {}));
  }

  TEST_CASE_METHOD(ValueFixture, "CounterStyleRule", "[CSS][Rules][CounterStyleRule]")
  {
    auto rule = CounterStyleRule::Create(
      CSSOMStringAtom(u8"foo"),
      Descriptors(u8"system: cyclic; symbols: \"*\"; suffix: \" \"; speak-as: bullets"));
    CHECK(rule->Type() == RuleType::CounterStyle);
    CHECK(rule->IsCounterStyleRule());
    CHECK(rule->Name() == CSSOMStringAtom(u8"foo"));
    CHECK(ToNarrow(rule->System()) == "cyclic");
    CHECK(ToNarrow(rule->Symbols()) == "\"*\"");
    CHECK(ToNarrow(rule->Suffix()) == "\" \"");
    CHECK(rule->Prefix().empty());
    CHECK(rule->Range().empty());
    // speak-as is never serialised.
    CHECK(rule->SpeakAs().empty());
    CHECK(HasFlag(rule->Descriptors().ExplicitlySetDescriptors, CounterStyleDescriptor::SpeakAs));

    rule->SetName(CSSOMStringAtom(u8"bar"));
    rule->MutableDescriptors().SetPrefix({false, u8"#"});

    Ref<Rule> base = rule;
    auto copied = base->Copy();
    auto &copy = Downcast<CounterStyleRule>(*copied);
    CHECK(copy.Name() == CSSOMStringAtom(u8"bar"));
    CHECK(copy.Descriptors() == rule->Descriptors());
  }
}
