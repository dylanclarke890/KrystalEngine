#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML
{

  ExceptionOr<void> NodeAlgorithms::StringReplaceAll(DOMString &&string, ContainerNode &parent) noexcept
  {
    if (string.empty())
    {
      return MutationAlgorithms::ReplaceAll(nullptr, parent);
    }
    else
    {
      Ref<Text> textNode = CreateRef<Text>(parent.NodeDocument(), Krys::Move(string));
      return MutationAlgorithms::ReplaceAll(textNode.get(), parent);
    }
  }

  Ref<Node> NodeAlgorithms::CloneNode(const Node &node, RawPtr<Document> document, bool subtree,
                                      RawPtr<ContainerNode> parent,
                                      RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    if (document == nullptr)
    {
      document = node.OwnerDocument();
    }

    assert(!Is<Document>(node) || &node == document);

    auto copy = CloneSingleNode(node, *document, fallbackRegistry);
    ExtensibilityHooks::NodeCloned(node, *copy, subtree);

    if (parent != nullptr)
    {
      parent->AppendChild(*copy);
    }

    if (subtree)
    {
      assert(Is<ContainerNode>(node));
      for (auto &child : ConstChildNodeRange(Downcast<ContainerNode>(node)))
      {
        CloneNode(child, document, subtree, Downcast<ContainerNode>(copy.get()), fallbackRegistry);
      }
    }

    if (auto *element = DynamicDowncast<Element>(node))
    {
      auto *elementCopy = DynamicDowncast<Element>(copy.get());
      if (auto shadowRoot = element->ShadowRoot(); shadowRoot && shadowRoot->Clonable())
      {
        assert(elementCopy->ShadowRoot() && !elementCopy->ShadowRoot()->Clonable());
        // TODO(impl):
      }
    }

    return copy;
  }

  Ref<Node> NodeAlgorithms::CloneSingleNode(const Node &node, Document &document,
                                            RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    RefPtr<Node> copy = nullptr;

    if (auto *element = DynamicDowncast<Element>(node))
    {
      // TODO(impl):
    }
    else
    {
      if (Is<Document>(node))
      {
        // TODO(impl):
      }
      else if (Is<DocumentType>(node))
      {
        // TODO(impl):
      }
      else if (Is<Attr>(node))
      {
        // TODO(impl):
      }
      else if (Is<Text>(node) || Is<Comment>(node))
      {
        // TODO(impl):
      }
      else if (Is<ProcessingInstruction>(node))
      {
        // TODO(impl):
      }
    }

    assert(copy);

    return copy;
  }

  ExceptionOr<Ref<Node>> NodeAlgorithms::ConvertNodesIntoNode(const List<NodeOrString> &nodes,
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
}