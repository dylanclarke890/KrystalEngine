#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Live range pre-remove steps: boundary points in the removed node move to where it was",
            "[DOM][LiveRangeUpdater][dom:live-range-pre-remove-steps]")
  {
    auto document = Document::ParseHTMLUnsafe(*CreateRef<HTMLDocument>(),
                                              u8"<!DOCTYPE html><body><div id=d><b id=b>bb</b><i id=i>ii</i>"
                                              u8"<u id=u></u></div>")
                      .ReleaseValue();
    Ref<Element> div = ShareRef(*document->GetElementById(u8"d"));
    Ref<Element> b = ShareRef(*document->GetElementById(u8"b"));
    Ref<Element> i = ShareRef(*document->GetElementById(u8"i"));
    Ref<Text> iText = ShareRef(Downcast<Text>(*i->FirstChild()));

    auto rangeOf = [&](Node &startNode, uint64 startOffset, Node &endNode, uint64 endOffset)
    {
      auto range = document->CreateRange();
      REQUIRE_FALSE(range->SetStart(startNode, startOffset).HasException());
      REQUIRE_FALSE(range->SetEnd(endNode, endOffset).HasException());
      return range;
    };

    SECTION("In the removed node's descendants, Text nodes included")
    {
      auto inText = rangeOf(*iText, 1, *iText, 2);
      auto inElement = rangeOf(*i, 0, *i, 1);
      REQUIRE_FALSE(div->RemoveChild(*i).HasException());
      for (auto &range : {inText, inElement})
      {
        CHECK(range->StartContainer() == div.get());
        CHECK(range->StartOffset() == 1uz);
        CHECK(range->EndContainer() == div.get());
        CHECK(range->EndOffset() == 1uz);
      }
    }

    SECTION("In the removed Text node itself")
    {
      auto range = rangeOf(*iText, 0, *iText, 2);
      REQUIRE_FALSE(i->RemoveChild(*iText).HasException());
      CHECK(range->StartContainer() == i.get());
      CHECK(range->StartOffset() == 0uz);
      CHECK(range->EndContainer() == i.get());
      CHECK(range->EndOffset() == 0uz);
    }

    SECTION("Only the end in it")
    {
      auto range = rangeOf(*b, 0, *iText, 1);
      REQUIRE_FALSE(div->RemoveChild(*i).HasException());
      CHECK(range->StartContainer() == b.get());
      CHECK(range->EndContainer() == div.get());
      CHECK(range->EndOffset() == 1uz);
    }

    SECTION("Offsets in the parent after the node decrease by one")
    {
      auto range = rangeOf(*div, 1, *div, 3);
      REQUIRE_FALSE(div->RemoveChild(*b).HasException());
      CHECK(range->StartOffset() == 0uz);
      CHECK(range->EndOffset() == 2uz);

      // Offsets at or before its index don't change.
      auto atIndex = rangeOf(*div, 0, *div, 0);
      REQUIRE_FALSE(div->RemoveChild(*i).HasException());
      CHECK(atIndex->StartOffset() == 0uz);
      CHECK(range->StartOffset() == 0uz);
      CHECK(range->EndOffset() == 1uz);
    }
  }
}
