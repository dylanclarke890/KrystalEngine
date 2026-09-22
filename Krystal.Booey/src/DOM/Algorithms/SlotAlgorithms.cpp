#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
#pragma region Slottables

  bool SlotAlgorithms::IsSlottable(const Node &node) noexcept
  {
    return Is<Element>(node) || Is<Text>(node);
  }

  bool SlotAlgorithms::IsSlottable(const Text &) noexcept
  {
    return true;
  }

  bool SlotAlgorithms::IsSlottable(const Element &) noexcept
  {
    return true;
  }

  html::HTMLSlotElement *SlotAlgorithms::DefaultSlot(Node &node) noexcept
  {
    krys_debug_assert(Is<ShadowRoot>(TreeQueries::Root(node)));

    for (auto &descendant : DescendantRange(TreeQueries::Root(node)))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name().empty())
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  const html::HTMLSlotElement *SlotAlgorithms::DefaultSlot(const Node &node) noexcept
  {
    krys_debug_assert(Is<ShadowRoot>(TreeQueries::Root(node)));

    for (auto &descendant : ConstDescendantRange(TreeQueries::Root(node)))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name().empty())
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  bool SlotAlgorithms::IsAssigned(const Node &node) noexcept
  {
    if (Is<Element>(node))
    {
      return IsAssigned(Downcast<Element>(node));
    }

    if (Is<Text>(node))
    {
      return IsAssigned(Downcast<Text>(node));
    }

    return false;
  }

  bool SlotAlgorithms::IsAssigned(const Text &node) noexcept
  {
    return node._assignedSlot != nullptr;
  }

  bool SlotAlgorithms::IsAssigned(const Element &node) noexcept
  {
    return node._assignedSlot != nullptr;
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Node &node) noexcept
  {
    if (Is<Element>(node))
    {
      return GetAssignedSlot(Downcast<Element>(node));
    }

    if (Is<Text>(node))
    {
      return GetAssignedSlot(Downcast<Text>(node));
    }

    return nullptr;
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Text &node) noexcept
  {
    return node._assignedSlot.get();
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Element &node) noexcept
  {
    return node._assignedSlot.get();
  }

#pragma endregion

#pragma region Finding slots and slotted elements

  html::HTMLSlotElement *SlotAlgorithms::FindSlot(Node &slottable, bool open) noexcept
  {
    krys_debug_assert(IsSlottable(slottable));

    Element *parent = slottable.ParentElement();

    if (parent == nullptr)
    {
      return nullptr;
    }

    RefPtr<ShadowRoot> shadow = parent->ShadowRoot();

    if (shadow == nullptr)
    {
      return nullptr;
    }

    if (open && shadow->Mode() != ShadowRootMode::Open)
    {
      return nullptr;
    }

    if (shadow->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      for (auto &descendant : DescendantRange(*shadow))
      {
        auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant);
        if (slot == nullptr)
        {
          continue;
        }

        for (auto &manuallyAssignedNode : slot->_manuallyAssignedNodes)
        {
          if (manuallyAssignedNode == &slottable)
          {
            return slot;
          }
        }
      }

      return nullptr;
    }

    auto &slottableName = Is<Element>(slottable) ? Downcast<Element>(slottable)._slottableName
                                                 : Downcast<Text>(slottable)._slottableName;

    for (auto &descendant : DescendantRange(*shadow))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name() == slottableName)
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  List<Ref<Node>> SlotAlgorithms::FindSlottables(html::HTMLSlotElement &slot) noexcept
  {
    List<Ref<Node>> result;
    ShadowRoot *root = DynamicDowncast<ShadowRoot>(TreeQueries::Root(slot));

    if (root == nullptr)
    {
      return result;
    }

    Element *host = root->Host();
    if (root->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      for (auto &manuallyAssignedNode : slot._manuallyAssignedNodes)
      {
        if (auto slottable = manuallyAssignedNode.lock())
        {
          if (slottable->ParentNode() == host)
          {
            result.push_back(ShareRef(*slottable));
          }
        }
      }
    }
    else
    {
      for (auto &slottable : ChildNodeRange(host))
      {
        if (IsSlottable(slottable))
        {
          auto *foundSlot = FindSlot(slottable);
          if (foundSlot == &slot)
          {
            result.push_back(ShareRef(slottable));
          }
        }
      }
    }

    return result;
  }

  List<Ref<Node>> SlotAlgorithms::FindFlattenedSlottables(html::HTMLSlotElement &slot) noexcept
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
        if (IsSlottable(child))
        {
          slottables.push_back(ShareRef(child));
        }
      }
    }

    for (auto &node : slottables)
    {
      if (Is<html::HTMLSlotElement>(node) && TreeQueries::IsInShadowTree(*node))
      {
        auto temporaryResult = FindFlattenedSlottables(Downcast<html::HTMLSlotElement>(*node));
        result.append_range(std::move(temporaryResult));
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

  void SlotAlgorithms::AssignSlottables(html::HTMLSlotElement &slot) noexcept
  {
    auto slottables = FindSlottables(slot);
    if (slottables != slot._assignedNodes)
    {
      SignalSlotChange(slot);
    }

    slot._assignedNodes = std::move(slottables);
    for (auto &slottable : slot._assignedNodes)
    {
      if (auto *element = DynamicDowncast<Element>(*slottable))
      {
        element->_assignedSlot = ShareRef(slot);
      }
      else if (auto *text = DynamicDowncast<Text>(*slottable))
      {
        text->_assignedSlot = ShareRef(slot);
      }
    }
  }

  void SlotAlgorithms::AssignSlottablesForTree(Node &root) noexcept
  {
    for (auto &inclusiveDescendant : InclusiveDescendantRange(root))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(inclusiveDescendant))
      {
        AssignSlottables(*slot);
      }
    }
  }

  void SlotAlgorithms::AssignSlot(Node &slottable) noexcept
  {
    html::HTMLSlotElement *slot = FindSlot(slottable);
    if (slot != nullptr)
    {
      AssignSlottables(*slot);
    }
  }

#pragma endregion

#pragma region Signaling slot change

  void SlotAlgorithms::SignalSlotChange(html::HTMLSlotElement &slot) noexcept
  {
    (void)slot;
    // TODO(impl): SLOTTABLES - Append slot to slot’s relevant agent’s signal slots
    // TODO(impl): MUTATION-OBSERVERS - Queue a mutation observer microtask.
  }

#pragma endregion
}