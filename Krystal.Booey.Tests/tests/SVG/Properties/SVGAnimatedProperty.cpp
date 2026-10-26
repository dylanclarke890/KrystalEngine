#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGAnimatedProperty: primitives", "[SVG][SVGAnimatedProperty]")
  {
    ResolverFixture fixture {u8"<svg><rect id=r /></svg>"};
    auto &rect = Downcast<SVGElement>(fixture.ById(u8"r"));

    auto number = SVGAnimatedNumber::Create(&rect, 1.5f);
    CHECK(number->IsAttached());
    CHECK(number->ContextElement() == &rect);
    CHECK(number->BaseVal() == 1.5f);
    CHECK(number->AnimVal() == 1.5f);
    CHECK(number->CurrentValue() == 1.5f);
    CHECK_FALSE(number->IsAnimating());

    // Parsing an attribute sets the base value without committing it.
    number->SetBaseValInternal(2);
    CHECK_FALSE(number->IsDirty());
    CHECK(number->AnimVal() == 2);

    // The DOM's change is committed to the element, which marks the property dirty.
    REQUIRE_FALSE(number->SetBaseVal(3).HasException());
    CHECK(number->IsDirty());
    CHECK(number->Synchronize() == dom::DOMString(u8"3"));
    CHECK_FALSE(number->IsDirty());
    CHECK(number->BaseValAsString() == u8"3");

    auto boolean = SVGAnimatedBoolean::Create(&rect);
    CHECK_FALSE(boolean->BaseVal());
    REQUIRE_FALSE(boolean->SetBaseVal(true).HasException());
    CHECK(boolean->Synchronize() == dom::DOMString(u8"true"));

    auto integer = SVGAnimatedInteger::Create(&rect, 7);
    CHECK(integer->BaseValAsString() == u8"7");

    auto string = SVGAnimatedString::Create(&rect);
    CHECK(string->BaseVal().empty());
    REQUIRE_FALSE(string->SetBaseVal(dom::DOMString {u8"#target"}).HasException());
    CHECK(string->Synchronize() == dom::DOMString(u8"#target"));

    // Detached from its element, a change goes nowhere.
    number->Detach();
    CHECK_FALSE(number->IsAttached());
    REQUIRE_FALSE(number->SetBaseVal(4).HasException());
    CHECK_FALSE(number->IsDirty());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGAnimatedProperty: values", "[SVG][SVGAnimatedProperty]")
  {
    ResolverFixture fixture {u8"<svg><rect id=r /></svg>"};
    auto &rect = Downcast<SVGElement>(fixture.ById(u8"r"));

    auto length = SVGAnimatedLength::Create(&rect, SVGLengthMode::Width);
    CHECK(length->BaseVal()->Value().LengthMode() == SVGLengthMode::Width);
    CHECK(length->BaseVal()->ContextElement() == &rect);
    CHECK(length->BaseVal()->IsAttached());

    // The animated value is a read-only copy that follows the base value.
    const auto &animVal = length->AnimVal();
    REQUIRE(animVal);
    CHECK(animVal->IsReadOnly());
    CHECK(animVal->SetValueAsString(u8"1px").GetException().Code()
          == dom::ExceptionCode::NoModificationAllowedError);

    REQUIRE_FALSE(length->BaseVal()->SetValueAsString(u8"10px").HasException());
    CHECK(animVal->ValueAsString() == u8"10px");
    CHECK(length->CurrentValue().ValueAsString() == u8"10px");
    CHECK(length->IsDirty());
    CHECK(length->Synchronize() == dom::DOMString(u8"10px"));

    length->SetBaseValInternal(SVGLengthValue(SVGLengthMode::Width, u8"5%"));
    CHECK(animVal->ValueAsString() == u8"5%");
    CHECK_FALSE(length->IsDirty());

    auto rectValue = SVGAnimatedRect::Create(&rect);
    REQUIRE_FALSE(rectValue->BaseVal()->SetWidth(20).HasException());
    CHECK(rectValue->BaseValAsString() == u8"0 0 20 0");
    CHECK(rectValue->AnimVal()->Width() == 20);

    auto angle = SVGAnimatedAngle::Create(&rect);
    REQUIRE_FALSE(angle->BaseVal()->SetValueAsString(u8"90deg").HasException());
    CHECK(angle->Synchronize() == dom::DOMString(u8"90deg"));

    auto preserveAspectRatio = SVGAnimatedPreserveAspectRatio::Create(&rect);
    REQUIRE_FALSE(preserveAspectRatio->BaseVal()
                    ->SetAlign(SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMIN)
                    .HasException());
    CHECK(preserveAspectRatio->BaseValAsString() == u8"xMinYMin meet");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGAnimatedProperty: lists", "[SVG][SVGAnimatedProperty]")
  {
    ResolverFixture fixture {u8"<svg><rect id=r /></svg>"};
    auto &rect = Downcast<SVGElement>(fixture.ById(u8"r"));

    auto lengths = SVGAnimatedLengthList::Create(&rect, SVGLengthMode::Height);
    CHECK(lengths->BaseVal()->LengthMode() == SVGLengthMode::Height);
    REQUIRE(lengths->BaseVal()->Parse(u8"1px 2px"));

    const auto &animVal = lengths->AnimVal();
    REQUIRE(animVal);
    CHECK(animVal->ValueAsString() == u8"1px 2px");
    CHECK(animVal->LengthMode() == SVGLengthMode::Height);
    CHECK(animVal->Clear().GetException().Code() == dom::ExceptionCode::NoModificationAllowedError);

    // A change to an item of the base value reaches the animated value and the element.
    REQUIRE_FALSE(lengths->BaseVal()->Items()[1]->SetValueAsString(u8"3em").HasException());
    CHECK(animVal->ValueAsString() == u8"1px 3em");
    CHECK(animVal->Items()[1]->IsReadOnly());
    CHECK(lengths->IsDirty());
    CHECK(lengths->Synchronize() == dom::DOMString(u8"1px 3em"));
    CHECK(&lengths->CurrentValue() == lengths->BaseVal().get());

    auto numbers = SVGAnimatedNumberList::Create(&rect);
    REQUIRE_FALSE(numbers->BaseVal()->AppendItem(SVGNumber::Create(4)).HasException());
    CHECK(numbers->AnimVal()->ValueAsString() == u8"4");

    auto points = SVGAnimatedPointList::Create(&rect);
    REQUIRE(points->BaseVal()->Parse(u8"1 2 3 4"));
    CHECK(points->AnimVal()->NumberOfItems() == 2);

    auto transforms = SVGAnimatedTransformList::Create(&rect);
    transforms->BaseVal()->Parse(u8"translate(1 2)");
    REQUIRE_FALSE(transforms->BaseVal()->Items()[0]->Matrix().SetA(2).HasException());
    CHECK(transforms->IsDirty());
    CHECK(transforms->AnimVal()->ValueAsString() == u8"matrix(2 0 0 1 1 2)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGAnimatedProperty: enumerations",
                   "[SVG][SVGAnimatedProperty]")
  {
    ResolverFixture fixture {u8"<svg><rect id=r /></svg>"};
    auto &rect = Downcast<SVGElement>(fixture.ById(u8"r"));

    auto units = SVGAnimatedEnumeration::Create(&rect, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(units->BaseVal() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(units->AnimVal() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(units->CurrentValue<SVGUnitTypes::SVGUnitType>() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(units->BaseValAsString() == u8"userSpaceOnUse");

    // The DOM can't set 0 or a value beyond the enumeration.
    CHECK(units->SetBaseVal(0).GetException().Code() == dom::ExceptionCode::TypeError);
    CHECK(units->SetBaseVal(3).GetException().Code() == dom::ExceptionCode::TypeError);
    CHECK_FALSE(units->IsDirty());

    REQUIRE_FALSE(units->SetBaseVal(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX).HasException());
    CHECK(units->IsDirty());
    CHECK(units->Synchronize() == dom::DOMString(u8"objectBoundingBox"));
    CHECK_FALSE(units->Synchronize());

    // Orientations are enumerations too, set to auto-start-reverse by parsing.
    auto orient = SVGAnimatedOrientType::Create(&rect, SVGMarkerOrientAngle);
    CHECK(orient->BaseVal() == SVGMarkerOrientAngle);
    orient->SetBaseValInternal(SVGMarkerOrientAutoStartReverse);
    CHECK(orient->BaseVal() == SVGMarkerOrientAutoStartReverse);
    CHECK(orient->BaseValAsString() == u8"auto-start-reverse");
  }
}
