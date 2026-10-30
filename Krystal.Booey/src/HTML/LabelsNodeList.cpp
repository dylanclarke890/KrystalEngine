#include "Krystal.Booey/HTML/LabelsNodeList.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/HTMLLabelElement.hpp"

namespace krys::boo::html
{
  LabelsNodeList::LabelsNodeList(HTMLElement &ownerNode) noexcept : _ownerNode(ShareRef(ownerNode))
  {
  }

  bool LabelsNodeList::ElementMatches(const dom::Element &testNode) const noexcept
  {
    auto *label = DynamicDowncast<HTMLLabelElement>(testNode);
    return label && label->Control().get() == _ownerNode.get();
  }

  List<Ref<dom::Element>> LabelsNodeList::Elements() const noexcept
  {
    // The list is rooted at the owner's tree, as the `for` attribute can name it from anywhere in it.
    auto &root = dom::TreeQueries::Root(*_ownerNode);
    List<Ref<dom::Element>> elements;
    for (auto &node : dom::DescendantRange(root))
    {
      if (auto *element = DynamicDowncast<dom::Element>(node); element && ElementMatches(*element))
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  RefPtr<dom::Node> LabelsNodeList::Item(size_t index) noexcept
  {
    auto elements = Elements();
    return index < elements.size() ? RefPtr<dom::Node>(elements[index]) : nullptr;
  }

  RefPtr<const dom::Node> LabelsNodeList::Item(size_t index) const noexcept
  {
    return const_cast<LabelsNodeList *>(this)->Item(index);
  }

  RefPtr<dom::Node> LabelsNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const dom::Node> LabelsNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t LabelsNodeList::Length() const noexcept
  {
    return Elements().size();
  }
}
