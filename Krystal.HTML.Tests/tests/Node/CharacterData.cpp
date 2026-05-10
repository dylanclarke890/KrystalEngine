#include "Krystal.HTML.Tests/TestCharacterData.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  struct CommonTestData
  {
    Ref<Document> Document;
    Ref<TestCharacterData> Node;

    CommonTestData(DOMString &&value)
        : Document(CreateRef<HTML::Document>()),
          Node(CreateRef<TestCharacterData>(*Document, Krys::Move(value)))
    {
    }
  };

  TEST_CASE("CharacterData::Data", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    REQUIRE(data.Node->Data() == u8"Hello, world!");
    REQUIRE_FALSE(data.Node->Data(u8"New value").HasException());
    REQUIRE(data.Node->Data() == u8"New value");
  }

  TEST_CASE("CharacterData::Length", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};
    REQUIRE(data.Node->Length() == 13);
  }

  TEST_CASE("CharacterData::SubstringData", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    auto substringOutOfBounds = data.Node->SubstringData(20, 5);
    REQUIRE(substringOutOfBounds.HasException());
    REQUIRE(substringOutOfBounds.GetException().Code() == ExceptionCode::IndexSizeError);

    auto substringToEnd = data.Node->SubstringData(7, 20);
    REQUIRE_FALSE(substringToEnd.HasException());
    REQUIRE(substringToEnd.Value() == u8"world!");

    auto substring = data.Node->SubstringData(0, 5);
    REQUIRE_FALSE(substring.HasException());
    REQUIRE(substring.Value() == u8"Hello");
  }

  TEST_CASE("CharacterData::AppendData", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    auto appendResult = data.Node->AppendData(u8" How are you?");
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->Data() == u8"Hello, world! How are you?");
  }

  TEST_CASE("CharacterData::InsertData", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    auto outOfBoundsInsertResult = data.Node->InsertData(20, u8"test");
    REQUIRE(outOfBoundsInsertResult.HasException());
    REQUIRE(outOfBoundsInsertResult.GetException().Code() == ExceptionCode::IndexSizeError);

    auto insertResult = data.Node->InsertData(7, u8"beautiful ");
    REQUIRE_FALSE(insertResult.HasException());

    REQUIRE(data.Node->Data() == u8"Hello, beautiful world!");
  }

  TEST_CASE("CharacterData::DeleteData", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    auto outOfBoundsDeleteResult = data.Node->DeleteData(20, 5);
    REQUIRE(outOfBoundsDeleteResult.HasException());
    REQUIRE(outOfBoundsDeleteResult.GetException().Code() == ExceptionCode::IndexSizeError);

    auto deleteResult = data.Node->DeleteData(5, 7);
    REQUIRE_FALSE(deleteResult.HasException());

    REQUIRE(data.Node->Data() == u8"Hello!");
  }

  TEST_CASE("CharacterData::ReplaceData", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    auto outOfBoundsReplaceResult = data.Node->ReplaceData(20, 5, u8"test");
    REQUIRE(outOfBoundsReplaceResult.HasException());
    REQUIRE(outOfBoundsReplaceResult.GetException().Code() == ExceptionCode::IndexSizeError);

    auto replaceResult = data.Node->ReplaceData(7, 5, u8"universe");
    REQUIRE_FALSE(replaceResult.HasException());

    REQUIRE(data.Node->Data() == u8"Hello, universe!");
  }

  TEST_CASE("CharacterData::NodeValue", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    REQUIRE(data.Node->NodeValue() == u8"Hello, world!");

    auto setNodeValueResult = data.Node->NodeValue(u8"New value");
    REQUIRE_FALSE(setNodeValueResult.HasException());

    REQUIRE(data.Node->NodeValue() == u8"New value");
  }

  TEST_CASE("CharacterData::TextContent", "[HTML][CharacterData]")
  {
    CommonTestData data {u8"Hello, world!"};

    REQUIRE(data.Node->TextContent() == u8"Hello, world!");

    auto setTextContentResult = data.Node->TextContent(u8"New value");
    REQUIRE_FALSE(setTextContentResult.HasException());

    REQUIRE(data.Node->TextContent() == u8"New value");
  }

  // TODO(impl): none of the above tests check that live ranges are updated or mutation records are queued
}