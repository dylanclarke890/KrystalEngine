#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "Property", "[CSS][Properties][Property]")
  {
    SECTION("Defaults")
    {
      Property property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red));
      CHECK(property.Id() == PropertyId::Color);
      CHECK_FALSE(property.Important());
      CHECK_FALSE(property.Implicit());
      CHECK_FALSE(property.SetFromShorthand());
      CHECK(property.ShorthandIndex() == 0u);
      CHECK(property.ShorthandId() == PropertyId::Invalid);
      CHECK(CssText(property.Value()) == "red");
    }

    SECTION("Flags")
    {
      Property property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant(true),
                        IsImplicit(true));
      CHECK(property.Important());
      CHECK(property.Implicit());
      CHECK_FALSE(property.SetFromShorthand());
    }

    SECTION("ShorthandId")
    {
      // margin-top's shorthands are `all` and `margin`.
      Property fromMargin(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant(false),
                          IsImplicit(false), IsSetFromShorthand(true), 1u);
      CHECK(fromMargin.SetFromShorthand());
      CHECK(fromMargin.ShorthandIndex() == 1u);
      CHECK(fromMargin.ShorthandId() == PropertyId::Margin);

      Property fromAll(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant(false),
                       IsImplicit(false), IsSetFromShorthand(true), 0u);
      CHECK(fromAll.ShorthandId() == PropertyId::All);
    }

    SECTION("Metadata constructor")
    {
      Property original(PropertyId::MarginTop, PrimitiveValue::Create(0.0), IsImportant(true),
                        IsImplicit(false), IsSetFromShorthand(true), 1u);
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
      CHECK_FALSE(a == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant(true)));
      CHECK_FALSE(a
                  == Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red), IsImportant(false),
                              IsImplicit(true)));
    }

    SECTION("ParsedPropertyList")
    {
      ParsedPropertyList list;
      list.emplace_back(PropertyId::Color, PrimitiveValue::Create(ValueId::Red));
      REQUIRE(list.size() == 1uz);
      CHECK(list[0].Id() == PropertyId::Color);
    }
  }

  TEST_CASE("Property statics", "[CSS][Properties][Property]")
  {
    SECTION("IsDirectionAwareProperty")
    {
      CHECK(Property::IsDirectionAwareProperty(PropertyId::MarginInlineStart));
      CHECK_FALSE(Property::IsDirectionAwareProperty(PropertyId::MarginLeft));
      CHECK_FALSE(Property::IsDirectionAwareProperty(PropertyId::Color));
    }

    SECTION("IsListValuedProperty")
    {
      CHECK(Property::IsListValuedProperty(PropertyId::BackgroundImage));
      CHECK(Property::IsListValuedProperty(PropertyId::Custom));
      CHECK_FALSE(Property::IsListValuedProperty(PropertyId::Color));
    }

    SECTION("IsSizingProperty")
    {
      CHECK(Property::IsSizingProperty(PropertyId::Width));
      CHECK(Property::IsSizingProperty(PropertyId::MaxHeight));
      CHECK(Property::IsSizingProperty(PropertyId::MinInlineSize));
      CHECK_FALSE(Property::IsSizingProperty(PropertyId::Margin));
    }
  }
}
