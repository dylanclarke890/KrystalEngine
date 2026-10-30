#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/TestSVGElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyOwnerRegistry: lookups",
                   "[SVG][SVGPropertyOwnerRegistry]")
  {
    auto document = ResolverFixture::Parse(u8"<svg></svg>");
    auto element = CreateTestSVGElement(*document);
    using Registry = TestSVGElement::PropertyRegistry;

    // The conditional processing attributes are registered when an element first has them.
    (void)element->SVGElement::ConditionalProcessingAttributes();

    // An owner knows the attributes of its own properties; its registry reaches those of its bases too.
    CHECK(Registry::IsKnownAttribute(SVGNames::XAttr()));
    CHECK(Registry::IsKnownAttribute(TestAttributeName(u8"order")));
    CHECK_FALSE(Registry::IsKnownAttribute(SVGNames::ClassAttr()));
    CHECK(SVGElement::PropertyRegistry::IsKnownAttribute(SVGNames::ClassAttr()));
    CHECK(SVGTests::PropertyRegistry::IsKnownAttribute(SVGNames::SystemLanguageAttr()));
    CHECK(SVGURIReference::PropertyRegistry::IsKnownAttribute(SVGNames::HrefAttr()));

    // Attributes are matched by their namespace and local name, whatever their prefix.
    dom::QualifiedName unprefixedXLinkHref {dom::DOMStringAtom(infra::Namespaces::XLink),
                                            dom::DOMStringAtom::Null(), dom::DOMStringAtom(u8"href")};
    CHECK(SVGURIReference::PropertyRegistry::IsKnownAttribute(unprefixedXLinkHref));
    CHECK_FALSE(SVGURIReference::PropertyRegistry::IsKnownAttribute(TestAttributeName(u8"xlink:href")));

    CHECK(Registry::IsAnimatedLengthAttribute(SVGNames::XAttr()));
    CHECK_FALSE(Registry::IsAnimatedLengthAttribute(TestAttributeName(u8"k1")));
    CHECK_FALSE(Registry::IsAnimatedLengthAttribute(TestAttributeName(u8"lengths")));

    const auto &registry = element->GetPropertyRegistry();
    CHECK(registry.IsAnimatedPropertyAttribute(SVGNames::XAttr()));
    CHECK(registry.IsAnimatedPropertyAttribute(TestAttributeName(u8"order")));
    CHECK(registry.IsAnimatedPropertyAttribute(SVGNames::ClassAttr()));
    CHECK(registry.IsAnimatedPropertyAttribute(XLinkNames::HrefAttr()));
    CHECK_FALSE(registry.IsAnimatedPropertyAttribute(SVGNames::SystemLanguageAttr()));
    CHECK_FALSE(registry.IsAnimatedPropertyAttribute(TestAttributeName(u8"viewTransform")));
    CHECK_FALSE(registry.IsAnimatedPropertyAttribute(TestAttributeName(u8"bogus")));
    CHECK(element->IsAnimatedPropertyAttribute(TestAttributeName(u8"transform")));

    // Only the geometry lengths that are also CSS properties are style attributes.
    CHECK(registry.IsAnimatedStylePropertyAttribute(SVGNames::XAttr()));
    CHECK_FALSE(registry.IsAnimatedStylePropertyAttribute(TestAttributeName(u8"dx")));
    CHECK_FALSE(registry.IsAnimatedStylePropertyAttribute(SVGNames::ClassAttr()));

    CHECK(registry.AnimatedPropertyAttributeName(*element->X) == SVGNames::XAttr());
    CHECK(registry.AnimatedPropertyAttributeName(*element->OrderY) == TestAttributeName(u8"order"));
    CHECK(registry.AnimatedPropertyAttributeName(*element->OrientType) == TestAttributeName(u8"orient"));
    CHECK(registry.AnimatedPropertyAttributeName(*element->Units) == TestAttributeName(u8"filterUnits"));
    CHECK(registry.AnimatedPropertyAttributeName(element->ClassNameAnimated()) == SVGNames::ClassAttr());
    // href is reflected from either attribute; the first registered names it.
    CHECK(registry.AnimatedPropertyAttributeName(element->HrefAnimated()) == SVGNames::HrefAttr());
    CHECK(registry.PropertyAttributeName(*element->ViewTransform) == TestAttributeName(u8"viewTransform"));
    CHECK(registry.PropertyAttributeName(element->SystemLanguage()) == SVGNames::SystemLanguageAttr());
    CHECK(registry.PropertyAttributeName(element->RequiredExtensions())
          == SVGNames::RequiredExtensionsAttr());

    auto unregistered = SVGAnimatedNumber::Create(element.get());
    CHECK(registry.AnimatedPropertyAttributeName(*unregistered) == dom::NullQualifiedName());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyOwnerRegistry: attributes parsed",
                   "[SVG][SVGPropertyOwnerRegistry]")
  {
    auto document = ResolverFixture::Parse(u8"<svg></svg>");
    auto element = CreateTestSVGElement(*document);

    REQUIRE_FALSE(element->SetAttribute(u8"x", u8"5px").HasException());
    CHECK(element->X->BaseVal()->ValueAsString() == u8"5px");
    CHECK_FALSE(element->X->IsDirty());
    REQUIRE_FALSE(element->ChangedAttributes.empty());
    CHECK(element->ChangedAttributes.back() == SVGNames::XAttr());

    REQUIRE_FALSE(element->SetAttribute(u8"transform", u8"scale(2)").HasException());
    CHECK(element->Transform->BaseVal()->ValueAsString() == u8"scale(2 2)");

    // A change to the style attribute isn't one to the element's properties.
    element->ChangedAttributes.clear();
    REQUIRE_FALSE(element->SetAttribute(u8"style", u8"fill: red").HasException());
    CHECK(element->ChangedAttributes.empty());

    SECTION("className")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"class", u8"a b").HasException());
      CHECK(element->ClassNameAnimated().BaseVal() == u8"a b");
      CHECK(element->ClassName() == u8"a b");
      CHECK(element->ChangedAttributes.back() == SVGNames::ClassAttr());

      element->RemoveAttribute(u8"class");
      CHECK(element->ClassNameAnimated().BaseVal().empty());
    }

    SECTION("href")
    {
      auto xlink = dom::DOMStringAtom(infra::Namespaces::XLink);
      REQUIRE_FALSE(element->SetAttributeNS(xlink, u8"xlink:href", u8"#a").HasException());
      CHECK(element->Href() == u8"#a");

      // href takes precedence over xlink:href.
      REQUIRE_FALSE(element->SetAttribute(u8"href", u8"#b").HasException());
      CHECK(element->Href() == u8"#b");
      REQUIRE_FALSE(element->SetAttributeNS(xlink, u8"xlink:href", u8"#c").HasException());
      CHECK(element->Href() == u8"#b");
      element->RemoveAttribute(u8"href");
      CHECK(element->Href() == u8"#c");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyOwnerRegistry: changes written to attributes",
                   "[SVG][SVGPropertyOwnerRegistry]")
  {
    auto document = ResolverFixture::Parse(u8"<svg></svg>");
    auto element = CreateTestSVGElement(*document);

    // A change to a style property is written to its attribute at once.
    REQUIRE_FALSE(element->X->BaseVal()->SetValueAsString(u8"7px").HasException());
    CHECK_FALSE(element->X->IsDirty());
    CHECK(element->ChangedAttributes.back() == SVGNames::XAttr());
    CHECK(element->GetAttribute(u8"x") == dom::DOMString(u8"7px"));

    // Other changes are written when the attributes are next read.
    REQUIRE_FALSE(element->Dx->BaseVal()->SetValueAsString(u8"3px").HasException());
    CHECK(element->Dx->IsDirty());
    CHECK(element->ChangedAttributes.back() == TestAttributeName(u8"dx"));
    CHECK(element->GetAttribute(u8"dx") == dom::DOMString(u8"3px"));
    CHECK_FALSE(element->Dx->IsDirty());

    REQUIRE_FALSE(element->Boolean->SetBaseVal(true).HasException());
    REQUIRE_FALSE(element->Integer->SetBaseVal(3).HasException());
    REQUIRE_FALSE(element->Number->SetBaseVal(0.5f).HasException());
    REQUIRE_FALSE(element->Units->SetBaseVal(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX).HasException());
    REQUIRE_FALSE(element->Lengths->BaseVal()->AppendItem(SVGLength::Create()).HasException());
    REQUIRE_FALSE(element->Numbers->BaseVal()->AppendItem(SVGNumber::Create(4)).HasException());
    REQUIRE_FALSE(element->Points->BaseVal()->AppendItem(SVGPoint::Create({1, 2})).HasException());
    REQUIRE_FALSE(element->PreserveAspectRatio->BaseVal()
                    ->SetMeetOrSlice(SVGPreserveAspectRatioValue::SVG_MEETORSLICE_SLICE)
                    .HasException());
    REQUIRE_FALSE(element->ViewBox->BaseVal()->SetWidth(10).HasException());
    REQUIRE_FALSE(element->String->SetBaseVal(dom::DOMString {u8"out"}).HasException());
    REQUIRE_FALSE(element->Transform->BaseVal()
                    ->AppendItem(SVGTransform::Create(SVGTransformValue::TranslateTransformValue({1, 2})))
                    .HasException());
    // A property that isn't animated commits itself.
    REQUIRE_FALSE(
      element->ViewTransform->AppendItem(SVGTransform::Create(SVGTransformValue::ScaleTransformValue({2, 2})))
        .HasException());
    REQUIRE_FALSE(element->SystemLanguage().AppendItem(dom::DOMString {u8"en"}).HasException());
    REQUIRE_FALSE(element->ClassNameAnimated().SetBaseVal(dom::DOMString {u8"c"}).HasException());
    REQUIRE_FALSE(element->HrefAnimated().SetBaseVal(dom::DOMString {u8"#d"}).HasException());

    CHECK(element->GetAttribute(u8"preserveAlpha") == dom::DOMString(u8"true"));
    CHECK(element->GetAttribute(u8"numOctaves") == dom::DOMString(u8"3"));
    CHECK(element->GetAttribute(u8"k1") == dom::DOMString(u8"0.5"));
    CHECK(element->GetAttribute(u8"filterUnits") == dom::DOMString(u8"objectBoundingBox"));
    CHECK(element->GetAttribute(u8"lengths") == dom::DOMString(u8"0"));
    CHECK(element->GetAttribute(u8"values") == dom::DOMString(u8"4"));
    CHECK(element->GetAttribute(u8"points") == dom::DOMString(u8"1 2"));
    CHECK(element->GetAttribute(u8"preserveAspectRatio") == dom::DOMString(u8"xMidYMid slice"));
    CHECK(element->GetAttribute(u8"viewBox") == dom::DOMString(u8"0 0 10 0"));
    CHECK(element->GetAttribute(u8"result") == dom::DOMString(u8"out"));
    CHECK(element->GetAttribute(u8"transform") == dom::DOMString(u8"translate(1 2)"));
    CHECK(element->GetAttribute(u8"viewTransform") == dom::DOMString(u8"scale(2 2)"));
    CHECK(element->GetAttribute(u8"systemLanguage") == dom::DOMString(u8"en"));
    CHECK(element->GetAttribute(u8"href") == dom::DOMString(u8"#d"));

    // The classes are read from the attribute, so className is written to it at once.
    CHECK(element->HasClassName(u8"c"));
    CHECK(element->GetAttribute(u8"class") == dom::DOMString(u8"c"));

    // Written attributes don't come back to the element as changes.
    CHECK(element->Href() == u8"#d");
    CHECK(element->Transform->BaseVal()->NumberOfItems() == 1);

    SECTION("Pairs")
    {
      // Two integers or numbers are written as one if they're equal.
      REQUIRE_FALSE(element->OrderY->SetBaseVal(2).HasException());
      CHECK(element->GetAttribute(u8"order") == dom::DOMString(u8"1, 2"));
      REQUIRE_FALSE(element->StdDeviationX->SetBaseVal(2).HasException());
      REQUIRE_FALSE(element->StdDeviationY->SetBaseVal(2).HasException());
      CHECK(element->GetAttribute(u8"stdDeviation") == dom::DOMString(u8"2"));

      // An orientation is an angle or a keyword; setting one resets the other.
      REQUIRE_FALSE(element->OrientAngle->BaseVal()->SetValueAsString(u8"45deg").HasException());
      CHECK(element->GetAttribute(u8"orient") == dom::DOMString(u8"45deg"));
      REQUIRE_FALSE(element->OrientType->SetBaseVal(SVGMarkerOrientAuto).HasException());
      CHECK(element->OrientAngle->BaseVal()->ValueAsString() == u8"0");
      CHECK(element->GetAttribute(u8"orient") == dom::DOMString(u8"auto"));
      REQUIRE_FALSE(element->OrientAngle->BaseVal()->SetValueAsString(u8"10deg").HasException());
      CHECK(element->OrientType->BaseVal() == SVGMarkerOrientAngle);
      CHECK(element->GetAttribute(u8"orient") == dom::DOMString(u8"10deg"));
    }

    SECTION("Detached")
    {
      element->DetachAllProperties();
      CHECK_FALSE(element->X->IsAttached());
      CHECK_FALSE(element->OrderX->IsAttached());
      CHECK_FALSE(element->OrientType->IsAttached());
      CHECK_FALSE(element->ClassNameAnimated().IsAttached());
      CHECK_FALSE(element->HrefAnimated().IsAttached());
      CHECK(element->ViewTransform->ContextElement() == nullptr);
      CHECK(element->SystemLanguage().ContextElement() == nullptr);

      // A detached property's changes go nowhere.
      REQUIRE_FALSE(element->Dx->BaseVal()->SetValueAsString(u8"9px").HasException());
      CHECK(element->GetAttribute(u8"dx") == dom::DOMString(u8"3px"));
    }
  }
}
