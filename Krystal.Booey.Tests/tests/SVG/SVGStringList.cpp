#include "Krystal.Booey/SVG/SVGStringList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE("SVGStringList: parsing, resetting and the list interface", "[SVG][SVGStringList]")
  {
    RecordingPropertyOwner owner;
    auto list = SVGStringList::Create(&owner);

    CHECK(list->Parse(u8" a,b  ,c d", u8','));
    REQUIRE(list->NumberOfItems() == 4);
    CHECK(list->Items()[2] == u8"c");
    CHECK(list->ValueAsString() == u8"a b c d");

    // Resetting splits on spaces, and keeps an empty string when there is nothing else.
    list->Reset(u8"x y");
    CHECK(list->ValueAsString() == u8"x y");
    list->Reset(u8"   ");
    REQUIRE(list->NumberOfItems() == 1);
    CHECK(list->Items()[0].empty());

    auto appended = list->AppendItem(dom::DOMString {u8"z"});
    REQUIRE_FALSE(appended.HasException());
    CHECK(appended.Value() == u8"z");
    REQUIRE(owner.Committed.size() == 1);
    CHECK(owner.Committed[0] == list.get());

    auto inserted = list->InsertItemBefore(dom::DOMString {u8"w"}, 0);
    REQUIRE_FALSE(inserted.HasException());
    CHECK(list->ValueAsString() == u8"w  z");

    REQUIRE_FALSE(list->SetItem(1, dom::DOMString {u8"v"}).HasException());
    CHECK(list->ValueAsString() == u8"w v z");

    auto removed = list->RemoveItem(0);
    REQUIRE_FALSE(removed.HasException());
    CHECK(removed.Value() == u8"w");
    CHECK(list->ValueAsString() == u8"v z");
    CHECK(owner.Committed.size() == 4);
  }
}
