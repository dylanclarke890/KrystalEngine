#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Range.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<HTML::Text> Node;
      Ref<Range> FullRange;
      Ref<Range> PartialRange;

      CommonTestData()
          : Document(CreateRef<HTML::Document>()), Node(CreateRef<HTML::Text>(*Document, u8"Hello, world!")),
            FullRange(Document->CreateRange()), PartialRange(Document->CreateRange())
      {
        FullRange->SetStart(*Node, 0uz);
        FullRange->SetEnd(*Node, Node->Length());

        // "lo, w"
        PartialRange->SetStart(*Node, 3uz);
        PartialRange->SetEnd(*Node, 7uz);
      }
    };
  }

  TEST_CASE("CharacterData::Data", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    REQUIRE(data.Node->Data() == u8"Hello, world!");

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    REQUIRE_FALSE(data.Node->Data(u8"New value").HasException());
    REQUIRE(data.Node->Data() == u8"New value");

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 0uz);
    REQUIRE(data.PartialRange->StartOffset() == 0uz);
    REQUIRE(data.PartialRange->EndOffset() == 0uz);
  }

  TEST_CASE("CharacterData::Length", "[HTML][CharacterData]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->Length() == 13uz);

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);
  }

  TEST_CASE("CharacterData::SubstringData", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    auto substringOutOfBounds = data.Node->SubstringData(20, 5);
    REQUIRE(substringOutOfBounds.HasException());
    REQUIRE(substringOutOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    auto substringToEnd = data.Node->SubstringData(7, 20);
    REQUIRE_FALSE(substringToEnd.HasException());
    REQUIRE(substringToEnd.Value() == u8"world!");

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    auto substring = data.Node->SubstringData(0, 5);
    REQUIRE_FALSE(substring.HasException());
    REQUIRE(substring.Value() == u8"Hello");

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);
  }

  TEST_CASE("CharacterData::AppendData", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    auto appendResult = data.Node->AppendData(u8" How are you?");
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    REQUIRE(data.Node->Data() == u8"Hello, world! How are you?");
  }

  TEST_CASE("CharacterData::InsertData", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    auto outOfBoundsInsertResult = data.Node->InsertData(20, u8"test");
    REQUIRE(outOfBoundsInsertResult.HasException());
    REQUIRE(outOfBoundsInsertResult.GetException().Code() == ExceptionCode::IndexSizeError);

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    auto insertResult = data.Node->InsertData(7, u8"beautiful ");
    REQUIRE_FALSE(insertResult.HasException());

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 23uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    REQUIRE(data.Node->Data() == u8"Hello, beautiful world!");
  }

  TEST_CASE("CharacterData::DeleteData", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    auto outOfBoundsDeleteResult = data.Node->DeleteData(20, 5);
    REQUIRE(outOfBoundsDeleteResult.HasException());
    REQUIRE(outOfBoundsDeleteResult.GetException().Code() == ExceptionCode::IndexSizeError);

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 13uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    auto deleteResult = data.Node->DeleteData(5, 7);
    REQUIRE_FALSE(deleteResult.HasException());

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 6uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 5uz);

    REQUIRE(data.Node->Data() == u8"Hello!");
  }

  TEST_CASE("CharacterData::ReplaceData", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    auto outOfBoundsReplaceResult = data.Node->ReplaceData(20, 5, u8"test");
    REQUIRE(outOfBoundsReplaceResult.HasException());
    REQUIRE(outOfBoundsReplaceResult.GetException().Code() == ExceptionCode::IndexSizeError);

    auto replaceResult = data.Node->ReplaceData(7, 5, u8"universe");
    REQUIRE_FALSE(replaceResult.HasException());

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 16uz);
    REQUIRE(data.PartialRange->StartOffset() == 3uz);
    REQUIRE(data.PartialRange->EndOffset() == 7uz);

    REQUIRE(data.Node->Data() == u8"Hello, universe!");
  }

  TEST_CASE("CharacterData::NodeValue", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    REQUIRE(data.Node->NodeValue() == u8"Hello, world!");

    auto setNodeValueResult = data.Node->NodeValue(u8"New value");
    REQUIRE_FALSE(setNodeValueResult.HasException());

    REQUIRE(data.Node->NodeValue() == u8"New value");
  }

  TEST_CASE("CharacterData::TextContent", "[HTML][CharacterData]")
  {
    CommonTestData data {};

    REQUIRE(data.Node->TextContent() == u8"Hello, world!");

    auto setTextContentResult = data.Node->TextContent(u8"New value");
    REQUIRE_FALSE(setTextContentResult.HasException());

    REQUIRE(data.Node->TextContent() == u8"New value");

    REQUIRE(data.FullRange->StartContainer() == data.Node.get());
    REQUIRE(data.PartialRange->StartContainer() == data.Node.get());
    REQUIRE(data.FullRange->EndContainer() == data.Node.get());
    REQUIRE(data.PartialRange->EndContainer() == data.Node.get());

    REQUIRE(data.FullRange->StartOffset() == 0uz);
    REQUIRE(data.FullRange->EndOffset() == 0uz);
    REQUIRE(data.PartialRange->StartOffset() == 0uz);
    REQUIRE(data.PartialRange->EndOffset() == 0uz);
  }

  // TODO(test): none of the above tests check that mutation records are queued
}