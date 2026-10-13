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
                         IsImportant(true));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Important());
      CHECK_FALSE(properties[0].SetFromShorthand());
      CHECK_FALSE(properties[0].Implicit());
      CHECK(CssText(properties[0].Value()) == "red");
    }

    SECTION("From an ambiguous shorthand")
    {
      result.AddProperty(state, PropertyId::MarginTop, PropertyId::Margin, PrimitiveValue::Create(0.0),
                         IsImportant(false));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].SetFromShorthand());
      CHECK(properties[0].ShorthandIndex() == 1u);
      CHECK(properties[0].ShorthandId() == PropertyId::Margin);
    }

    SECTION("A null value is the implicit initial value")
    {
      result.AddProperty(state, PropertyId::MarginTop, PropertyId::Margin, nullptr, IsImportant(false));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Implicit());
      CHECK(properties[0].Value().IsImplicitInitialValue());
    }

    SECTION("AddPropertyForCurrentShorthand")
    {
      state.CurrentProperty = PropertyId::Margin;
      state.Important = IsImportant(true);
      result.AddPropertyForCurrentShorthand(state, PropertyId::MarginLeft, PrimitiveValue::Create(0.0));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Important());
      CHECK(properties[0].ShorthandId() == PropertyId::Margin);
    }

    SECTION("AddPropertyForAllLonghandsOfShorthand")
    {
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::Margin, PrimitiveValue::Create(0.0),
                                                   IsImportant(false));
      REQUIRE(properties.size() == 4uz);
      CHECK(properties[0].Id() == PropertyId::MarginTop);
      CHECK(properties[3].Id() == PropertyId::MarginLeft);
      for (auto &property : properties)
      {
        CHECK(property.ShorthandId() == PropertyId::Margin);
        CHECK_FALSE(property.Implicit());
      }
      // Every longhand shares the value.
      CHECK(&properties[0].Value() == &properties[3].Value());
    }

    SECTION("AddPropertyForAllLonghandsOfShorthand with a null value")
    {
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::Margin, nullptr, IsImportant(false));
      REQUIRE(properties.size() == 4uz);
      for (auto &property : properties)
      {
        CHECK(property.Implicit());
        CHECK(property.Value().IsImplicitInitialValue());
      }
    }

    SECTION("AddPropertyForAllLonghandsOfCurrentShorthand")
    {
      state.CurrentProperty = PropertyId::Margin;
      result.AddPropertyForAllLonghandsOfCurrentShorthand(state, PrimitiveValue::Create(0.0),
                                                          IsImplicit(true));
      REQUIRE(properties.size() == 4uz);
      CHECK(properties[2].Id() == PropertyId::MarginBottom);
      CHECK(properties[2].Implicit());
    }
  }
}
