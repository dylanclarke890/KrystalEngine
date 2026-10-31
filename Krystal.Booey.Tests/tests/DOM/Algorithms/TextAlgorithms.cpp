#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include <limits>

namespace krys::boo::dom::tests
{
  namespace
  {
    struct ReplaceTestData
    {
      Ref<dom::Document> Document;
      Ref<Text> Node;

      ReplaceTestData()
          : Document(CreateRef<dom::Document>()), Node(CreateRef<Text>(*Document, u8"0123456789"))
      {
      }

      /// @brief A collapsed live range at the offset in the node.
      Ref<Range> RangeAt(size_t offset) noexcept
      {
        auto range = Document->CreateRange();
        REQUIRE_FALSE(range->SetStart(*Node, offset).HasException());
        REQUIRE_FALSE(range->SetEnd(*Node, offset).HasException());
        return range;
      }
    };
  }

  TEST_CASE("Replace data: inserts the data at the offset and removes count code units after it",
            "[DOM][TextAlgorithms][dom:concept-cd-replace][dom:concept-cd-data]")
  {
    ReplaceTestData data;

    SECTION("Inside the data")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 2, 3, u8"ab").HasException());
      CHECK(data.Node->Data() == u8"01ab56789");
    }

    SECTION("A count past the end is clamped to the end")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 8, 5, u8"x").HasException());
      CHECK(data.Node->Data() == u8"01234567x");
    }

    SECTION("At the end")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 10, 0, u8"x").HasException());
      CHECK(data.Node->Data() == u8"0123456789x");
    }

    SECTION("An offset past the end throws an IndexSizeError, changing nothing")
    {
      auto result = TextAlgorithms::Replace(*data.Node, 11, 0, u8"x");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::IndexSizeError);
      CHECK(data.Node->Data() == u8"0123456789");
    }
  }

  TEST_CASE("Replace data: a count that overflows offset + count is clamped too",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto range = data.RangeAt(5);

    REQUIRE_FALSE(
      TextAlgorithms::Replace(*data.Node, 1, std::numeric_limits<size_t>::max(), u8"").HasException());

    CHECK(data.Node->Data() == u8"0");
    CHECK(range->StartOffset() == 1uz);
    CHECK(range->EndOffset() == 1uz);
  }

  TEST_CASE("Replace data: queues a characterData record with the old data",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    REQUIRE_FALSE(
      observer
        ->Observe(*data.Node, MutationObserverInit {.CharacterData = true, .CharacterDataOldValue = true})
        .HasException());

    REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 0, 1, u8"x").HasException());
    auto failed = TextAlgorithms::Replace(*data.Node, 20, 0, u8"y");
    REQUIRE(failed.HasException());

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->Type() == u8"characterData");
    CHECK(records[0]->Target().get() == data.Node.get());
    CHECK(records[0]->OldValue() == u8"0123456789");
  }

  TEST_CASE("Replace data: moves live range boundaries in the node",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto before = data.RangeAt(2);
    auto atOffset = data.RangeAt(3);
    auto inside = data.RangeAt(4);
    auto atEnd = data.RangeAt(6);
    auto after = data.RangeAt(8);

    // Replaces "345" with "abcdef".
    REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 3, 3, u8"abcdef").HasException());

    CHECK(before->StartOffset() == 2uz);
    CHECK(atOffset->StartOffset() == 3uz);
    // Greater than offset, up to offset + count: moved to the offset.
    CHECK(inside->StartOffset() == 3uz);
    CHECK(inside->EndOffset() == 3uz);
    CHECK(atEnd->StartOffset() == 3uz);
    // Past offset + count: shifted by the data's length less count.
    CHECK(after->StartOffset() == 11uz);
    CHECK(after->EndOffset() == 11uz);

    SECTION("Each boundary is moved on its own")
    {
      auto range = data.Document->CreateRange();
      REQUIRE_FALSE(range->SetStart(*data.Node, 1).HasException());
      REQUIRE_FALSE(range->SetEnd(*data.Node, 12).HasException());
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 0, 2, u8"").HasException());
      CHECK(range->StartOffset() == 0uz);
      CHECK(range->EndOffset() == 10uz);
    }
  }

  TEST_CASE("Substring data: count code units from the offset, to the end at most",
            "[DOM][TextAlgorithms][dom:concept-cd-substring]")
  {
    ReplaceTestData data;
    CHECK(TextAlgorithms::Substring(*data.Node, 2, 3).Value() == u8"234");
    CHECK(TextAlgorithms::Substring(*data.Node, 7, 5).Value() == u8"789");
    CHECK(TextAlgorithms::Substring(*data.Node, 1, std::numeric_limits<size_t>::max()).Value()
          == u8"123456789");
    CHECK(TextAlgorithms::Substring(*data.Node, 10, 1).Value() == u8"");

    auto result = TextAlgorithms::Substring(*data.Node, 11, 0);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::IndexSizeError);
  }
}
