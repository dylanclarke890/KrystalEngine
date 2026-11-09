#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSVariableReferenceValue.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<CSSVariableReferenceValue> Reference(CSSOMString variable,
                                                            RefPtr<CSSUnparsedValue> fallback = {})
    {
      auto result = CSSVariableReferenceValue::Create(krys::move(variable), krys::move(fallback));
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    KRYS_NODISCARD CSSUnparsedSegment Segment(Ref<CSSVariableReferenceValue> reference)
    {
      return RefPtr<CSSVariableReferenceValue>(krys::move(reference));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSUnparsedValue: segments",
                   "[CSS][TypedOM][css-typed-om:cssunparsedvalue]")
  {
    auto value = CSSUnparsedValue::Create({CSSOMString(u8"1px "), Segment(Reference(u8"--a"))});
    CHECK(value->Length() == 2);
    CHECK(ToNarrow(value->ToString()) == "1px var(--a)");
    CHECK(value->IsSupportedPropertyIndex(1));
    CHECK_FALSE(value->IsSupportedPropertyIndex(2));
    CHECK_FALSE(value->Item(2));
    REQUIRE(value->Item(0));
    CHECK(ToNarrow(std::get<CSSOMString>(*value->Item(0))) == "1px ");

    // Setting the item just past the end appends it; further is a RangeError.
    CHECK(value->SetItem(2, CSSOMString(u8" end")).HasValue());
    CHECK(value->Length() == 3);
    CHECK(value->SetItem(0, CSSOMString(u8"2px ")).HasValue());
    CHECK(ToNarrow(value->ToString()) == "2px var(--a) end");
    auto outOfRange = value->SetItem(5, CSSOMString(u8"x"));
    REQUIRE(outOfRange.HasException());
    CHECK(outOfRange.GetException().Code() == dom::ExceptionCode::RangeError);

    // Its CSS value is a variable reference.
    auto cssValue = value->ToCSSValue();
    REQUIRE(cssValue);
    CHECK(Is<VariableReferenceValue>(*cssValue));
  }

  TEST_CASE_METHOD(ValueFixture, "CSSUnparsedValue: reifying tokens",
                   "[CSS][TypedOM][css-typed-om:reify-a-list-of-component-values]")
  {
    auto document = CreateRef<dom::Document>();
    auto parsed =
      CSSStyleValue::Parse(*document, u8"--x", u8"a var(--b, var(--c, 1px)) calc(var(--d) + 2px)");
    REQUIRE(parsed.HasValue());
    auto &value = Downcast<CSSUnparsedValue>(*parsed.Value());
    CHECK(ToNarrow(value.ToString()) == "a var(--b, var(--c, 1px)) calc(var(--d) + 2px)");

    // The references are segments, with their fallbacks.
    REQUIRE(value.Length() == 5);
    const auto &reference = *std::get<RefPtr<CSSVariableReferenceValue>>(value.Segments()[1]);
    CHECK(ToNarrow(reference.Variable()) == "--b");
    REQUIRE(reference.Fallback());
    CHECK(ToNarrow(reference.Fallback()->ToString()) == " var(--c, 1px)");
    CHECK(ToNarrow(std::get<CSSOMString>(value.Segments()[2])) == " calc(");
    CHECK(ToNarrow(std::get<RefPtr<CSSVariableReferenceValue>>(value.Segments()[3])->Variable()) == "--d");

    // A `var()` that isn't a reference, and an unclosed one, are reified as well as they can be.
    auto notReference = CSSStyleValue::Parse(*document, u8"--x", u8"var(1)");
    REQUIRE(notReference.HasValue());
    CHECK(ToNarrow(notReference.Value()->ToString()) == "var(1)");
    auto unclosed = CSSStyleValue::Parse(*document, u8"--x", u8"var(--a, 1");
    REQUIRE(unclosed.HasValue());
    CHECK(ToNarrow(unclosed.Value()->ToString()) == "var(--a, 1)");
  }

  TEST_CASE_METHOD(ValueFixture, "CSSVariableReferenceValue: variable",
                   "[CSS][TypedOM][css-typed-om:cssvariablereferencevalue]")
  {
    CHECK(CSSVariableReferenceValue::Create(u8"a").GetException().Code() == dom::ExceptionCode::TypeError);

    auto reference = Reference(u8"--a", CSSUnparsedValue::Create({CSSOMString(u8"1px")}));
    CHECK(ToNarrow(reference->ToString()) == "var(--a,1px)");
    CHECK(reference->SetVariable(u8"b").HasException());
    CHECK(ToNarrow(reference->Variable()) == "--a");
    CHECK_FALSE(reference->SetVariable(u8"--b").HasException());
    CHECK(ToNarrow(reference->ToString()) == "var(--b,1px)");
  }
}
