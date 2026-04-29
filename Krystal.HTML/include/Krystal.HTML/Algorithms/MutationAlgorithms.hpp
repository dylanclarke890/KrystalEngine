#pragma once

#include "Krystal.HTML/Node/InsertAdjacentWhere.hpp"
#include "Krystal.HTML/Node/NodeOrString.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class CustomElementRegistry;
  class Document;
  class Element;
  class Node;

  struct SuppressObservers : public StronglyTypedBool<SuppressObservers>
  {
    using Base::Base;
  };

  class MutationAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
    KRYS_NODISCARD static ExceptionOr<void> EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                    RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-pre-insert
    KRYS_NODISCARD static ExceptionOr<Node &> PreInsert(Node &node, ContainerNode &parent,
                                                        RawPtr<Node> child) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-insert
    KRYS_NODISCARD static ExceptionOr<void>
      Insert(Node &node, ContainerNode &parent, RawPtr<Node> refChild,
             SuppressObservers suppressObservers = SuppressObservers(false)) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-append
    KRYS_NODISCARD static ExceptionOr<Node &> Append(Node &node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#move
    KRYS_NODISCARD static ExceptionOr<void> Move(Node &node, ContainerNode &newParent,
                                                 RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-replace
    KRYS_NODISCARD static ExceptionOr<Node &> Replace(Node &child, Node &node,
                                                      ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-replace-all
    KRYS_NODISCARD static ExceptionOr<void> ReplaceAll(RawPtr<Node> node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-pre-remove
    KRYS_NODISCARD static ExceptionOr<Node &> PreRemove(Node &node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove
    KRYS_NODISCARD static ExceptionOr<void>
      Remove(Node &node, SuppressObservers suppressObservers = SuppressObservers(false)) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone
    static Ref<Node> CloneNode(Node &node, RawPtr<Document> document = nullptr, bool subtree = false,
                               RawPtr<ContainerNode> parent = nullptr,
                               RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#clone-a-single-node
    KRYS_NODISCARD static Ref<Node>
      CloneSingleNode(Node &node, Document &document,
                      RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#convert-nodes-into-a-node
    KRYS_NODISCARD static ExceptionOr<Ref<Node>> ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                                      Document &document) noexcept;

    /// @see https://dom.spec.whatwg.org/#insert-adjacent
    KRYS_NODISCARD static ExceptionOr<RawPtr<Node>>
      InsertAdjacent(Element &element, InsertAdjacentWhere where, Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#string-replace-all
    static ExceptionOr<void> StringReplaceAll(DOMString&& string, ContainerNode& parent) noexcept;
  };
}