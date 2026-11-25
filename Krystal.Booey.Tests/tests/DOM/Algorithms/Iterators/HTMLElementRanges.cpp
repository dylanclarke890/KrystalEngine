#include "Krystal.Booey/DOM/Algorithms/Iterators/HTMLElementRanges.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/DOMTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("ChildHTMLElementRange: the HTML element children", "[DOM][SubtreeRanges]")
  {
    auto document = boo::tests::ParseDocument(
      u8"<!DOCTYPE html><body><div id=parent>text<p id=first></p><svg id=svg></svg><!-- comment --><span "
      u8"id=last></span></div>");
    auto &parent = *document->GetElementById(u8"parent");

    List<DOMString> ids;
    for (auto &element : ChildHTMLElementRange(parent))
    {
      ids.push_back(element.Id());
    }
    CHECK(ids == List<DOMString> {u8"first", u8"last"});

    // The const range walks the same children.
    const auto &constParent = parent;
    CHECK(std::ranges::distance(ConstChildHTMLElementRange(constParent)) == 2);

    // A node without HTML element children gives an empty range.
    auto &svg = *document->GetElementById(u8"svg");
    CHECK(ChildHTMLElementRange(svg).begin() == ChildHTMLElementRange(svg).end());
  }
}
