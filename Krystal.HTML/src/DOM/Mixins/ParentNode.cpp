#include "Krystal.HTML/DOM/Mixins/ParentNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"

namespace Krys::HTML::Mixins
{
  ExceptionOr<Ref<Node>> ParentNode::ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                          Document &document) noexcept
  {
    SmallNodeList nodeList;
    for (auto &nodeOrString : nodes)
    {
      if (std::holds_alternative<DOMString>(nodeOrString))
      {
        DOMString copy = std::get<DOMString>(nodeOrString);
        nodeList.emplace_back(document.CreateTextNode(Krys::Move(copy)));
      }
      else
      {
        nodeList.push_back(std::get<Ref<Node>>(nodeOrString));
      }
    }

    if (nodeList.size() == 1uz)
    {
      return nodeList[0];
    }

    Ref<Node> fragment = CreateRef<DocumentFragment>(document);
    for (auto &node : nodeList)
    {
      if (auto result = MutationAlgorithms::Append(*node, Downcast<ContainerNode>(*fragment));
          result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return fragment;
  }

  RefPtr<Element> ParentNode::FirstElementChild(ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(node));
  }

  RefPtr<const Element> ParentNode::FirstElementChild(const ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(node));
  }

  RefPtr<Element> ParentNode::LastElementChild(ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(node));
  }

  RefPtr<const Element> ParentNode::LastElementChild(const ContainerNode &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(node));
  }

  size_t ParentNode::ChildElementCount(const ContainerNode &node) noexcept
  {
    return Krys::HTML::Count(ConstChildElementRange(node));
  }

  ExceptionOr<void> ParentNode::Prepend(ContainerNode &parent, const List<NodeOrString> &nodes) noexcept
  {
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, parent.FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNode::Append(ContainerNode &parent, const List<NodeOrString> &nodes) noexcept
  {
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::Append(*node.Value(), parent); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNode::ReplaceChildren(ContainerNode &parent,
                                                const List<NodeOrString> &nodes) noexcept
  {
    auto node = ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::EnsurePreInsertValidity(*node.Value(), parent, nullptr);
        result.HasException())
    {
      return result.ReleaseException();
    }

    return MutationAlgorithms::ReplaceAll(node.Value().get(), parent);
  }

  ExceptionOr<void> ParentNode::MoveBefore(ContainerNode &parent, Node &node, RawPtr<Node> refChild) noexcept
  {
    if (refChild == &node)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, parent, refChild);
  }

  ExceptionOr<RefPtr<Element>> ParentNode::QuerySelector(ContainerNode &parent,
                                                         DOMStringView selectors) noexcept
  {
    // TODO(impl): CSS-SELECTORS
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> ParentNode::QuerySelectorAll(ContainerNode &parent,
                                                          DOMStringView selectors) noexcept
  {
    // TODO(impl): CSS-SELECTORS
    return Exception {ExceptionCode::NotSupportedError};
  }
}