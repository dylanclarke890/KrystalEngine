#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include <string>

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<css::VariableData> VariableData(CSSOMStringView text) noexcept
    {
      css::InputStream input {CSSOMString {text}};
      css::Tokenizer tokenizer {input};
      std::ignore = tokenizer.PumpTokenizer();
      return css::VariableData::Create(tokenizer.Tokens());
    }

    KRYS_NODISCARD std::string Serialised(const CustomProperty &property) noexcept
    {
      auto text = property.PropertyValueSerialisation(css::DefaultSerialisationContext(),
                                                      RenderStyle::DefaultStyleSingleton());
      return {text.begin(), text.end()};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Custom properties", "[Style][CustomProperty]")
  {
    const CSSOMStringAtom name {u8"--x"};

    auto invalid = CustomProperty::CreateForGuaranteedInvalid(name);
    CHECK(invalid->IsGuaranteedInvalid());
    CHECK_FALSE(invalid->IsAnimatable());
    CHECK(Serialised(*invalid).empty());
    CHECK(invalid->Tokens().empty());

    auto tokens = CustomProperty::CreateForVariableData(name, VariableData(u8"1px  solid"));
    CHECK(tokens->IsVariableData());
    CHECK(tokens->GetName() == name);
    CHECK_FALSE(tokens->IsAnimatable());
    CHECK(Serialised(*tokens) == "1px  solid");
    CHECK(tokens->Tokens().size() == 3);
    CHECK(*tokens == *CustomProperty::CreateForVariableData(name, VariableData(u8"1px  solid")));
    CHECK_FALSE(*tokens == *CustomProperty::CreateForVariableData(name, VariableData(u8"2px")));

    auto length = CustomProperty::CreateForValue(name, CustomProperty::Value {Length<> {10}});
    CHECK(length->IsValue());
    CHECK(length->IsAnimatable());
    CHECK(Serialised(*length) == "10px");
    // Typed values are tokenized from their serialisation.
    REQUIRE(length->Tokens().size() == 1);
    CHECK(length->Tokens()[0].Type() == css::TokenType::Dimension);
    CHECK(css::tests::CssText(
            *length->PropertyValue(css::ValuePool::MainThreadPool(), RenderStyle::DefaultStyleSingleton()))
          == "10px");

    auto list = CustomProperty::CreateForValueList(
      name, CustomProperty::ValueList {.Values = {Length<> {1}, Length<> {2}},
                                       .Separator = css::ValueSeparator::CommaSeparator});
    CHECK(list->IsValueList());
    CHECK(Serialised(*list) == "1px, 2px");
    CHECK(css::tests::CssText(
            *list->PropertyValue(css::ValuePool::MainThreadPool(), RenderStyle::DefaultStyleSingleton()))
          == "1px, 2px");

    CHECK_FALSE(*length == *list);
    CHECK_FALSE(*length == *CustomProperty::CreateForValue(u8"--y", CustomProperty::Value {Length<> {10}}));
  }

  TEST_CASE("Custom property data", "[Style][CustomProperty]")
  {
    auto value = [](float px)
    {
      return CustomProperty::CreateForValue(u8"--a", CustomProperty::Value {Length<> {px}});
    };
    auto named = [](const char8_t *name, float px)
    {
      return CustomProperty::CreateForValue(CSSOMStringAtom {name}, CustomProperty::Value {Length<> {px}});
    };

    auto root = CustomPropertyData::Create();
    CHECK(root->Size() == 0);
    root->Set(u8"--a", value(1));
    root->Set(u8"--b", named(u8"--b", 2));
    CHECK(root->Size() == 2);
    CHECK(root->MayHaveAnimatableProperties());
    REQUIRE(root->Get(u8"--a"));
    CHECK(*root->Get(u8"--a") == *value(1));
    CHECK_FALSE(root->Get(u8"--missing"));

    // A copy of the root references it as its parent; overriding a value doesn't change the size.
    auto child = root->Copy();
    CHECK(*child == *root);
    child->Set(u8"--a", value(5));
    CHECK(child->Size() == 2);
    CHECK(*child->Get(u8"--a") == *value(5));
    CHECK(*root->Get(u8"--a") == *value(1));
    CHECK_FALSE(*child == *root);

    child->Set(u8"--c", named(u8"--c", 3));
    CHECK(child->Size() == 3);

    // Iteration visits each property once, preferring the overriding value.
    uint32 count = 0;
    child->ForEach(
      [&](const CustomPropertyData::Entry &entry)
      {
        ++count;
        if (entry.first == u8"--a")
        {
          CHECK(*entry.second == *value(5));
        }
        return IterationStatus::Continue;
      });
    CHECK(count == 3);
    CHECK(child->FindKeyAtIndex(3) == null);
    CHECK_FALSE(child->FindKeyAtIndex(0) == null);

    // Data with equal values under different parents compares equal.
    auto other = CustomPropertyData::Create();
    other->Set(u8"--a", value(5));
    other->Set(u8"--b", named(u8"--b", 2));
    other->Set(u8"--c", named(u8"--c", 3));
    CHECK(*other == *child);
  }
}
