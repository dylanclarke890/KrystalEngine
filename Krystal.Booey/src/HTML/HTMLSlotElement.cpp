#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSlotElement::HTMLSlotElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Slot, HTMLElementFlags::None,
                    dom::NodeFlags::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  dom::DOMString HTMLSlotElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(dom::DOMString &&name) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(name));
  }

  List<Ref<dom::Node>> HTMLSlotElement::AssignedNodes(const AssignedNodesOptions &options) noexcept
  {
    // 1. If options["flatten"] is false, then return this's assigned nodes.
    if (!options.Flatten)
    {
      List<Ref<dom::Node>> nodes;
      nodes.reserve(_assignedNodes.size());
      for (auto &weakNode : _assignedNodes)
      {
        if (auto node = weakNode.lock())
        {
          nodes.push_back(krys::move(node));
        }
      }
      return nodes;
    }

    // 2. Return the result of finding flattened slottables with this.
    return dom::SlotAlgorithms::FindFlattenedSlottables(*this);
  }

  List<Ref<dom::Element>> HTMLSlotElement::AssignedElements(const AssignedNodesOptions &options) noexcept
  {
    // 1. If options["flatten"] is false, then return this's assigned nodes, filtered to contain only Element
    //    nodes.
    // 2. Return the result of finding flattened slottables with this, filtered to contain only Element nodes.
    List<Ref<dom::Element>> elements;
    for (auto &node : AssignedNodes(options))
    {
      if (auto *element = DynamicDowncast<dom::Element>(*node))
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  void HTMLSlotElement::Assign(const List<dom::ElementOrText> &nodes) noexcept
  {
    // 1. For each node of this's manually assigned nodes, set node's manual slot assignment to null.
    for (auto &weakNode : _manuallyAssignedNodes)
    {
      if (auto node = weakNode.lock())
      {
        dom::SlotAlgorithms::SetManualSlotAssignment(*node, nullptr);
      }
    }

    // 2. Let nodesSet be a new ordered set.
    List<WeakRef<dom::Node>> nodesSet;

    // 3. For each node of nodes:
    for (auto &item : nodes)
    {
      dom::Node &node = std::visit([](auto &slottable) -> dom::Node & { return *slottable; }, item);

      // 3a. If node's manual slot assignment refers to a slot, then remove node from that slot's manually
      //     assigned nodes.
      if (auto *slot = dom::SlotAlgorithms::GetManualSlotAssignment(node))
      {
        std::erase_if(slot->_manuallyAssignedNodes,
                      [&](const auto &weakNode) { return weakNode.get() == &node; });
      }

      // 3b. Set node's manual slot assignment to this.
      dom::SlotAlgorithms::SetManualSlotAssignment(node, this);

      // 3c. Append node to nodesSet.
      if (!std::ranges::any_of(nodesSet, [&](const auto &weakNode) { return weakNode.get() == &node; }))
      {
        nodesSet.push_back(CreateWeakRef<dom::Node>(node));
      }
    }

    // 4. Set this's manually assigned nodes to nodesSet.
    _manuallyAssignedNodes = krys::move(nodesSet);

    // 5. Run assign slottables for a tree for this's root.
    dom::SlotAlgorithms::AssignSlottablesForTree(dom::TreeQueries::Root(*this));
  }

#pragma endregion

  void HTMLSlotElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    // https://dom.spec.whatwg.org/#slot-name
    // 1. If element is a slot, localName is name, and namespace is null:
    if (localName != Attributes::Name || namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // 1a. If value is oldValue, then return.
    if (value == oldValue)
    {
      return;
    }

    // 1b. If value is null and oldValue is the empty string, then return.
    if (!value.has_value() && oldValue.has_value() && oldValue->empty())
    {
      return;
    }

    // 1c. If value is the empty string and oldValue is null, then return.
    if (value.has_value() && value->empty() && !oldValue.has_value())
    {
      return;
    }

    // 1d. If value is null or the empty string, then set element's name to the empty string.
    // 1e. Otherwise, set element's name to value.
    // The name is read from the attribute (Name()), which already holds value.

    // 1f. Run assign slottables for a tree with element's root.
    dom::SlotAlgorithms::AssignSlottablesForTree(dom::TreeQueries::Root(*this));
  }
}
