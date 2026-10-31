#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/ChildChangeInvalidation.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD bool HasElementChild(ContainerNode &node) noexcept
    {
      return HasNodeOfType<Element>(ChildNodeRange(node));
    }

    KRYS_NODISCARD bool IsDocTypeOrDocTypeFollows(Node *node) noexcept
    {
      if (node == nullptr)
      {
        return false;
      }

      if (Is<DocumentType>(node))
      {
        return true;
      }

      return HasNodeOfType<DocumentType>(ConstNextSiblingRange(*node));
    }

    SmallNodeList CollectChildNodes(ContainerNode &parent) noexcept
    {
      SmallNodeList nodes;

      for (Node &child : ChildNodeRange(parent))
      {
        nodes.emplace_back(ShareRef(child));
      }

      return nodes;
    }

    using ChildChange = ContainerNode::ChildChange;

    /// @brief The parser appends to the elements whose children it's still parsing.
    KRYS_NODISCARD ChildChange::Source ChildChangeSource(const ContainerNode &parent) noexcept
    {
      auto *element = DynamicDowncast<Element>(parent);
      return element && !element->IsFinishedParsingChildren() ? ChildChange::Source::Parser
                                                              : ChildChange::Source::API;
    }

    KRYS_NODISCARD ChildChange MakeChildChangeForInsertion(ContainerNode &parent, Node &child,
                                                           Node *beforeChild) noexcept
    {
      auto changeType = [&]
      {
        if (Is<Element>(child))
        {
          return ChildChange::Type::ElementInserted;
        }
        if (Is<Text>(child))
        {
          return ChildChange::Type::TextInserted;
        }
        return ChildChange::Type::NonContentsChildInserted;
      }();

      auto *beforeChildElement = DynamicDowncast<Element>(beforeChild);
      return {changeType,
              DynamicDowncast<Element>(child),
              beforeChild ? TreeQueries::PreviousElementSibling(*beforeChild)
                          : TreeQueries::LastElementChild(parent),
              !beforeChild || beforeChildElement ? beforeChildElement
                                                 : TreeQueries::NextElementSibling(*beforeChild),
              ChildChangeSource(parent),
              changeType == ChildChange::Type::ElementInserted ? ChildChange::AffectsElements::Yes
                                                               : ChildChange::AffectsElements::No};
    }

    KRYS_NODISCARD ChildChange MakeChildChangeForRemoval(ContainerNode &parent, Node &childToRemove) noexcept
    {
      auto changeType = [&]
      {
        if (Is<Element>(childToRemove))
        {
          return ChildChange::Type::ElementRemoved;
        }
        if (Is<Text>(childToRemove))
        {
          return ChildChange::Type::TextRemoved;
        }
        return ChildChange::Type::NonContentsChildRemoved;
      }();

      return {changeType,
              DynamicDowncast<Element>(childToRemove),
              TreeQueries::PreviousElementSibling(childToRemove),
              TreeQueries::NextElementSibling(childToRemove),
              ChildChangeSource(parent),
              changeType == ChildChange::Type::ElementRemoved ? ChildChange::AffectsElements::Yes
                                                              : ChildChange::AffectsElements::No};
    }

    /// @brief Invalidates the style that inserting or removing @p child affects, before the change (when
    /// constructed) and after it (when destroyed).
    class ChildChangeStyleInvalidation
    {
      KRYS_NON_COPY_MOVABLE(ChildChangeStyleInvalidation);

    private:
      const ChildChange _childChange;
      style::ChildChangeInvalidation _invalidation;

    public:
      ChildChangeStyleInvalidation(ContainerNode &parent, const ChildChange &childChange) noexcept
          : _childChange(childChange), _invalidation(parent, _childChange)
      {
      }
    };
  }

  namespace
  {
    void RebuildSVGExtensionsElementsIfNecessary(ContainerNode &parent) noexcept
    {
      auto *extensions = parent.NodeDocument().SVGExtensionsIfExists();
      auto *svgHost = DynamicDowncast<svg::SVGElement>(TreeQueries::ShadowHost(parent));
      if (extensions && !(svgHost && Is<svg::SVGUseElement>(*svgHost)))
      {
        extensions->RebuildElements();
      }
    }
  }

  ExceptionOr<void> MutationAlgorithms::EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                Node *child,
                                                                Span<Node *const> childrenToExclude) noexcept
  {
    auto isExcluded = [&](const Node &candidate)
    {
      return std::ranges::contains(childrenToExclude, &candidate);
    };
    auto hasChildNotExcluded = [&]<typename TNode>()
    {
      return std::ranges::any_of(ConstChildNodeRange(parent), [&](const Node &candidate)
                                 { return Is<TNode>(candidate) && !isExcluded(candidate); });
    };

    // 1. If parent is not a Document, DocumentFragment, or Element node, then throw a "HierarchyRequestError"
    //    DOMException.
    if (!IsOneOf<Document, DocumentFragment, Element>(parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 2. If node is a host-including inclusive ancestor of parent, then throw a "HierarchyRequestError"
    //    DOMException.
    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 3. If child is non-null and its parent is not parent, then throw a "NotFoundError" DOMException.
    if (child != nullptr && child->ParentNode() != &parent)
    {
      return ExceptionCode::NotFoundError;
    }

    // 4. If node is not a DocumentFragment, DocumentType, Element, or CharacterData node, then throw a
    //    "HierarchyRequestError" DOMException.
    if (!IsOneOf<DocumentFragment, DocumentType, Element, CharacterData>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 5. If parent is not a document:
    if (!Is<Document>(parent))
    {
      // 5a. If node is a doctype, then throw a "HierarchyRequestError" DOMException.
      if (Is<DocumentType>(node))
      {
        return ExceptionCode::HierarchyRequestError;
      }

      // 5b. Return.
      return {};
    }

    // 6. If node is a Text node, then throw a "HierarchyRequestError" DOMException.
    if (Is<Text>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 7. If node is a CharacterData node, then return.
    if (Is<CharacterData>(node))
    {
      return {};
    }

    // 8. If node is a DocumentFragment node:
    if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
    {
      // 8a. If node has more than one element child or has a Text node child, then throw a
      //     "HierarchyRequestError" DOMException.
      auto count = documentFragment->ChildElementCount();
      if (count > 1uz || HasNodeOfType<Text>(ConstChildNodeRange(*documentFragment)))
      {
        return ExceptionCode::HierarchyRequestError;
      }

      // 8b. If node has no element child, then return.
      if (count == 0uz)
      {
        return {};
      }
    }

    // 9. If node is a DocumentFragment or Element node:
    if (IsOneOf<DocumentFragment, Element>(node))
    {
      // 9a. If any of the following are true:
      //     - parent has an element child that childrenToExclude does not contain;
      //     - child is non-null and a doctype is following child; or
      //     - child is a doctype that childrenToExclude does not contain,
      //     then throw a "HierarchyRequestError" DOMException.
      if (hasChildNotExcluded.template operator()<Element>()
          || (child != nullptr && HasNodeOfType<DocumentType>(ConstFollowingRange(*child)))
          || (Is<DocumentType>(child) && !isExcluded(*child)))
      {
        return ExceptionCode::HierarchyRequestError;
      }

      // 9b. Return.
      return {};
    }

    // 10. Assert: node is a doctype.
    krys_debug_assert(Is<DocumentType>(node));

    // 11. If any of the following are true:
    //     - parent has a doctype child that childrenToExclude does not contain;
    //     - child is non-null and an element is preceding child; or
    //     - child is null and parent has an element child that childrenToExclude does not contain,
    //     then throw a "HierarchyRequestError" DOMException.
    if (hasChildNotExcluded.template operator()<DocumentType>()
        || (child != nullptr && HasNodeOfType<Element>(ConstPrecedingRange(*child)))
        || (child == nullptr && hasChildNotExcluded.template operator()<Element>()))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::PreInsert(Node &node, ContainerNode &parent, Node *child) noexcept
  {
    // 1. Ensure pre-insert validity given node, parent, child, and « ».
    if (auto result = EnsurePreInsertValidity(node, parent, child); result.HasException())
    {
      return result.ReleaseException();
    }

    // 2. Let referenceChild be child.
    auto referenceChild = child;

    // 3. If referenceChild is node, then set referenceChild to node's next sibling.
    if (&node == referenceChild)
    {
      referenceChild = node.NextSibling();
    }

    // 4. Insert node into parent before referenceChild.
    if (auto result = Insert(node, parent, referenceChild); result.HasException())
    {
      return result.ReleaseException();
    }

    // 5. Return node.
    return node;
  }

  ExceptionOr<void> MutationAlgorithms::Insert(Node &node, ContainerNode &parent, Node *child,
                                               SuppressObservers suppressObservers) noexcept
  {
    // 1. Let nodes be node's children, if node is a DocumentFragment node; otherwise « node ».
    SmallNodeList nodes;
    if (Is<DocumentFragment>(node))
    {
      nodes = CollectChildNodes(Downcast<ContainerNode>(node));
    }
    else
    {
      nodes.push_back(ShareRef<Node>(node));
    }

    // 2. Let count be nodes's size.
    auto count = nodes.size();

    // 3. If count is 0, then return.
    if (count == 0uz)
    {
      return {};
    }

    // 4. If node is a DocumentFragment node:
    if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
    {
      // 4a. Remove its children with suppressObservers set to true.
      while (auto *fragmentChild = documentFragment->FirstChild())
      {
        auto result = Remove(*fragmentChild, SuppressObservers(true));
        if (result.HasException())
        {
          return result.ReleaseException();
        }
      }

      // 4b. Queue a tree mutation record for node with « », nodes, null, and null.
      SmallNodeList nodesCopy = nodes;
      MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(node), {}, krys::move(nodesCopy), nullptr,
                                                          nullptr);
    }

    // 5. If child is non-null:
    //   5a. For each live range whose start node is parent and start offset is greater than child's index:
    //       increase its start offset by count.
    //   5b. For each live range whose end node is parent and end offset is greater than child's index:
    //       increase its end offset by count.
    if (child != nullptr)
    {
      LiveRangeUpdater::InsertedBeforeNode(*child, count);
    }

    // 6. Let previousSibling be child's previous sibling or parent's last child if child is null.
    auto *previousSibling = child != nullptr ? child->PreviousSibling() : parent.LastChild();

    // A manual shadow root's host assigns a slot too, so that a node assign() gave a slot before it was the
    // host's child is assigned once it is.
    bool isParentShadowHost = TreeQueries::IsShadowHost(parent);
    bool isParentRootShadowRoot = Is<ShadowRoot>(TreeQueries::Root(parent));

    auto *slotParent = DynamicDowncast<html::HTMLSlotElement>(parent);

    // 7. For each node in nodes, in tree order:
    for (auto &target : nodes)
    {
      // 7a. Adopt node into parent's node document.
      if (auto result = DocumentAlgorithms::AdoptNode(*target, parent.NodeDocument()); result.HasException())
      {
        return result.ReleaseException();
      }

      ChildChangeStyleInvalidation styleInvalidation(parent,
                                                     MakeChildChangeForInsertion(parent, *target, child));

      // 7b. If child is null, then append node to parent's children.
      // 7c. Otherwise, insert node into parent's children before child's index.
      target->SetParentNode(&parent);
      if (child == nullptr)
      {
        if (auto *lastChild = parent.LastChild())
        {
          target->SetPreviousSibling(lastChild);
          lastChild->SetNextSibling(target.get());
        }
        else
        {
          parent.SetFirstChild(target.get());
        }

        parent.SetLastChild(target.get());
      }
      else
      {
        if (auto *childPreviousSibling = child->PreviousSibling())
        {
          childPreviousSibling->SetNextSibling(target.get());
          target->SetPreviousSibling(childPreviousSibling);

          child->SetPreviousSibling(target.get());
          target->SetNextSibling(child);
        }
        else
        {
          krys_debug_assert(parent.FirstChild() == child);
          parent.SetFirstChild(target.get());

          child->SetPreviousSibling(target.get());
          target->SetNextSibling(child);
        }
      }

      // 7d. If parent is a shadow host whose shadow root's slot assignment is "named" and node is a
      // slottable,
      //     then assign a slot for node.
      if (isParentShadowHost && SlotAlgorithms::IsSlottable(*target))
      {
        SlotAlgorithms::AssignSlot(*target);
      }

      // 7e. If parent's root is a shadow root, and parent is a slot whose assigned nodes is the empty list,
      // then
      //     run signal a slot change for parent.
      if (isParentRootShadowRoot && slotParent != nullptr && slotParent->_assignedNodes.empty())
      {
        SlotAlgorithms::SignalSlotChange(*slotParent);
      }

      // 7f. Run assign slottables for a tree with node's root.
      SlotAlgorithms::AssignSlottablesForTree(Downcast<ContainerNode>(TreeQueries::Root(*target)));

      // 7g. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including
      // tree
      //     order:
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        // 7g.i. Run the insertion steps with inclusiveDescendant.
        ExtensibilityHooks::NodeInserted(inclusiveDescendant, &inclusiveDescendant == target.get());

        // 7g.ii. If inclusiveDescendant is not connected, then continue.
        if (!inclusiveDescendant.IsConnected())
        {
          continue;
        }

        // 7g.iii. If inclusiveDescendant is an element and inclusiveDescendant's custom element registry is
        //         non-null:
        if (auto *element = DynamicDowncast<Element>(inclusiveDescendant))
        {
          if (auto registry = element->CustomElementRegistry())
          {
            // 7g.iii.A. If inclusiveDescendant's custom element registry's is scoped is true, then append
            //           inclusiveDescendant's node document to inclusiveDescendant's custom element
            //           registry's scoped document set.
            if (registry->IsScoped())
            {
              registry->AppendToScopedDocumentSet(element->NodeDocument());
            }

            // 7g.iii.B. If inclusiveDescendant is custom, then enqueue a custom element callback reaction
            // with
            //           inclusiveDescendant, callback name "connectedCallback", and « ».
            if (element->IsDefinedCustomElement())
            {
              html::CustomElementReactionQueue::EnqueueConnectedCallbackIfNeeded(*element);
            }
            // 7g.iii.C. Otherwise, try to upgrade inclusiveDescendant.
            // Only an upgrade candidate can be upgraded.
            else if (element->IsCustomElementUpgradeCandidate())
            {
              html::CustomElementReactionQueue::TryToUpgradeElement(*element);
            }
          }
        }
        // 7g.iv. Otherwise, if inclusiveDescendant is a shadow root, inclusiveDescendant's custom element
        // registry
        //        is non-null, and inclusiveDescendant's custom element registry's is scoped is true, then
        //        append inclusiveDescendant's node document to inclusiveDescendant's custom element
        //        registry's scoped document set.
        else if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(inclusiveDescendant))
        {
          if (auto registry = shadowRoot->CustomElementRegistry(); registry && registry->IsScoped())
          {
            registry->AppendToScopedDocumentSet(shadowRoot->NodeDocument());
          }
        }
      }
    }

    // 8. If suppressObservers is false, then queue a tree mutation record for parent with nodes, « »,
    //    previousSibling, and child.
    if (!suppressObservers)
    {
      SmallNodeList nodesCopy = nodes;
      MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(parent), krys::move(nodesCopy), {},
                                                          ShareRefPtr(previousSibling), ShareRefPtr(child));
    }

    bool onlyReplacedElementsInserted = std::ranges::all_of(nodes,
                                                            [](const Ref<Node> &node)
                                                            {
                                                              auto *element = DynamicDowncast<Element>(*node);
                                                              return element && element->IsReplaced();
                                                            });
    // 9. Run the children changed steps for parent.
    ExtensibilityHooks::NodeChildrenChanged(parent, onlyReplacedElementsInserted);

    RebuildSVGExtensionsElementsIfNecessary(parent);

    // 10. Let staticNodeList be a list of nodes, initially « ».
    SmallNodeList staticNodeList;

    // 11. For each node of nodes, in tree order:
    //   11a. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including
    //   tree
    //        order: append inclusiveDescendant to staticNodeList.
    for (auto &target : nodes)
    {
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        staticNodeList.push_back(ShareRef(inclusiveDescendant));
      }
    }

    // 12. For each node of staticNodeList: if node is connected, then run the post-connection steps with
    // node.
    for (auto &target : staticNodeList)
    {
      if (target->IsConnected())
      {
        ExtensibilityHooks::NodePostConnection(*target);
      }
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::Append(Node &node, ContainerNode &parent) noexcept
  {
    return PreInsert(node, parent, nullptr);
  }

  ExceptionOr<void> MutationAlgorithms::Move(Node &node, ContainerNode &newParent, Node *child) noexcept
  {
    // 1. If newParent's shadow-including root is not the same as node's shadow-including root, then throw a
    //    "HierarchyRequestError" DOMException.
    if (&TreeQueries::ShadowIncludingRoot(newParent) != &TreeQueries::ShadowIncludingRoot(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 2. If node is a host-including inclusive ancestor of newParent, then throw a "HierarchyRequestError"
    //    DOMException.
    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, newParent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 3. If child is non-null and its parent is not newParent, then throw a "NotFoundError" DOMException.
    if (child != nullptr && child->ParentNode() != &newParent)
    {
      return ExceptionCode::NotFoundError;
    }

    // 4. If node is not an Element or a CharacterData node, then throw a "HierarchyRequestError"
    // DOMException.
    if (!IsOneOf<Element, CharacterData>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 5. If node is a Text node and newParent is a document, then throw a "HierarchyRequestError"
    // DOMException.
    if (Is<Text>(node) && Is<Document>(newParent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 6. If newParent is a document, node is an Element node, and either newParent has an element child,
    // child
    //    is a doctype, or child is non-null and a doctype is following child then throw a
    //    "HierarchyRequestError" DOMException.
    if (Is<Document>(newParent) && Is<Element>(node)
        && (HasElementChild(newParent) || IsDocTypeOrDocTypeFollows(child)))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 7. Let oldParent be node's parent.
    // 8. Assert: oldParent is non-null.
    krys_debug_assert(node.ParentNode() != nullptr);
    auto &oldParent = *node.ParentNode();

    // 9. Run the live range pre-remove steps, given node.
    LiveRangeUpdater::PreRemove(node);

    // 10. For each NodeIterator object iterator whose root's node document is node's node document: run the
    //     NodeIterator pre-remove steps given node and iterator.
    for (auto iterator : node.NodeDocument().NodeIterators())
    {
      if (&iterator->Root().NodeDocument() == &node.NodeDocument())
      {
        IteratorAlgorithms::PreRemove(*iterator, node);
      }
    }

    // 11. Let oldPreviousSibling be node's previous sibling.
    auto *oldPreviousSibling = node.PreviousSibling();

    // 12. Let oldNextSibling be node's next sibling.
    auto *oldNextSibling = node.NextSibling();

    node.NodeDocument().NodeWillBeRemoved(node);

    {
      ChildChangeStyleInvalidation removalStyleInvalidation(oldParent,
                                                            MakeChildChangeForRemoval(oldParent, node));

      // A moved subtree is styled again in its new place.
      ExtensibilityHooks::DestroyRenderTreeIfNeeded(node);

      // 13. Remove node from oldParent's children.
      if (auto previousSibling = node.PreviousSibling())
      {
        previousSibling->SetNextSibling(node.NextSibling());
      }
      else
      {
        krys_debug_assert(oldParent.FirstChild() == &node);
        oldParent.SetFirstChild(node.NextSibling());
      }

      if (auto nextSibling = node.NextSibling())
      {
        nextSibling->SetPreviousSibling(node.PreviousSibling());
      }
      else
      {
        krys_debug_assert(oldParent.LastChild() == &node);
        oldParent.SetLastChild(node.PreviousSibling());
      }
      krys_debug_assert(oldParent.FirstChild() != &node);
      krys_debug_assert(oldParent.LastChild() != &node);
      node.SetParentNode(nullptr);
      node.SetPreviousSibling(nullptr);
      node.SetNextSibling(nullptr);

      // 14. If node is assigned, then run assign slottables for node's assigned slot.
      if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(node))
      {
        SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      // 15. If oldParent's root is a shadow root, and oldParent is a slot whose assigned nodes is empty, then
      //     run signal a slot change for oldParent.
      auto &oldParentRoot = TreeQueries::Root(oldParent);
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(oldParentRoot))
      {
        if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(oldParent);
            slot && slot->_assignedNodes.empty())
        {
          SlotAlgorithms::SignalSlotChange(*slot);
        }
      }

      // 16. If node has an inclusive descendant that is a slot:
      if (HasNodeOfType<html::HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
      {
        // 16a. Run assign slottables for a tree with oldParent's root.
        SlotAlgorithms::AssignSlottablesForTree(oldParentRoot);

        // 16b. Run assign slottables for a tree with node.
        SlotAlgorithms::AssignSlottablesForTree(node);
      }
    }

    // 17. Add transient registered observers given node and oldParent.
    MutationObserverAlgorithms::AddTransientRegisteredObservers(node, oldParent);

    // 18. If child is non-null:
    //   18a. For each live range whose start node is newParent and start offset is greater than child's
    //   index:
    //        increase its start offset by 1.
    //   18b. For each live range whose end node is newParent and end offset is greater than child's index:
    //        increase its end offset by 1.
    if (child != nullptr)
    {
      LiveRangeUpdater::MovedBeforeNode(*child);
    }

    // 19. Let newPreviousSibling be child's previous sibling if child is non-null, and newParent's last child
    //     otherwise.
    auto *newPreviousSibling = child != nullptr ? child->PreviousSibling() : newParent.LastChild();

    {
      ChildChangeStyleInvalidation insertionStyleInvalidation(
        newParent, MakeChildChangeForInsertion(newParent, node, child));

      // 20. If child is null, then append node to newParent's children.
      // 21. Otherwise, insert node into newParent's children before child's index.
      if (child == nullptr)
      {
        if (auto *lastChild = newParent.LastChild())
        {
          node.SetPreviousSibling(lastChild);
          lastChild->SetNextSibling(&node);
        }
        else
        {
          newParent.SetFirstChild(&node);
        }

        newParent.SetLastChild(&node);
      }
      else
      {
        if (auto *previousSibling = child->PreviousSibling())
        {
          previousSibling->SetNextSibling(&node);
          node.SetPreviousSibling(previousSibling);

          child->SetPreviousSibling(&node);
          node.SetNextSibling(child);
        }
        else
        {
          krys_debug_assert(newParent.FirstChild() == child);
          newParent.SetFirstChild(&node);

          child->SetPreviousSibling(&node);
          node.SetNextSibling(child);
        }
      }
      node.SetParentNode(&newParent);

      // 22. If newParent is a shadow host whose shadow root's slot assignment is "named" and node is a
      //     slottable, then assign a slot for node.
      // As on insertion, a manual shadow root's host assigns a slot too.
      if (TreeQueries::IsShadowHost(newParent) && SlotAlgorithms::IsSlottable(node))
      {
        SlotAlgorithms::AssignSlot(node);
      }

      // 23. If newParent's root is a shadow root, and newParent is a slot whose assigned nodes is empty, then
      //     run signal a slot change for newParent.
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(newParent);
          slot && Is<ShadowRoot>(TreeQueries::Root(newParent)) && slot->_assignedNodes.empty())
      {
        SlotAlgorithms::SignalSlotChange(*slot);
      }

      // 24. Run assign slottables for a tree with node's root.
      SlotAlgorithms::AssignSlottablesForTree(TreeQueries::Root(node));

      // 25. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including
      // tree
      //     order:
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        // 25a. Let isSubtreeRoot be true if inclusiveDescendant is node; otherwise false.
        auto isSubtreeRoot = &inclusiveDescendant == &node;

        // 25b. Run the moving steps with inclusiveDescendant, isSubtreeRoot, and oldParent.
        ExtensibilityHooks::NodeMoved(inclusiveDescendant, isSubtreeRoot, oldParent);

        // 25c. If inclusiveDescendant is custom and newParent is connected, then enqueue a custom element
        //      callback reaction with inclusiveDescendant, callback name "connectedMoveCallback", and « ».
        if (auto *element = DynamicDowncast<Element>(inclusiveDescendant);
            element && element->IsDefinedCustomElement() && newParent.IsConnected())
        {
          html::CustomElementReactionQueue::EnqueueConnectedMoveCallbackIfNeeded(*element);
        }
      }
    }

    // 26. Queue a tree mutation record for oldParent with « », « node », oldPreviousSibling, and
    // oldNextSibling.
    MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(oldParent), {}, {ShareRef(node)},
                                                        ShareRefPtr(oldPreviousSibling),
                                                        ShareRefPtr(oldNextSibling));

    // 27. Queue a tree mutation record for newParent with « node », « », newPreviousSibling, and child.
    MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(newParent), {ShareRef(node)}, {},
                                                        ShareRefPtr(newPreviousSibling), ShareRefPtr(child));

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::Replace(Node &child, Node &node, ContainerNode &parent) noexcept
  {
    // 1. Ensure pre-insert validity given node, parent, child, and « child ».
    Node *childrenToExclude[] {&child};
    if (auto result = EnsurePreInsertValidity(node, parent, &child, childrenToExclude); result.HasException())
    {
      return result.ReleaseException();
    }

    // 2. Let referenceChild be child's next sibling.
    auto referenceChild = child.NextSibling();

    // 3. If referenceChild is node, then set referenceChild to node's next sibling.
    if (referenceChild == &node)
    {
      referenceChild = node.NextSibling();
    }

    // 4. Let previousSibling be child's previous sibling.
    auto previousSibling = child.PreviousSibling();

    // 5. Let removedNodes be the empty set.
    SmallNodeList removedNodes;

    // 6. Adopt node into parent's node document.
    if (auto result = DocumentAlgorithms::AdoptNode(node, parent.NodeDocument()); result.HasException())
    {
      return result.ReleaseException();
    }

    // 7. If child's parent is non-null:
    if (child.ParentNode())
    {
      // 7a. Set removedNodes to « child ».
      removedNodes.push_back(ShareRef(child));

      // 7b. Remove child with suppressObservers set to true.
      if (auto result = Remove(child, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 8. Let nodes be node's children if node is a DocumentFragment node; otherwise « node ».
    SmallNodeList nodes;
    if (Is<DocumentFragment>(node))
    {
      nodes = CollectChildNodes(Downcast<ContainerNode>(node));
    }
    else
    {
      nodes.push_back(ShareRef(node));
    }

    // 9. Insert node into parent before referenceChild with suppressObservers set to true.
    if (auto result = Insert(node, parent, referenceChild, SuppressObservers(true)); result.HasException())
    {
      return result.ReleaseException();
    }

    // 10. Queue a tree mutation record for parent with nodes, removedNodes, previousSibling, and
    // referenceChild.
    MutationObserverAlgorithms::QueueTreeMutationRecord(
      ShareRef(parent), krys::move(nodes), krys::move(removedNodes), ShareRefPtr(previousSibling),
      ShareRefPtr(referenceChild));

    // 11. Return child.
    return child;
  }

  ExceptionOr<void> MutationAlgorithms::ReplaceAll(Node *node, ContainerNode &parent) noexcept
  {
    // 1. Let removedNodes be parent's children.
    SmallNodeList removedNodes = CollectChildNodes(parent);

    // 2. Let addedNodes be the empty set.
    SmallNodeList addedNodes;

    // 3. If node is a DocumentFragment node, then set addedNodes to node's children.
    if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
    {
      addedNodes = CollectChildNodes(*documentFragment);
    }
    // 4. Otherwise, if node is non-null, set addedNodes to « node ».
    else if (node != nullptr)
    {
      addedNodes.push_back(ShareRef(*node));
    }

    // 5. Remove all parent's children, in tree order, with suppressObservers set to true.
    while (auto *firstChild = parent.FirstChild())
    {
      if (auto result = Remove(*firstChild, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 6. If node is non-null, then insert node into parent before null with suppressObservers set to true.
    if (node != nullptr)
    {
      if (auto result = Insert(*node, parent, nullptr, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 7. If either addedNodes or removedNodes is not empty, then queue a tree mutation record for parent with
    //    addedNodes, removedNodes, null, and null.
    if (!addedNodes.empty() || !removedNodes.empty())
    {
      MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(parent), krys::move(addedNodes),
                                                          krys::move(removedNodes), nullptr, nullptr);
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::PreRemove(Node &node, ContainerNode &parent) noexcept
  {
    // 1. If child's parent is not parent, then throw a "NotFoundError" DOMException.
    if (node.ParentNode() != &parent)
    {
      return ExceptionCode::NotFoundError;
    }

    // 2. Remove child.
    if (auto result = Remove(node); result.HasException())
    {
      return result.ReleaseException();
    }

    // 3. Return child.
    return node;
  }

  ExceptionOr<void> MutationAlgorithms::Remove(Node &node, SuppressObservers suppressObservers) noexcept
  {
    auto protectedNode = Protect(node);

    // 1. Let parent be node's parent.
    // 2. Assert: parent is non-null.
    krys_debug_assert(node.ParentNode() != nullptr);
    auto &parent = *node.ParentNode();

    // 3. Run the live range pre-remove steps, given node.
    LiveRangeUpdater::PreRemove(node);

    // 4. For each NodeIterator object iterator whose root's node document is node's node document: run the
    //    NodeIterator pre-remove steps given node and iterator.
    for (auto iterator : node.NodeDocument().NodeIterators())
    {
      if (&iterator->Root().NodeDocument() == &node.NodeDocument())
      {
        IteratorAlgorithms::PreRemove(*iterator, node);
      }
    }

    // 5. Let oldPreviousSibling be node's previous sibling.
    auto oldPreviousSibling = node.PreviousSibling();

    // 6. Let oldNextSibling be node's next sibling.
    auto oldNextSibling = node.NextSibling();

    node.NodeDocument().NodeWillBeRemoved(node);

    {
      ChildChangeStyleInvalidation styleInvalidation(parent, MakeChildChangeForRemoval(parent, node));

      ExtensibilityHooks::DestroyRenderTreeIfNeeded(node);

      // 7. Remove node from its parent's children.
      if (auto previousSibling = node.PreviousSibling())
      {
        previousSibling->SetNextSibling(node.NextSibling());
      }
      else
      {
        krys_debug_assert(parent.FirstChild() == &node);
        parent.SetFirstChild(node.NextSibling());
      }

      if (auto nextSibling = node.NextSibling())
      {
        nextSibling->SetPreviousSibling(node.PreviousSibling());
      }
      else
      {
        krys_debug_assert(parent.LastChild() == &node);
        parent.SetLastChild(node.PreviousSibling());
      }
      krys_debug_assert(parent.FirstChild() != &node);
      krys_debug_assert(parent.LastChild() != &node);
      node.SetParentNode(nullptr);
      node.SetPreviousSibling(nullptr);
      node.SetNextSibling(nullptr);

      // 8. If node is assigned, then run assign slottables for node's assigned slot.
      if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(node))
      {
        SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      // 9. If parent's root is a shadow root, and parent is a slot whose assigned nodes is the empty list,
      // then
      //    run signal a slot change for parent.
      auto &parentRoot = TreeQueries::Root(parent);
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(parentRoot))
      {
        if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(parent); slot && slot->_assignedNodes.empty())
        {
          SlotAlgorithms::SignalSlotChange(*slot);
        }
      }

      // 10. If node has an inclusive descendant that is a slot:
      if (HasNodeOfType<html::HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
      {
        // 10a. Run assign slottables for a tree with parent's root.
        SlotAlgorithms::AssignSlottablesForTree(parentRoot);

        // 10b. Run assign slottables for a tree with node.
        SlotAlgorithms::AssignSlottablesForTree(node);
      }

      // 11. Run the removing steps with node, true, and parent.
      ExtensibilityHooks::NodeRemoved(node, true, parent);

      // 12. Let isParentConnected be parent's connected.
      bool isParentConnected = parent.IsConnected();

      // 13. If node is custom and isParentConnected is true, then enqueue a custom element callback reaction
      // with
      //     node, callback name "disconnectedCallback", and « ».
      if (auto *element = DynamicDowncast<Element>(node);
          element && element->IsDefinedCustomElement() && isParentConnected)
      {
        html::CustomElementReactionQueue::EnqueueDisconnectedCallbackIfNeeded(*element);
      }

      // 14. For each shadow-including descendant descendant of node, in shadow-including tree order:
      for (auto &descendant : ShadowIncludingDescendantRange(node))
      {
        // 14a. Run the removing steps with descendant, false, and parent.
        ExtensibilityHooks::NodeRemoved(descendant, false, parent);

        // 14b. If descendant is custom and isParentConnected is true, then enqueue a custom element callback
        //      reaction with descendant, callback name "disconnectedCallback", and « ».
        if (auto *element = DynamicDowncast<Element>(descendant);
            element && element->IsDefinedCustomElement() && isParentConnected)
        {
          html::CustomElementReactionQueue::EnqueueDisconnectedCallbackIfNeeded(*element);
        }
      }
    }

    // 15. Add transient registered observers given node and parent.
    MutationObserverAlgorithms::AddTransientRegisteredObservers(node, parent);

    // 16. If suppressObservers is false, then queue a tree mutation record for parent with « », « node »,
    //     oldPreviousSibling, and oldNextSibling.
    if (!suppressObservers)
    {
      MutationObserverAlgorithms::QueueTreeMutationRecord(ShareRef(parent), {}, {krys::move(protectedNode)},
                                                          ShareRefPtr(oldPreviousSibling),
                                                          ShareRefPtr(oldNextSibling));
    }

    // 17. Run the children changed steps for parent.
    ExtensibilityHooks::NodeChildrenChanged(parent);

    RebuildSVGExtensionsElementsIfNecessary(parent);

    return {};
  }
}
