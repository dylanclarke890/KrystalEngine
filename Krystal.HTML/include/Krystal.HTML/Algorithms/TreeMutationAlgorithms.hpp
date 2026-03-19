#pragma once

#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class CustomElementRegistry;
  class Document;
  class Node;

  struct SuppressObservers : public StronglyTypedBool<SuppressObservers>
  {
    using Base::Base;
  };

  class TreeMutationAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
    KRYS_NODISCARD static ExceptionOr<void> EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                    RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-pre-insert
    KRYS_NODISCARD static ExceptionOr<void> PreInsert(Node &node, ContainerNode &parent,
                                                      RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-insert
    KRYS_NODISCARD static ExceptionOr<void>
      Insert(Node &node, ContainerNode &parent, RawPtr<Node> refChild,
             SuppressObservers suppressObservers = SuppressObservers(false)) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-append
    KRYS_NODISCARD static ExceptionOr<void> Append(Node &node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#move
    KRYS_NODISCARD static ExceptionOr<void> Move(Node &node, ContainerNode &newParent,
                                                 RawPtr<Node> refChild) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-pre-remove
    KRYS_NODISCARD static ExceptionOr<void> PreRemove(Node &node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove
    KRYS_NODISCARD static ExceptionOr<void>
      Remove(Node &node, ContainerNode &parent,
             SuppressObservers suppressObservers = SuppressObservers(false)) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone
    static Ref<Node> CloneNode(Node &node, RawPtr<Document> document = nullptr, bool subtree = false,
                               RawPtr<ContainerNode> parent = nullptr,
                               RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#clone-a-single-node
    KRYS_NODISCARD static Ref<Node>
      CloneSingleNode(Node &node, Document &document,
                      RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    // TODO(fix): replace this with a call to ReplaceAll with a null node.
    KRYS_NODISCARD static ExceptionOr<void>
      RemoveAllChildren(ContainerNode &parent,
                        SuppressObservers suppressObservers = SuppressObservers(false)) noexcept;
  };
}