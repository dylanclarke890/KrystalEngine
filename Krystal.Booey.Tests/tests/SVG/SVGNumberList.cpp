#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE("SVGNumberList: parsing and serialising", "[SVG][SVGNumberList]")
  {
    auto list = SVGNumberList::Create();
    CHECK(list->Parse(u8" 1, 2.5 -3e1 .5"));
    CHECK(list->ValueAsString() == u8"1 2.5 -30 0.5");

    CHECK_FALSE(list->Parse(u8"1 2 x"));
    CHECK(list->ValueAsString() == u8"1 2");

    List<float> values = *list;
    CHECK(values == List<float> {1, 2});
  }

  TEST_CASE("SVGNumber: values and owners", "[SVG][SVGNumber]")
  {
    RecordingPropertyOwner owner;
    auto number = SVGNumber::Create(&owner, SVGPropertyAccess::ReadWrite, 1.5f);
    CHECK(number->ValueForBindings() == 1.5f);
    CHECK(number->ValueAsString() == u8"1.5");

    REQUIRE_FALSE(number->SetValueForBindings(4).HasException());
    CHECK(number->ValueAsString() == u8"4");
    CHECK(owner.Committed.size() == 1);

    auto readOnly = SVGNumber::Create(&owner, SVGPropertyAccess::ReadOnly);
    CHECK(readOnly->SetValueForBindings(1).GetException().Code()
          == dom::ExceptionCode::NoModificationAllowedError);
    CHECK(SVGNumber::Create(2)->Clone()->ValueForBindings() == 2);
  }
}
