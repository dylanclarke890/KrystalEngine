#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Dom/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLSlotElement::HTMLSlotElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Slot, {}, dom::NodeFlag::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  DOMString HTMLSlotElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(DOMString name) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(name));
  }

  // https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assignednodes
  List<Ref<Node>> HTMLSlotElement::AssignedNodes(const html::AssignedNodesOptions &options) noexcept
  {
    // 1. If options["flatten"] is false, then return this's assigned nodes.
    if (!options.Flatten)
    {
      List<Ref<Node>> nodes;
      nodes.reserve(_assignedNodes.size());
      for (auto &weakNode : _assignedNodes)
      {
        if (auto node = weakNode.lock())
        {
          nodes.emplace_back(krys::move(node));
        }
      }
      return nodes;
    }

    // 2. Return the result of finding flattened slottables with this.
    return dom::SlotAlgorithms::FindFlattenedSlottables(*this);
  }

  // https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assignedelements
  List<Ref<Element>> HTMLSlotElement::AssignedElements(const html::AssignedNodesOptions &options) noexcept
  {
    // 1. If options["flatten"] is false, then return this's assigned nodes, filtered to contain only Element
    //    nodes.
    // 2. Return the result of finding flattened slottables with this, filtered to contain only Element nodes.
    List<Ref<Element>> elements;
    for (auto &node : AssignedNodes(options))
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  // https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assign
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
    List<WeakRef<Node>> nodesSet;

    // 3. For each node of nodes:
    for (auto &item : nodes)
    {
      Node &node = std::visit([](auto &slottable) -> Node & { return *slottable; }, item);

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
        nodesSet.push_back(CreateWeakRef<Node>(node));
      }
    }

    // 4. Set this's manually assigned nodes to nodesSet.
    _manuallyAssignedNodes = krys::move(nodesSet);

    // 5. Run assign slottables for a tree for this's root.
    dom::SlotAlgorithms::AssignSlottablesForTree(dom::TreeQueries::Root(*this));
  }

#pragma endregion

  void HTMLSlotElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    // https://dom.spec.whatwg.org/#slot-name
    // 1. If element is a slot, localName is name, and namespace is null:
    if (localName != u8"name" || namespaceURI != null)
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
