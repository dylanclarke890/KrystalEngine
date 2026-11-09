#include "Krystal.Booey/HTML/Collections/LabelsNodeList.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"

namespace krys::boo::html
{
  LabelsNodeList::LabelsNodeList(HTMLElement &ownerNode) noexcept
      : LiveNodeList(
          ownerNode,
          [this](const dom::Node &node)
          {
            auto *element = DynamicDowncast<dom::Element>(node);
            return element && ElementMatches(*element);
          },
          dom::LiveCollectionInvalidation::Labels, dom::LiveCollectionTraversal::OwnerTree),
        _ownerNode(ShareRef(ownerNode))
  {
  }

  bool LabelsNodeList::ElementMatches(const dom::Element &testNode) const noexcept
  {
    auto *label = DynamicDowncast<HTMLLabelElement>(testNode);
    return label && label->Control().get() == _ownerNode.get();
  }
}
