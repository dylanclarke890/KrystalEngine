#include "Krystal.Web/Html/Collections/LabelsNodeList.hpp"
#include "Krystal.Web/Dom/Internals/Iterators/SubtreeRanges.hpp"
#include "Krystal.Web/Html/Elements/HTMLLabelElement.hpp"

namespace krys::web::html
{
  LabelsNodeList::LabelsNodeList(HTMLElement &ownerNode) noexcept
      : LiveNodeList(
          ownerNode,
          [this](const Node &node)
          {
            auto *element = DynamicDowncast<Element>(node);
            return element && ElementMatches(*element);
          },
          dom::LiveCollectionInvalidation::Labels, dom::LiveCollectionTraversal::OwnerTree),
        _ownerNode(ShareRef(ownerNode))
  {
  }

  bool LabelsNodeList::ElementMatches(const Element &testNode) const noexcept
  {
    auto *label = DynamicDowncast<HTMLLabelElement>(testNode);
    return label && label->Control().get() == _ownerNode.get();
  }
}
