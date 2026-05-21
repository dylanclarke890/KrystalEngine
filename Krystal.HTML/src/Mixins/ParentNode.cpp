#include "Krystal.HTML/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
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
    List<Ref<Node>> nodeList;
    for (auto &nodeOrString : nodes)
    {
      if (std::holds_alternative<DOMString>(nodeOrString))
      {
        DOMString copy = std::get<DOMString>(nodeOrString);
        nodeList.emplace_back(CreateRef<Text>(document, Krys::Move(copy)));
      }
      else
      {
        nodeList.push_back(std::get<Ref<Node>>(nodeOrString));
      }
    }

    if (nodeList.size() == 1)
    {
      return nodeList[0];
    }

    auto fragment = CreateRef<DocumentFragment>(document);
    for (auto &node : nodeList)
    {
      if (auto result = MutationAlgorithms::Append(*node, *fragment); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return AdoptRef<Node>(*fragment);
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