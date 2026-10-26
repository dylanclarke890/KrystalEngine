#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLength: unit types and values", "[SVG][SVGLength]")
  {
    auto length = SVGLength::Create();
    CHECK(length->UnitType() == SVGLength::SVG_LENGTHTYPE_NUMBER);
    CHECK(length->ValueAsString() == u8"0");

    REQUIRE_FALSE(length->SetValueAsString(u8"2.5cm").HasException());
    CHECK(length->UnitType() == SVGLength::SVG_LENGTHTYPE_CM);
    CHECK(length->ValueInSpecifiedUnits() == 2.5f);

    // Units SVG's interface doesn't expose are unknown to it.
    REQUIRE_FALSE(length->SetValueAsString(u8"3ch").HasException());
    CHECK(length->UnitType() == SVGLength::SVG_LENGTHTYPE_UNKNOWN);
    CHECK(length->ValueAsString() == u8"3ch");

    CHECK(length->SetValueAsString(u8"3 px").GetException().Code() == dom::ExceptionCode::SyntaxError);

    REQUIRE_FALSE(length->NewValueSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PX, 12).HasException());
    CHECK(length->ValueAsString() == u8"12px");
    CHECK(length->NewValueSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_UNKNOWN, 1).GetException().Code()
          == dom::ExceptionCode::NotSupportedError);
    CHECK(length->NewValueSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PC + 1, 1).GetException().Code()
          == dom::ExceptionCode::NotSupportedError);

    auto value = length->ValueForBindings();
    REQUIRE_FALSE(value.HasException());
    CHECK(value.Value() == 12);

    REQUIRE_FALSE(length->SetValueForBindings(24).HasException());
    CHECK(length->ValueAsString() == u8"24px");

    REQUIRE_FALSE(length->ConvertToSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PT).HasException());
    CHECK(length->ValueAsString() == u8"18pt");
    CHECK(length->ConvertToSpecifiedUnits(0).GetException().Code() == dom::ExceptionCode::NotSupportedError);

    REQUIRE_FALSE(length->SetValueInSpecifiedUnits(9).HasException());
    CHECK(length->ValueAsString() == u8"9pt");

    auto clone = length->Clone();
    CHECK(clone.get() != length.get());
    CHECK(clone->ValueAsString() == u8"9pt");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLength: owners and read-only lengths", "[SVG][SVGLength]")
  {
    RecordingPropertyOwner owner;
    auto length =
      SVGLength::Create(&owner, SVGPropertyAccess::ReadWrite, SVGLengthValue(SVGLengthMode::Width));
    CHECK(length->IsAttached());

    REQUIRE_FALSE(length->SetValueAsString(u8"5px").HasException());
    REQUIRE_FALSE(length->NewValueSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_MM, 2).HasException());
    REQUIRE(owner.Committed.size() == 2);
    CHECK(owner.Committed[0] == length.get());

    // A failed change isn't committed.
    CHECK(length->SetValueAsString(u8"oops").HasException());
    CHECK(owner.Committed.size() == 2);

    // The owner marks the property dirty; synchronising gives the attribute's new value once.
    length->SetDirty();
    CHECK(length->Synchronize() == dom::DOMString(u8"2mm"));
    CHECK_FALSE(length->Synchronize());

    auto readOnly = SVGLength::Create(&owner, SVGPropertyAccess::ReadOnly);
    auto code = dom::ExceptionCode::NoModificationAllowedError;
    CHECK(readOnly->SetValueAsString(u8"1px").GetException().Code() == code);
    CHECK(readOnly->SetValueForBindings(1).GetException().Code() == code);
    CHECK(readOnly->SetValueInSpecifiedUnits(1).GetException().Code() == code);
    CHECK(readOnly->NewValueSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PX, 1).GetException().Code() == code);
    CHECK(readOnly->ConvertToSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PX).GetException().Code() == code);
    CHECK(owner.Committed.size() == 2);

    length->Detach();
    CHECK_FALSE(length->IsAttached());
    CHECK(length->Access() == SVGPropertyAccess::ReadWrite);
  }
}
