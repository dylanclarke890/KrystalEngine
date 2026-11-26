#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "Property", "[CSS][Properties][Property]")
  {
    SECTION("Defaults")
    {
      Property property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red));
      CHECK(property.Id() == PropertyId::Color);
      CHECK(property.Important() == IsImportant::No);
      CHECK(property.Implicit() == IsImplicit::No);
      CHECK(property.SetFromShorthand() == IsSetFromShorthand::No);
      CHECK(property.Metadata().ShorthandIndex == 0u);
      CHECK(property.ShorthandId() == PropertyId::Invalid);
      CHECK(CssText(property.Value()) == "red");
    }

    SECTION("Flags")
    {
      Property property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant::Yes,
                        IsImplicit::Yes);
      CHECK(property.Important() == IsImportant::Yes);
      CHECK(property.Implicit() == IsImplicit::Yes);
      CHECK(property.SetFromShorthand() == IsSetFromShorthand::No);
    }

    SECTION("ShorthandId")
    {
      // margin-top's shorthands are `all` and `margin`.
      Property fromMargin(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant::No, IsImplicit::No,
                          IsSetFromShorthand::Yes, 1u);
      CHECK(fromMargin.SetFromShorthand() == IsSetFromShorthand::Yes);
      CHECK(fromMargin.Metadata().ShorthandIndex == 1u);
      CHECK(fromMargin.ShorthandId() == PropertyId::Margin);

      Property fromAll(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant::No, IsImplicit::No,
                       IsSetFromShorthand::Yes, 0u);
      CHECK(fromAll.ShorthandId() == PropertyId::All);
    }

    SECTION("Metadata constructor")
    {
      Property original(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant::Yes, IsImplicit::No,
                        IsSetFromShorthand::Yes, 1u);
      Property copy(original.Metadata(), PrimitiveValue::Create(0.0));
      CHECK(copy.Metadata() == original.Metadata());
      CHECK(copy == original);
    }

    SECTION("Equality")
    {
      Property a(PropertyId::Color, PrimitiveValue::Create(ValueId::Red));
      CHECK(a == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red)));
      CHECK_FALSE(a == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Blue)));
      CHECK_FALSE(a == Property(PropertyId::BackgroundColor, PrimitiveValue::Create(ValueId::Red)));
      CHECK_FALSE(a == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant::Yes));
      CHECK_FALSE(a
                  == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant::No,
                              IsImplicit::Yes));
    }

    SECTION("ParsedPropertyList")
    {
      ParsedPropertyList list;
      list.emplace_back(PropertyId::Color, PrimitiveValue::Create(ValueId::Red));
      REQUIRE(list.size() == 1uz);
      CHECK(list[0].Id() == PropertyId::Color);
    }
  }
}
