#include "Krystal.Booey/CSS/Properties/PropertyParserResult.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "PropertyParserResult", "[CSS][Properties][PropertyParserResult]")
  {
    ParserContext context(ParserMode::HTMLStandard);
    PropertyParserState state {.Context = context};
    ParsedPropertyList properties;
    PropertyParserResult result {.ParsedProperties = properties};

    SECTION("AddProperty(Property)")
    {
      result.AddProperty(Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red)));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Id() == PropertyId::Color);
    }

    SECTION("Longhand")
    {
      result.AddProperty(state, PropertyId::Color, PropertyId::Invalid, PrimitiveValue::Create(ValueId::Red),
                         IsImportant::Yes);
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Important() == IsImportant::Yes);
      CHECK(properties[0].SetFromShorthand() == IsSetFromShorthand::No);
      CHECK(properties[0].Implicit() == IsImplicit::No);
      CHECK(CssText(properties[0].Value()) == "red");
    }

    SECTION("From an ambiguous shorthand")
    {
      result.AddProperty(state, PropertyId::MarginTop, PropertyId::Margin, PrimitiveValue::Create(0.0),
                         IsImportant::No);
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].SetFromShorthand() == IsSetFromShorthand::Yes);
      CHECK(properties[0].Metadata().ShorthandIndex == 1u);
      CHECK(properties[0].ShorthandId() == PropertyId::Margin);
    }

    SECTION("A null value is the implicit initial value")
    {
      result.AddProperty(state, PropertyId::MarginTop, PropertyId::Margin, nullptr, IsImportant::No);
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Implicit() == IsImplicit::Yes);
      CHECK(properties[0].Value().IsImplicitInitialValue());
    }

    SECTION("AddPropertyForCurrentShorthand")
    {
      state.CurrentProperty = PropertyId::Margin;
      state.Important = IsImportant::Yes;
      result.AddPropertyForCurrentShorthand(state, PropertyId::MarginLeft, PrimitiveValue::Create(0.0));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Important() == IsImportant::Yes);
      CHECK(properties[0].ShorthandId() == PropertyId::Margin);
    }

    SECTION("AddPropertyForAllLonghandsOfShorthand")
    {
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::Margin, PrimitiveValue::Create(0.0),
                                                   IsImportant::No);
      REQUIRE(properties.size() == 4uz);
      CHECK(properties[0].Id() == PropertyId::MarginTop);
      CHECK(properties[3].Id() == PropertyId::MarginLeft);
      for (auto &property : properties)
      {
        CHECK(property.ShorthandId() == PropertyId::Margin);
        CHECK(property.Implicit() == IsImplicit::No);
      }
      // Every longhand shares the value.
      CHECK(&properties[0].Value() == &properties[3].Value());
    }

    SECTION("AddPropertyForAllLonghandsOfShorthand with a null value")
    {
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::Margin, nullptr, IsImportant::No);
      REQUIRE(properties.size() == 4uz);
      for (auto &property : properties)
      {
        CHECK(property.Implicit() == IsImplicit::Yes);
        CHECK(property.Value().IsImplicitInitialValue());
      }
    }

    SECTION("AddPropertyForAllLonghandsOfCurrentShorthand")
    {
      state.CurrentProperty = PropertyId::Margin;
      result.AddPropertyForAllLonghandsOfCurrentShorthand(state, PrimitiveValue::Create(0.0),
                                                          IsImplicit::Yes);
      REQUIRE(properties.size() == 4uz);
      CHECK(properties[2].Id() == PropertyId::MarginBottom);
      CHECK(properties[2].Implicit() == IsImplicit::Yes);
    }
  }
}
