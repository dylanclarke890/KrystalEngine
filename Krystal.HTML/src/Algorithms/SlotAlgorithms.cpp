#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML
{
#pragma region Slottables

  bool SlotAlgorithms::IsSlottable(const Node &node) noexcept
  {
    return Is<Element>(node) || Is<Text>(node);
  }

  bool SlotAlgorithms::IsSlottable(const Text &node) noexcept
  {
    return true;
  }

  bool SlotAlgorithms::IsSlottable(const Element &node) noexcept
  {
    return true;
  }

  RawPtr<HTMLSlotElement> SlotAlgorithms::DefaultSlot(Node &node) noexcept
  {
    assert(Is<ShadowRoot>(TreeQueries::Root(node)));

    for (auto &descendant : DescendantRange(TreeQueries::Root(node)))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(descendant))
      {
        if (slot->Name().empty())
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  RawPtr<const HTMLSlotElement> SlotAlgorithms::DefaultSlot(const Node &node) noexcept
  {
    return DefaultSlot(const_cast<Node &>(node));
  }

  bool SlotAlgorithms::IsAssigned(const Node &node) noexcept
  {
    // TODO(impl): SLOTTABLES - A slottable is assigned if its assigned slot is non-null.
    return false;
  }

  RawPtr<HTMLSlotElement> SlotAlgorithms::GetAssignedSlot(const Node &node) noexcept
  {
    // TODO(impl): SLOTTABLES - check for either text or element node and get assigned slot if slottable
    return nullptr;
  }

#pragma endregion

#pragma region Finding slots and slotted elements

  RawPtr<HTMLSlotElement> SlotAlgorithms::FindSlot(Node &slottable, bool open) noexcept
  {
    RawPtr<Element> parent = slottable.ParentElement();

    if (parent == nullptr)
    {
      return nullptr;
    }

    RefPtr<ShadowRoot> shadow = parent->ShadowRoot();

    if (shadow == nullptr || (open && shadow->Mode() != ShadowRootMode::Open))
    {
      return nullptr;
    }

    if (shadow->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      // TODO(impl): SLOTTABLES - return the slot in shadow’s descendants whose manually assigned nodes
      // contains slottable, if any; otherwise null.
      return nullptr;
    }
    else
    {
      // TODO(impl): SLOTTABLES - Return the first slot in tree order in shadow’s descendants whose name is
      // slottable’s name, if any; otherwise null.
      return nullptr;
    }
  }

  List<Ref<Node>> SlotAlgorithms::FindSlottables(HTMLSlotElement &slot) noexcept
  {
    List<Ref<Node>> result;
    RawPtr<ShadowRoot> root = DynamicDowncast<ShadowRoot>(TreeQueries::Root(slot));

    if (root == nullptr)
    {
      return result;
    }

    RawPtr<Element> host = root->Host();
    if (root->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      // TODO(impl): SLOTTABLES - For each slottable slottable of slot’s manually assigned nodes, if
      // slottable’s parent is host, append slottable to result.
    }
    else
    {
      // TODO(impl): SLOTTABLES - for each slottable child slottable of host, in tree order:
      // Let foundSlot be the result of finding a slot given slottable.
      // If foundSlot is slot,
      // then append slottable to result.
    }

    return result;
  }

  List<Ref<Node>> SlotAlgorithms::FindFlattenedSlottables(HTMLSlotElement &slot) noexcept
  {
    List<Ref<Node>> result;

    if (!TreeQueries::IsInShadowTree(slot))
    {
      return result;
    }

    auto slottables = FindSlottables(slot);
    if (slottables.empty())
    {
      for (auto &child : ChildNodeRange(slot))
      {
        if (Is<Text>(child) || Is<Element>(child))
        {
          slottables.push_back(ShareRef(child));
        }
      }
    }

    for (auto &node : slottables)
    {
      auto *slotNode = DynamicDowncast<HTMLSlotElement>(*node);
      auto *slotShadowRoot = slotNode ? DynamicDowncast<ShadowRoot>(TreeQueries::Root(*slotNode)) : nullptr;

      if (slotShadowRoot != nullptr)
      {
        result.append_range(FindFlattenedSlottables(*slotNode));
      }
      else
      {
        result.push_back(node);
      }
    }

    return result;
  }

#pragma endregion

#pragma region Assigning slottables and slots

  void SlotAlgorithms::AssignSlottables(HTMLSlotElement &slot) noexcept
  {
    auto slottables = FindSlottables(slot);
    // TODO(impl): SLOTTABLES - if slottables and slot’s assigned nodes are not identical, then run signal a
    // slot change for slot. Set slot’s assigned nodes to slottables. For each slottable of slottables: set
    // slottable’s assigned slot to slot.
  }

  void SlotAlgorithms::AssignSlottablesForTree(Node &root) noexcept
  {
    for (auto &inclusiveDescendant : InclusiveDescendantRange(root))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(inclusiveDescendant))
      {
        AssignSlottables(*slot);
      }
    }
  }

  void SlotAlgorithms::AssignSlot(Node &slottable) noexcept
  {
    RawPtr<HTMLSlotElement> slot = FindSlot(slottable);
    if (slot != nullptr)
    {
      AssignSlottables(*slot);
    }
  }

#pragma endregion

#pragma region Signaling slot change

  void SlotAlgorithms::SignalSlotChange(HTMLSlotElement &slot) noexcept
  {
    // TODO(impl): SLOTTABLES - Append slot to slot’s relevant agent’s signal slots
    // Queue a mutation observer microtask.
  }

#pragma endregion
}