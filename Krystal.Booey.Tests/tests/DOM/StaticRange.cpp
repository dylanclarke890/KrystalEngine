#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief parent > (childA, text "abc").
    struct StaticRangeTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Parent;
      Ref<TestElement> ChildA;
      Ref<Text> Text;

      StaticRangeTestData()
          : Document(CreateRef<dom::Document>()), Parent(CreateRef<TestElement>(*Document)),
            ChildA(CreateRef<TestElement>(*Document)), Text(CreateRef<dom::Text>(*Document, u8"abc"))
      {
        REQUIRE_FALSE(Parent->AppendChild(*ChildA).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Text).HasException());
      }

      Ref<StaticRange> MakeRange(Node &startNode, uint64 startOffset, Node &endNode, uint64 endOffset)
      {
        return CreateRef<StaticRange>(
          StaticRangeInit::Create(BoundaryPoint {ShareRef(startNode), startOffset},
                                  BoundaryPoint {ShareRef(endNode), endOffset})
            .ReleaseValue());
      }
    };
  }

  TEST_CASE("new StaticRange(init): the init's start and end, which can't be in a DocumentType or Attr",
            "[DOM][StaticRange][dom:interface-staticrange][dom:dom-staticrange-staticrange]")
  {
    StaticRangeTestData data;

    // Any offsets and order are allowed; they aren't checked until the range is.
    auto range = data.MakeRange(*data.Text, 5, *data.ChildA, 0);
    CHECK(range->StartContainer() == data.Text.get());
    CHECK(range->StartOffset() == 5uz);
    CHECK(range->EndContainer() == data.ChildA.get());
    CHECK(range->EndOffset() == 0uz);

    auto doctype = data.Document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
    auto attr = data.Document->CreateAttribute(u8"test").ReleaseValue();
    for (Ref<Node> node : {Ref<Node>(doctype), Ref<Node>(attr)})
    {
      for (bool atStart : {true, false})
      {
        auto init = atStart ? StaticRangeInit::Create({node, 0}, {data.Text, 0})
                            : StaticRangeInit::Create({data.Text, 0}, {node, 0});
        REQUIRE(init.HasException());
        CHECK(init.GetException().Code() == ExceptionCode::InvalidNodeTypeError);
      }
    }
  }

  TEST_CASE("Valid StaticRange: in one tree, within its nodes, with its start before or equal to its end",
            "[DOM][StaticRange][dom:staticrange-valid]")
  {
    StaticRangeTestData data;

    CHECK(data.MakeRange(*data.ChildA, 0, *data.Text, 3)->IsValid());
    CHECK(data.MakeRange(*data.Parent, 0, *data.Parent, 2)->IsValid());

    SECTION("A collapsed range is valid")
    {
      CHECK(data.MakeRange(*data.Text, 1, *data.Text, 1)->IsValid());
      CHECK(data.MakeRange(*data.Parent, 2, *data.Parent, 2)->IsValid());
    }

    SECTION("Its start after its end isn't")
    {
      CHECK_FALSE(data.MakeRange(*data.Text, 2, *data.Text, 1)->IsValid());
      CHECK_FALSE(data.MakeRange(*data.Text, 0, *data.ChildA, 0)->IsValid());
    }

    SECTION("An offset past its node's length isn't")
    {
      CHECK_FALSE(data.MakeRange(*data.Text, 4, *data.Text, 4)->IsValid());
      CHECK_FALSE(data.MakeRange(*data.ChildA, 0, *data.ChildA, 1)->IsValid());
      CHECK_FALSE(data.MakeRange(*data.Parent, 3, *data.Parent, 3)->IsValid());
    }

    SECTION("Nodes in different trees aren't")
    {
      auto other = CreateRef<TestElement>(*data.Document);
      CHECK_FALSE(data.MakeRange(*data.ChildA, 0, *other, 0)->IsValid());
    }

    SECTION("A range made invalid by a change to the tree")
    {
      auto range = data.MakeRange(*data.Text, 1, *data.Text, 3);
      REQUIRE(range->IsValid());
      REQUIRE_FALSE(data.Text->DeleteData(0, 2).HasException());
      CHECK_FALSE(range->IsValid());
    }
  }
}
