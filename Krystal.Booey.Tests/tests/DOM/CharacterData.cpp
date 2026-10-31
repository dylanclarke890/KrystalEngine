#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<dom::Text> Node;
      Ref<dom::Range> FullRange;
      Ref<dom::Range> PartialRange;

      CommonTestData()
          : Document(CreateRef<dom::Document>()), Node(CreateRef<dom::Text>(*Document, u8"Hello, world!")),
            FullRange(Document->CreateRange()), PartialRange(Document->CreateRange())
      {
        REQUIRE_FALSE(FullRange->SetStart(*Node, 0uz).HasException());
        REQUIRE_FALSE(FullRange->SetEnd(*Node, Node->Length()).HasException());

        // "lo, "
        REQUIRE_FALSE(PartialRange->SetStart(*Node, 3uz).HasException());
        REQUIRE_FALSE(PartialRange->SetEnd(*Node, 7uz).HasException());
      }

      void CheckRanges(size_t fullEnd, size_t partialStart, size_t partialEnd) const
      {
        CHECK(FullRange->StartOffset() == 0uz);
        CHECK(FullRange->EndOffset() == fullEnd);
        CHECK(PartialRange->StartOffset() == partialStart);
        CHECK(PartialRange->EndOffset() == partialEnd);
      }

      Ref<MutationObserver> Observe() const
      {
        auto observer =
          CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
        REQUIRE_FALSE(
          observer
            ->Observe(*Node, MutationObserverInit {.CharacterData = true, .CharacterDataOldValue = true})
            .HasException());
        return observer;
      }
    };
  }

  TEST_CASE(
    "CharacterData.data: its data, which setting replaces all of",
    "[DOM][CharacterData][dom:interface-characterdata][dom:dom-characterdata-data][dom:concept-cd-data]")
  {
    CommonTestData data;
    CHECK(data.Node->Data() == u8"Hello, world!");
    auto observer = data.Observe();

    REQUIRE_FALSE(data.Node->Data(u8"New value").HasException());
    CHECK(data.Node->Data() == u8"New value");
    data.CheckRanges(0, 0, 0);

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->OldValue() == u8"Hello, world!");

    SECTION("Comments and processing instructions too")
    {
      auto comment = data.Document->CreateComment(u8"a");
      REQUIRE_FALSE(comment->Data(u8"b").HasException());
      CHECK(comment->Data() == u8"b");
      auto pi = data.Document->CreateProcessingInstruction(u8"x", u8"a").ReleaseValue();
      REQUIRE_FALSE(pi->Data(u8"b").HasException());
      CHECK(pi->Data() == u8"b");
    }
  }

  TEST_CASE("CharacterData.length: its length, in UTF-8 code units",
            "[DOM][CharacterData][dom:dom-characterdata-length]")
  {
    CommonTestData data;
    CHECK(data.Node->Length() == 13uz);
    CHECK(CreateRef<Text>(*data.Document, u8"é")->Length() == 2uz);
    CHECK(CreateRef<Text>(*data.Document, u8"")->Length() == 0uz);
  }

  TEST_CASE("CharacterData.substringData(): substrings its data, changing nothing",
            "[DOM][CharacterData][dom:dom-characterdata-substringdata]")
  {
    CommonTestData data;

    CHECK(data.Node->SubstringData(0, 5).Value() == u8"Hello");
    CHECK(data.Node->SubstringData(7, 20).Value() == u8"world!");
    auto outOfBounds = data.Node->SubstringData(20, 5);
    REQUIRE(outOfBounds.HasException());
    CHECK(outOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);

    CHECK(data.Node->Data() == u8"Hello, world!");
    data.CheckRanges(13, 3, 7);
  }

  TEST_CASE("CharacterData.appendData(): replaces data at the end, with nothing removed",
            "[DOM][CharacterData][dom:dom-characterdata-appenddata]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->AppendData(u8" How are you?").HasException());
    CHECK(data.Node->Data() == u8"Hello, world! How are you?");
    // A boundary at the end isn't after offset + count, so it stays.
    data.CheckRanges(13, 3, 7);
  }

  TEST_CASE("CharacterData.insertData(): replaces data at the offset, with nothing removed",
            "[DOM][CharacterData][dom:dom-characterdata-insertdata]")
  {
    CommonTestData data;

    auto outOfBounds = data.Node->InsertData(20, u8"test");
    REQUIRE(outOfBounds.HasException());
    CHECK(outOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);

    REQUIRE_FALSE(data.Node->InsertData(7, u8"beautiful ").HasException());
    CHECK(data.Node->Data() == u8"Hello, beautiful world!");
    data.CheckRanges(23, 3, 7);
  }

  TEST_CASE("CharacterData.deleteData(): replaces count code units at the offset with nothing",
            "[DOM][CharacterData][dom:dom-characterdata-deletedata]")
  {
    CommonTestData data;

    auto outOfBounds = data.Node->DeleteData(20, 5);
    REQUIRE(outOfBounds.HasException());
    CHECK(outOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);
    data.CheckRanges(13, 3, 7);

    REQUIRE_FALSE(data.Node->DeleteData(5, 7).HasException());
    CHECK(data.Node->Data() == u8"Hello!");
    data.CheckRanges(6, 3, 5);
  }

  TEST_CASE("CharacterData.replaceData(): replaces count code units at the offset with the data",
            "[DOM][CharacterData][dom:dom-characterdata-replacedata]")
  {
    CommonTestData data;
    auto observer = data.Observe();

    auto outOfBounds = data.Node->ReplaceData(20, 5, u8"test");
    REQUIRE(outOfBounds.HasException());
    CHECK(outOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);

    REQUIRE_FALSE(data.Node->ReplaceData(7, 5, u8"universe").HasException());
    CHECK(data.Node->Data() == u8"Hello, universe!");
    data.CheckRanges(16, 3, 7);
    CHECK(observer->TakeRecords().size() == 1uz);
  }

  TEST_CASE("Node.nodeValue: character data's is its data, which setting it replaces",
            "[DOM][CharacterData][dom:dom-node-nodevalue]")
  {
    CommonTestData data;
    CHECK(data.Node->NodeValue() == u8"Hello, world!");
    REQUIRE_FALSE(data.Node->NodeValue(u8"New value").HasException());
    CHECK(data.Node->NodeValue() == u8"New value");
    data.CheckRanges(0, 0, 0);
  }

  TEST_CASE("Node.textContent: character data's is its data, which setting it replaces",
            "[DOM][CharacterData][dom:dom-node-textcontent][dom:get-text-content][dom:set-text-content]")
  {
    CommonTestData data;
    CHECK(data.Node->TextContent() == u8"Hello, world!");
    REQUIRE_FALSE(data.Node->TextContent(u8"New value").HasException());
    CHECK(data.Node->TextContent() == u8"New value");
    data.CheckRanges(0, 0, 0);
  }
}
