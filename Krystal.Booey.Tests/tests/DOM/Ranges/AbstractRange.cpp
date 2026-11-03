#include "Krystal.Booey/DOM/Ranges/AbstractRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Ranges/Range.hpp"
#include "Krystal.Booey/DOM/Ranges/StaticRange.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct AbstractRangeTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Parent;
      Ref<Text> Text;

      AbstractRangeTestData()
          : Document(CreateRef<dom::Document>()), Parent(CreateRef<TestElement>(*Document)),
            Text(CreateRef<dom::Text>(*Document, u8"text"))
      {
        REQUIRE_FALSE(Parent->AppendChild(*Text).HasException());
      }

      /// @brief A static range, or a live one, from (startNode, startOffset) to (endNode, endOffset).
      Ref<AbstractRange> MakeRange(bool live, Node &startNode, uint64 startOffset, Node &endNode,
                                   uint64 endOffset)
      {
        if (live)
        {
          auto range = Document->CreateRange();
          REQUIRE_FALSE(range->SetStart(startNode, startOffset).HasException());
          REQUIRE_FALSE(range->SetEnd(endNode, endOffset).HasException());
          return range;
        }
        return CreateRef<StaticRange>(
          StaticRangeInit::Create(BoundaryPoint {ShareRef(startNode), startOffset},
                                  BoundaryPoint {ShareRef(endNode), endOffset})
            .ReleaseValue());
      }
    };
  }

  TEST_CASE("AbstractRange: startContainer, startOffset, endContainer and endOffset are its start and end",
            "[DOM][AbstractRange][dom:interface-abstractrange][dom:concept-range][dom:concept-range-start]"
            "[dom:concept-range-end][dom:concept-range-start-node][dom:concept-range-start-offset]"
            "[dom:concept-range-end-node][dom:concept-range-end-offset][dom:dom-range-startcontainer]"
            "[dom:dom-range-startoffset][dom:dom-range-endcontainer][dom:dom-range-endoffset]")
  {
    AbstractRangeTestData data;
    for (bool live : {false, true})
    {
      auto range = data.MakeRange(live, *data.Text, 1, *data.Parent, 1);
      CHECK(range->IsLiveRange() == live);
      CHECK(range->StartContainer() == data.Text.get());
      CHECK(range->StartOffset() == 1uz);
      CHECK(range->EndContainer() == data.Parent.get());
      CHECK(range->EndOffset() == 1uz);
    }
  }

  TEST_CASE("AbstractRange.collapsed: whether its start and end are the same node and offset",
            "[DOM][AbstractRange][dom:dom-range-collapsed][dom:range-collapsed]")
  {
    AbstractRangeTestData data;
    for (bool live : {false, true})
    {
      CHECK(data.MakeRange(live, *data.Text, 2, *data.Text, 2)->Collapsed());
      CHECK_FALSE(data.MakeRange(live, *data.Text, 1, *data.Text, 2)->Collapsed());
      // The same offset in different nodes isn't collapsed.
      CHECK_FALSE(data.MakeRange(live, *data.Text, 1, *data.Parent, 1)->Collapsed());
    }
    // A new live range is collapsed at the start of its document.
    CHECK(data.Document->CreateRange()->Collapsed());
  }
}
