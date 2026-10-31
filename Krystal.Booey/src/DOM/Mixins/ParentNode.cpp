#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::mixins
{
  ExceptionOr<Ref<Node>> ParentNode::ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                          Document &document) noexcept
  {
    // 1. Replace each string in nodes with the result of creating a text node given document and the string.
    SmallNodeList nodeList;
    for (auto &nodeOrString : nodes)
    {
      if (std::holds_alternative<DOMString>(nodeOrString))
      {
        DOMString copy = std::get<DOMString>(nodeOrString);
        nodeList.emplace_back(document.CreateTextNode(krys::move(copy)));
      }
      else
      {
        nodeList.push_back(std::get<Ref<Node>>(nodeOrString));
      }
    }

    // 2. If nodes's size is 1, then return nodes[0].
    if (nodeList.size() == 1uz)
    {
      return nodeList[0];
    }

    // 3. Let fragment be the result of creating a document fragment given document.
    Ref<Node> fragment = CreateRef<DocumentFragment>(document);

    // 4. For each node of nodes: append node to fragment.
    for (auto &node : nodeList)
    {
      if (auto result = MutationAlgorithms::Append(*node, Downcast<ContainerNode>(*fragment));
          result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 5. Return fragment.
    return fragment;
  }

  RefPtr<Element> ParentNode::FirstElementChild(ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeQueries::FirstElementChild(node));
  }

  RefPtr<const Element> ParentNode::FirstElementChild(const ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeQueries::FirstElementChild(node));
  }

  RefPtr<Element> ParentNode::LastElementChild(ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeQueries::LastElementChild(node));
  }

  RefPtr<const Element> ParentNode::LastElementChild(const ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeQueries::LastElementChild(node));
  }

  size_t ParentNode::ChildElementCount(const ContainerNode &node) noexcept
  {
    return krys::boo::dom::Count(ConstChildElementRange(node));
  }

  ExceptionOr<void> ParentNode::Prepend(ContainerNode &parent, const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let node be the result of converting nodes into a node given nodes and this's node document.
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 2. Pre-insert node into this before this's first child.
    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, parent.FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNode::Append(ContainerNode &parent, const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let node be the result of converting nodes into a node given nodes and this's node document.
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 2. Append node to this.
    if (auto result = MutationAlgorithms::Append(*node.Value(), parent); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNode::ReplaceChildren(ContainerNode &parent,
                                                const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let node be the result of converting nodes into a node given nodes and this's node document.
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 2. Ensure pre-insert validity given node, this, null, and this's children.
    List<Node *> children;
    for (auto &child : ChildNodeRange(parent))
    {
      children.push_back(&child);
    }
    if (auto result = MutationAlgorithms::EnsurePreInsertValidity(*node.Value(), parent, nullptr, children);
        result.HasException())
    {
      return result.ReleaseException();
    }

    // 3. Replace all with node within this.
    return MutationAlgorithms::ReplaceAll(node.Value().get(), parent);
  }

  ExceptionOr<void> ParentNode::MoveBefore(ContainerNode &parent, Node &node, Node *refChild) noexcept
  {
    // 1. Let referenceChild be child.
    // 2. If referenceChild is node, then set referenceChild to node's next sibling.
    if (refChild == &node)
    {
      refChild = node.NextSibling();
    }

    // 3. Move node into this before referenceChild.
    return MutationAlgorithms::Move(node, parent, refChild);
  }

  ExceptionOr<RefPtr<Element>> ParentNode::QuerySelector(ContainerNode &parent,
                                                         DOMStringView selectors) noexcept
  {
    auto query = parent.NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return ShareRefPtr(query.Value().QueryFirst(parent));
  }

  ExceptionOr<Ref<NodeList>> ParentNode::QuerySelectorAll(ContainerNode &parent,
                                                          DOMStringView selectors) noexcept
  {
    auto query = parent.NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return query.Value().QueryAll(parent);
  }
}