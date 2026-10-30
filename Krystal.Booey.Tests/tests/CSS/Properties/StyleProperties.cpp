#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "StyleProperties::AsText", "[CSS][Properties][StyleProperties]")
  {
    SECTION("Longhands keep their order")
    {
      CHECK(AsText(ParseDeclarations(u8"color: red; width: 10px")) == "color: red; width: 10px;");
      CHECK(AsText(ParseDeclarations(u8"width: 10px; color: red !important"))
            == "width: 10px; color: red !important;");
    }

    SECTION("Longhands are collapsed into a shorthand where possible")
    {
      CHECK(AsText(ParseDeclarations(u8"margin: 1px 2px")) == "margin: 1px 2px;");
      CHECK(AsText(ParseDeclarations(u8"color: red; margin: 1px; width: 2px"))
            == "color: red; margin: 1px; width: 2px;");
      CHECK(AsText(
              ParseDeclarations(u8"margin-top: 1px; margin-right: 2px; margin-bottom: 3px; margin-left: 4px"))
            == "margin: 1px 2px 3px 4px;");
    }

    SECTION("A missing longhand prevents collapsing")
    {
      CHECK(AsText(ParseDeclarations(u8"margin-top: 1px; margin-right: 1px; margin-bottom: 1px"))
            == "margin-top: 1px; margin-right: 1px; margin-bottom: 1px;");
    }

    SECTION("Longhands with different importance aren't collapsed")
    {
      CHECK(AsText(ParseDeclarations(u8"margin: 1px; margin-left: 2px !important"))
            == "margin-top: 1px; margin-right: 1px; margin-bottom: 1px; margin-left: 2px !important;");
      CHECK(AsText(ParseDeclarations(u8"margin: 1px !important")) == "margin: 1px !important;");
    }

    SECTION("CSS-wide keywords")
    {
      CHECK(AsText(ParseDeclarations(u8"margin: inherit")) == "margin: inherit;");
      CHECK(AsText(ParseDeclarations(u8"margin: inherit; margin-top: initial"))
            == "margin-top: initial; margin-right: inherit; margin-bottom: inherit; margin-left: inherit;");
    }

    SECTION("Custom properties")
    {
      CHECK(AsText(ParseDeclarations(u8"--foo: bar; color: red")) == "--foo: bar; color: red;");
      CHECK(AsText(ParseDeclarations(u8"--foo: bar !important")) == "--foo: bar !important;");
    }

    SECTION("A shorthand with variables is kept as written")
    {
      CHECK(AsText(ParseDeclarations(u8"margin: var(--x) 1px")) == "margin: var(--x) 1px;");
    }

    SECTION("Legacy shorthands aren't used")
    {
      CHECK(AsText(ParseDeclarations(u8"break-after: page")) == "break-after: page;");
    }

    SECTION("Empty")
    {
      CHECK(AsText(MutableStyleProperties::Create()).empty());
      CHECK(MutableStyleProperties::Create()->AsTextAtom(DefaultSerialisationContext()).View().empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleProperties::GetPropertyValue", "[CSS][Properties][StyleProperties]")
  {
    auto properties = ParseDeclarations(u8"margin: 1px 2px; color: red; --foo: bar");

    CHECK(ValueOf(properties, PropertyId::MarginTop) == "1px");
    CHECK(ValueOf(properties, PropertyId::MarginRight) == "2px");
    CHECK(ValueOf(properties, PropertyId::Margin) == "1px 2px");
    CHECK(ValueOf(properties, PropertyId::Color) == "red");
    CHECK(ValueOf(properties, PropertyId::Width).empty());
    CHECK(ValueOf(properties, PropertyId::Padding).empty());
    CHECK(ToNarrow(properties->GetCustomPropertyValue(u8"--foo")) == "bar");
    CHECK(properties->GetCustomPropertyValue(u8"--bar").empty());

    SECTION("A shorthand set with a variable serialises its longhands as empty")
    {
      auto pending = ParseDeclarations(u8"margin: var(--x)");
      CHECK(ValueOf(pending, PropertyId::Margin) == "var(--x)");
      CHECK(ValueOf(pending, PropertyId::MarginTop).empty());
    }

    SECTION("System fonts")
    {
      auto font = ParseDeclarations(u8"font: caption");
      CHECK(ValueOf(font, PropertyId::Font) == "caption");
      CHECK_FALSE(font->GetPropertyCSSValue(PropertyId::FontSize));
      CHECK(ValueOf(font, PropertyId::FontSize).empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleProperties queries", "[CSS][Properties][StyleProperties]")
  {
    auto properties =
      ParseDeclarations(u8"margin: 1px !important; color: red; display: block; --foo: bar !important;"
                        u8"background: blue");

    SECTION("Importance")
    {
      CHECK(properties->PropertyIsImportant(PropertyId::MarginTop));
      CHECK(properties->PropertyIsImportant(PropertyId::Margin));
      CHECK_FALSE(properties->PropertyIsImportant(PropertyId::Color));
      CHECK_FALSE(properties->PropertyIsImportant(PropertyId::Width));
      CHECK_FALSE(properties->PropertyIsImportant(PropertyId::Padding));
      CHECK(properties->CustomPropertyIsImportant(u8"--foo"));
      CHECK_FALSE(properties->CustomPropertyIsImportant(u8"--bar"));
    }

    SECTION("Shorthand and implicit flags")
    {
      CHECK(ToNarrow(properties->GetPropertyShorthand(PropertyId::MarginTop)) == "margin");
      CHECK(properties->GetPropertyShorthand(PropertyId::Color).empty());
      CHECK(properties->GetPropertyShorthand(PropertyId::Width).empty());
      CHECK_FALSE(properties->IsPropertyImplicit(PropertyId::MarginTop));
      CHECK(properties->IsPropertyImplicit(PropertyId::BackgroundImage));
      CHECK_FALSE(properties->IsPropertyImplicit(PropertyId::BackgroundColor));
    }

    SECTION("Values")
    {
      CHECK(properties->PropertyAsValueId(PropertyId::Display) == ValueId::Block);
      CHECK(properties->PropertyAsValueId(PropertyId::BackgroundImage) == ValueId::None);
      CHECK_FALSE(properties->PropertyAsValueId(PropertyId::Width));
      CHECK(properties->PropertyAsColor(PropertyId::Color) == boo::Color(boo::Color::red));
      CHECK(properties->PropertyAsColor(PropertyId::BackgroundColor) == boo::Color(boo::Color::blue));
      CHECK_FALSE(properties->PropertyAsColor(PropertyId::Width));

      auto block = PrimitiveValue::Create(ValueId::Block);
      auto inlineValue = PrimitiveValue::Create(ValueId::Inline);
      CHECK(properties->PropertyMatches(PropertyId::Display, block.get()));
      CHECK_FALSE(properties->PropertyMatches(PropertyId::Display, inlineValue.get()));
      CHECK_FALSE(properties->PropertyMatches(PropertyId::Width, block.get()));
    }

    SECTION("Property references")
    {
      auto index = properties->FindPropertyIndex(PropertyId::Color);
      REQUIRE(index);
      auto reference = properties->PropertyAt(*index);
      CHECK(ToNarrow(reference.CssName()) == "color");
      CHECK(ToNarrow(reference.CssText(DefaultSerialisationContext())) == "color: red;");
      CHECK(reference.ToProperty().Id() == PropertyId::Color);

      auto custom = properties->FindCustomPropertyIndex(u8"--foo");
      REQUIRE(custom);
      CHECK(ToNarrow(properties->PropertyAt(*custom).CssText(DefaultSerialisationContext()))
            == "--foo: bar !important;");
      CHECK(ToNarrow(properties->PropertyAt(*custom).CssName()) == "--foo");
    }

    SECTION("Iteration")
    {
      auto count = 0u;
      for (auto property : *properties)
      {
        CHECK(property.Value());
        ++count;
      }

      CHECK(count == properties->PropertyCount());
      CHECK(properties->size() == properties->PropertyCount());
      CHECK_FALSE(properties->empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleProperties::MayDependOnBaseURL", "[CSS][Properties][StyleProperties]")
  {
    CHECK(ParseDeclarations(u8"background-image: url(a.png)")->MayDependOnBaseURL());
    CHECK(ParseDeclarations(u8"color: red; background: blue url(a.png)")->MayDependOnBaseURL());
    CHECK_FALSE(ParseDeclarations(u8"background-image: url(#a)")->MayDependOnBaseURL());
    CHECK_FALSE(ParseDeclarations(u8"color: red; width: 1px")->MayDependOnBaseURL());
    CHECK_FALSE(ParseDeclarations(u8"color: red")->HasCSSOMWrapper());
  }

  TEST_CASE_METHOD(ValueFixture, "MutableStyleProperties::SetProperty", "[CSS][Properties][StyleProperties]")
  {
    SECTION("A keyword")
    {
      auto properties = MutableStyleProperties::Create();
      CHECK(properties->SetProperty(PropertyId::Display, ValueId::Block));
      CHECK_FALSE(properties->SetProperty(PropertyId::Display, ValueId::Block));
      CHECK(properties->SetProperty(PropertyId::Display, ValueId::Flex, IsImportant(true)));
      CHECK(AsText(properties) == "display: flex !important;");
    }

    SECTION("A shorthand value sets every longhand")
    {
      auto properties = ParseDeclarations(u8"margin-top: 3px; color: red");
      properties->SetProperty(PropertyId::Margin, PrimitiveValue::Create(1.0, UnitType::px));
      CHECK(AsText(properties) == "color: red; margin: 1px;");
    }

    SECTION("Replacing a value keeps its position")
    {
      auto properties = ParseDeclarations(u8"width: 1px; color: red; height: 2px");
      CHECK(properties->SetProperty(PropertyId::Color, ValueId::Blue));
      CHECK(AsText(properties) == "width: 1px; color: blue; height: 2px;");
    }

    SECTION("Replacing a value in a logical property group moves it after a later mapping")
    {
      auto properties = ParseDeclarations(u8"margin-left: 1px; margin-inline-start: 2px");
      properties->SetProperty(Property(PropertyId::MarginLeft, PrimitiveValue::Create(3.0, UnitType::px)));
      CHECK(AsText(properties) == "margin-inline-start: 2px; margin-left: 3px;");

      properties->SetProperty(Property(PropertyId::MarginLeft, PrimitiveValue::Create(4.0, UnitType::px)));
      CHECK(AsText(properties) == "margin-inline-start: 2px; margin-left: 4px;");
    }

    SECTION("An important custom property isn't replaced by a normal one when merging")
    {
      auto properties = ParseDeclarations(u8"--foo: a !important");
      AddDeclaration(*properties, u8"--foo: b", DeclarationTestContext());
      CHECK(AsText(properties) == "--foo: a !important;");
      AddDeclaration(*properties, u8"--foo: c !important", DeclarationTestContext());
      CHECK(AsText(properties) == "--foo: c !important;");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "MutableStyleProperties::RemoveProperty",
                   "[CSS][Properties][StyleProperties]")
  {
    auto properties = ParseDeclarations(u8"margin: 1px; color: red; --foo: bar; width: 2px");

    CSSOMString text = u8"unchanged";
    CHECK(properties->RemoveProperty(PropertyId::Color, &text));
    CHECK(ToNarrow(text) == "red");

    CHECK_FALSE(properties->RemoveProperty(PropertyId::Color, &text));
    CHECK(text.empty());

    text = u8"unchanged";
    CHECK(properties->RemoveProperty(PropertyId::Margin, &text));
    CHECK(text.empty());
    CHECK_FALSE(properties->FindPropertyIndex(PropertyId::MarginTop));

    CHECK(properties->RemoveCustomProperty(u8"--foo", &text));
    CHECK(ToNarrow(text) == "bar");
    CHECK_FALSE(properties->RemoveCustomProperty(u8"--foo"));

    CHECK(AsText(properties) == "width: 2px;");

    const PropertyId toRemove[] {PropertyId::Width, PropertyId::Height};
    CHECK(properties->RemoveProperties(toRemove));
    CHECK_FALSE(properties->RemoveProperties(toRemove));
    CHECK(properties->empty());

    auto cleared = ParseDeclarations(u8"color: red");
    cleared->Clear();
    CHECK(cleared->empty());
  }

  TEST_CASE_METHOD(ValueFixture, "StyleProperties copies", "[CSS][Properties][StyleProperties]")
  {
    auto properties = ParseDeclarations(u8"margin: 1px; color: red !important; --foo: bar");

    SECTION("Immutable copies")
    {
      auto immutable = properties->ImmutableCopy();
      CHECK_FALSE(immutable->IsMutable());
      CHECK(immutable->PropertyCount() == properties->PropertyCount());
      CHECK(AsText(immutable) == AsText(properties));
      CHECK(immutable->PropertyIsImportant(PropertyId::Color));
      CHECK(ToNarrow(immutable->GetCustomPropertyValue(u8"--foo")) == "bar");
      CHECK(immutable->FindPropertyIndex(PropertyId::Color)
            == properties->FindPropertyIndex(PropertyId::Color));
      CHECK(immutable->FindCustomPropertyIndex(u8"--foo") == properties->FindCustomPropertyIndex(u8"--foo"));
      CHECK_FALSE(immutable->FindPropertyIndex(PropertyId::Width));
      CHECK_FALSE(immutable->FindCustomPropertyIndex(u8"--bar"));

      Ref<StyleProperties> base = immutable;
      CHECK(base->ImmutableCopyIfNeeded().get() == immutable.get());
    }

    SECTION("Deduplicated copies are shared")
    {
      constexpr utf8_stringview css = u8"margin: 1px; color: red !important";
      ImmutableStyleProperties::ClearDeduplicationMap();
      auto first = ParseDeclarations(css)->ImmutableCopyIfNeeded();
      auto second = ParseDeclarations(css)->ImmutableCopyIfNeeded();
      CHECK(first.get() == second.get());

      auto different = ParseDeclarations(u8"margin: 1px; color: red")->ImmutableCopyIfNeeded();
      CHECK(different.get() != first.get());

      auto quirks = ParseDeclarations(css, ParserMode::HTMLQuirks)->ImmutableCopyIfNeeded();
      CHECK(quirks.get() != first.get());

      ImmutableStyleProperties::ClearDeduplicationMap();
      auto third = ParseDeclarations(css)->ImmutableCopyIfNeeded();
      CHECK(third.get() != first.get());
      CHECK(AsText(third) == AsText(first));
    }

    SECTION("Mutable copies")
    {
      auto copy = properties->MutableCopy();
      CHECK(copy->IsMutable());
      CHECK(AsText(copy) == AsText(properties));
      copy->Clear();
      CHECK_FALSE(properties->empty());

      auto fromImmutable = properties->ImmutableCopy()->MutableCopy();
      CHECK(AsText(fromImmutable) == AsText(properties));
      CHECK(fromImmutable->ParserMode() == properties->ParserMode());
    }

    SECTION("Copying some properties")
    {
      const PropertyId subset[] {PropertyId::Color, PropertyId::MarginTop, PropertyId::Width};
      auto copy = properties->CopyProperties(subset);
      CHECK(AsText(copy) == "color: red; margin-top: 1px;");
    }

    SECTION("Merging")
    {
      auto target = ParseDeclarations(u8"color: blue; width: 2px");
      CHECK(target->MergeAndOverrideOnConflict(*properties));
      CHECK(AsText(target) == "color: red !important; width: 2px; margin: 1px; --foo: bar;");
      CHECK_FALSE(target->MergeAndOverrideOnConflict(*properties));
    }

    SECTION("Parser modes")
    {
      CHECK(MutableStyleProperties::Create()->ParserMode() == ParserMode::HTMLQuirks);
      CHECK(MutableStyleProperties::Create(ParserMode::UASheet)->ParserMode() == ParserMode::UASheet);
      CHECK(MutableStyleProperties::CreateEmpty()->ParserMode() == ParserMode::HTMLStandard);
      CHECK(MutableStyleProperties::CreateEmpty()->empty());
    }

    CHECK(StyleProperties::AverageSizeInBytes() == ImmutableStyleProperties::ObjectSize(4));
  }
}
