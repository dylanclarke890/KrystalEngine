#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<CounterValue> Counter(Ref<Value> style) noexcept
    {
      return CounterValue::Create(CSSOMStringAtom {u8"item"}, CSSOMStringAtom {u8""}, krys::move(style));
    }

    KRYS_NODISCARD Ref<CounterValue> Counters(CSSOMStringView separator, Ref<Value> style) noexcept
    {
      return CounterValue::Create(CSSOMStringAtom {u8"item"}, CSSOMStringAtom {separator}, krys::move(style));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CounterValue::Create", "[CSS][Values][CounterValue]")
  {
    auto style = PrimitiveValue::Create(ValueId::UpperRoman);
    auto counter = Counters(u8".", style);
    CHECK(counter->IsCounterValue());
    CHECK(counter->Identifier() == CSSOMStringAtom {u8"item"});
    CHECK(counter->Separator() == CSSOMStringAtom {u8"."});
    CHECK(counter->CounterStyle().get() == style.get());
    CHECK(ToNarrow(counter->CounterStyleCSSText()) == "upper-roman");
    CHECK(
      ToNarrow(Counter(PrimitiveValue::CreateCustomIdent(CSSOMString {u8"my-style"}))->CounterStyleCSSText())
      == "my-style");
  }

  TEST_CASE_METHOD(ValueFixture, "CounterValue serialisation", "[CSS][Values][CounterValue]")
  {
    SECTION("counter()")
    {
      CHECK(CssText(*Counter(PrimitiveValue::Create(ValueId::UpperRoman))) == "counter(item, upper-roman)");
      CHECK(CssText(*Counter(PrimitiveValue::CreateCustomIdent(CSSOMString {u8"my-style"})))
            == "counter(item, my-style)");
    }

    SECTION("counters()")
    {
      CHECK(CssText(*Counters(u8".", PrimitiveValue::Create(ValueId::UpperRoman)))
            == "counters(item, \".\", upper-roman)");
      CHECK(CssText(*Counters(u8"\"", PrimitiveValue::Create(ValueId::UpperRoman)))
            == "counters(item, \"\\\"\", upper-roman)");
    }

    SECTION("The decimal style is omitted")
    {
      CHECK(CssText(*Counter(PrimitiveValue::Create(ValueId::Decimal))) == "counter(item)");
      CHECK(CssText(*Counter(PrimitiveValue::CreateCustomIdent(CSSOMString {u8"decimal"})))
            == "counter(item)");
      CHECK(CssText(*Counters(u8".", PrimitiveValue::Create(ValueId::Decimal))) == "counters(item, \".\")");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CounterValue serialisation with a null separator",
                   "[CSS][Values][CounterValue]")
  {
    auto counter = CounterValue::Create(CSSOMStringAtom {u8"item"}, CSSOMStringAtom::Null(),
                                        PrimitiveValue::Create(ValueId::Decimal));
    CHECK(CssText(*counter) == "counter(item)");
  }

  TEST_CASE_METHOD(ValueFixture, "CounterValue equality", "[CSS][Values][CounterValue]")
  {
    auto a = Counters(u8".", PrimitiveValue::Create(ValueId::UpperRoman));
    CHECK(*a == *Counters(u8".", PrimitiveValue::Create(ValueId::UpperRoman)));
    CHECK_FALSE(*a == *Counters(u8"-", PrimitiveValue::Create(ValueId::UpperRoman)));
    CHECK_FALSE(*a == *Counters(u8".", PrimitiveValue::Create(ValueId::Decimal)));
    CHECK_FALSE(*a == *Counter(PrimitiveValue::Create(ValueId::UpperRoman)));
    CHECK_FALSE(*a
                == *CounterValue::Create(CSSOMStringAtom {u8"other"}, CSSOMStringAtom {u8"."},
                                         PrimitiveValue::Create(ValueId::UpperRoman)));
  }

  TEST_CASE_METHOD(ValueFixture, "CounterValue visits its counter style", "[CSS][Values][CounterValue]")
  {
    auto style = PrimitiveValue::Create(ValueId::UpperRoman);
    auto counter = Counter(style);
    auto children = Children(*counter);
    REQUIRE(children.size() == 1uz);
    CHECK(children[0uz] == style.get());
  }
}
