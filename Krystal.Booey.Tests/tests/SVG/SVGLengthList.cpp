#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthList: parsing and serialising", "[SVG][SVGLengthList]")
  {
    auto list = SVGLengthList::Create(SVGLengthMode::Width);
    CHECK(list->LengthMode() == SVGLengthMode::Width);

    CHECK(list->Parse(u8"  10px, 20%   3em,4 "));
    REQUIRE(list->NumberOfItems() == 4);
    CHECK(list->Items()[1]->UnitType() == SVGLength::SVG_LENGTHTYPE_PERCENTAGE);
    CHECK(list->Items()[1]->Value().LengthMode() == SVGLengthMode::Width);
    CHECK(list->ValueAsString() == u8"10px 20% 3em 4");

    // Parsing stops at the first bad length, keeping those before it.
    CHECK_FALSE(list->Parse(u8"1px 2qq 3px"));
    CHECK(list->ValueAsString() == u8"1px");

    CHECK(list->Parse(u8""));
    CHECK(list->IsEmpty());
    CHECK(list->ValueAsString() == u8"");

    // The values of the items, as a list.
    CHECK(list->Parse(u8"1 2"));
    List<SVGLengthValue> values = *list;
    CHECK(values.size() == 2);
    CHECK(values[1].ValueAsString() == u8"2");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthList: the list interface", "[SVG][SVGLengthList]")
  {
    RecordingPropertyOwner owner;
    auto list = SVGLengthList::Create(&owner, SVGPropertyAccess::ReadWrite, SVGLengthMode::Other);
    REQUIRE(list->Parse(u8"1px 2px"));

    auto appended = list->AppendItem(SVGLength::Create(SVGLengthValue(SVGLengthMode::Other, u8"3px")));
    REQUIRE_FALSE(appended.HasException());
    CHECK(appended.Value()->IsAttached());
    CHECK(list->Length() == 3);
    REQUIRE(owner.Committed.size() == 1);
    CHECK(owner.Committed[0] == list.get());

    // An item in a list commits its changes through the list.
    REQUIRE_FALSE(list->Items()[0]->SetValueAsString(u8"7px").HasException());
    REQUIRE(owner.Committed.size() == 2);
    CHECK(owner.Committed[1] == list.get());
    CHECK(list->ValueAsString() == u8"7px 2px 3px");

    // An item already in a list is cloned rather than moved.
    auto other = SVGLengthList::Create();
    auto item = list->GetItem(1);
    REQUIRE_FALSE(item.HasException());
    auto inserted = other->InsertItemBefore(Ref {item.Value()}, 5);
    REQUIRE_FALSE(inserted.HasException());
    CHECK(inserted.Value().get() != item.Value().get());
    CHECK(other->ValueAsString() == u8"2px");
    CHECK(list->Length() == 3);

    auto replaced = list->ReplaceItem(SVGLength::Create(SVGLengthValue(SVGLengthMode::Other, u8"9px")), 0);
    REQUIRE_FALSE(replaced.HasException());
    CHECK(list->ValueAsString() == u8"9px 2px 3px");

    auto removed = list->RemoveItem(2);
    REQUIRE_FALSE(removed.HasException());
    CHECK_FALSE(removed.Value()->IsAttached());
    CHECK(list->ValueAsString() == u8"9px 2px");

    CHECK(list->GetItem(2).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(list->RemoveItem(2).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(list->ReplaceItem(SVGLength::Create(), 2).GetException().Code()
          == dom::ExceptionCode::IndexSizeError);
    CHECK(list->IsSupportedPropertyIndex(1));
    CHECK_FALSE(list->IsSupportedPropertyIndex(2));

    auto initialised = list->Initialize(SVGLength::Create(SVGLengthValue(SVGLengthMode::Other, u8"4em")));
    REQUIRE_FALSE(initialised.HasException());
    CHECK(list->ValueAsString() == u8"4em");

    REQUIRE_FALSE(list->Clear().HasException());
    CHECK(list->IsEmpty());

    // A read-only copy, as an animated list's animVal is.
    REQUIRE(list->Parse(u8"1px 2px"));
    auto readOnly = SVGLengthList::Create(*list, SVGPropertyAccess::ReadOnly);
    CHECK(readOnly->ValueAsString() == u8"1px 2px");
    CHECK(readOnly->Items()[0]->IsReadOnly());
    auto code = dom::ExceptionCode::NoModificationAllowedError;
    CHECK(readOnly->Clear().GetException().Code() == code);
    CHECK(readOnly->AppendItem(SVGLength::Create()).GetException().Code() == code);
    CHECK(readOnly->Items()[0]->SetValueAsString(u8"3px").GetException().Code() == code);

    // Copying a list copies its values into new items.
    *readOnly = *SVGLengthList::Create();
    CHECK(readOnly->IsEmpty());
    readOnly->Resize(2);
    CHECK(readOnly->ValueAsString() == u8"0 0");
  }
}
