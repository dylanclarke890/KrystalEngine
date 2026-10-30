#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<VariableData> MakeData(utf8_stringview css)
    {
      if (css.empty())
      {
        return VariableData::Create(css::TokenRange(Span<const Token> {}));
      }

      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());
      return VariableData::Create(tokenizer.Tokens());
    }

    Ref<VariableReferenceValue> MakeReference(utf8_stringview css)
    {
      return VariableReferenceValue::Create(MakeData(css));
    }

    const CSSOMStringAtom &Name()
    {
      static const CSSOMStringAtom name {u8"--x"};
      return name;
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CustomPropertyValue::Create", "[CSS][Values][CustomPropertyValue]")
  {
    SECTION("CreateEmpty")
    {
      auto value = CustomPropertyValue::CreateEmpty(Name());
      CHECK(value->IsCustomPropertyValue());
      CHECK(value->Name() == Name());
      CHECK(value->IsVariableData());
      CHECK_FALSE(value->IsVariableReference());
      CHECK_FALSE(value->IsGlobalKeyword());
      CHECK_FALSE(value->TryGlobalKeyword());
      CHECK(value->AsVariableData()->Tokens().empty());
    }

    SECTION("CreateUnresolved")
    {
      auto reference = MakeReference(u8"var(--y)");
      auto *referencePointer = reference.get();
      auto value = CustomPropertyValue::CreateUnresolved(Name(), krys::move(reference));
      CHECK(value->IsVariableReference());
      CHECK_FALSE(value->IsVariableData());
      CHECK_FALSE(value->IsGlobalKeyword());
      CHECK_FALSE(value->TryGlobalKeyword());
      // The reference's own data, not a copy.
      CHECK(value->AsVariableData().get() == &referencePointer->Data());
    }

    SECTION("CreateSyntaxAll")
    {
      auto data = MakeData(u8"1px solid");
      auto *dataPointer = data.get();
      auto value = CustomPropertyValue::CreateSyntaxAll(Name(), krys::move(data));
      CHECK(value->IsVariableData());
      CHECK_FALSE(value->IsVariableReference());
      CHECK(value->AsVariableData().get() == dataPointer);
    }

    SECTION("CreateWithGlobalKeyword")
    {
      auto value = CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Inherit);
      CHECK(value->IsGlobalKeyword());
      CHECK_FALSE(value->IsVariableData());
      CHECK_FALSE(value->IsVariableReference());
      REQUIRE(value->TryGlobalKeyword());
      CHECK(*value->TryGlobalKeyword() == GlobalKeyword::Inherit);
      CHECK(value->AsVariableData()->Tokens().empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CustomPropertyValue serialisation", "[CSS][Values][CustomPropertyValue]")
  {
    // Only the value is serialised, not the name.
    CHECK(CssText(*CustomPropertyValue::CreateEmpty(Name())) == "");
    CHECK(CssText(*CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"1px solid"))) == "1px solid");
    CHECK(CssText(*CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--y, red)")))
          == "var(--y, red)");

    CHECK(CssText(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Initial))
          == "initial");
    CHECK(CssText(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Inherit))
          == "inherit");
    CHECK(CssText(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Unset)) == "unset");
    CHECK(CssText(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Revert)) == "revert");
    CHECK(CssText(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::RevertLayer))
          == "revert-layer");
  }

  TEST_CASE_METHOD(ValueFixture, "CustomPropertyValue::IsCurrentColor", "[CSS][Values][CustomPropertyValue]")
  {
    CHECK(CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"currentcolor"))->IsCurrentColor());
    CHECK(CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"currentcolor "))->IsCurrentColor());
    CHECK_FALSE(CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"red"))->IsCurrentColor());
    CHECK_FALSE(
      CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"currentcolor red"))->IsCurrentColor());
    CHECK_FALSE(CustomPropertyValue::CreateEmpty(Name())->IsCurrentColor());
    CHECK_FALSE(
      CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Inherit)->IsCurrentColor());
    CHECK_FALSE(CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--y)"))->IsCurrentColor());
  }

  TEST_CASE_METHOD(ValueFixture, "CustomPropertyValue equality and hashing",
                   "[CSS][Values][CustomPropertyValue]")
  {
    SECTION("Data")
    {
      auto a = CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"1px"));
      CHECK(*a == *CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"1px")));
      CHECK_FALSE(*a == *CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"2px")));
      CHECK_FALSE(*a
                  == *CustomPropertyValue::CreateSyntaxAll(CSSOMStringAtom {u8"--other"}, MakeData(u8"1px")));
      CHECK(*CustomPropertyValue::CreateEmpty(Name()) == *CustomPropertyValue::CreateEmpty(Name()));
    }

    SECTION("References")
    {
      auto a = CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--y)"));
      CHECK(*a == *CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--y)")));
      CHECK_FALSE(*a == *CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--z)")));
    }

    SECTION("Keywords")
    {
      auto a = CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Unset);
      CHECK(*a == *CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Unset));
      CHECK_FALSE(*a == *CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Initial));
    }

    SECTION("Different kinds are never equal")
    {
      // Same tokens, but data vs a reference.
      auto data = CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"var(--y)"));
      auto reference = CustomPropertyValue::CreateUnresolved(Name(), MakeReference(u8"var(--y)"));
      CHECK_FALSE(*data == *reference);
      CHECK_FALSE(*CustomPropertyValue::CreateEmpty(Name())
                  == *CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Initial));
    }

    // `CustomPropertyValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(CustomPropertyValue::CreateEmpty(Name())->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "CustomPropertyValue visitation", "[CSS][Values][CustomPropertyValue]")
  {
    // Only a variable reference is a child value.
    auto reference = MakeReference(u8"var(--y)");
    auto *referencePointer = reference.get();
    auto unresolved = CustomPropertyValue::CreateUnresolved(Name(), krys::move(reference));
    auto children = Children(*unresolved);
    REQUIRE(children.size() == 1uz);
    CHECK(children[0uz] == referencePointer);

    CHECK(Children(*CustomPropertyValue::CreateSyntaxAll(Name(), MakeData(u8"1px"))).empty());
    CHECK(Children(*CustomPropertyValue::CreateWithGlobalKeyword(Name(), GlobalKeyword::Inherit)).empty());
  }
}
