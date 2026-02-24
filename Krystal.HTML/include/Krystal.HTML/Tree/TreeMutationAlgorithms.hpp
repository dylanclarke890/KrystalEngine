#pragma once

#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  class ContainerNode;
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
    KRYS_NODISCARD static ExceptionOr<void> Insert(Node &node, ContainerNode &parent, RawPtr<Node> refChild,
                                                   SuppressObservers suppressObservers) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-pre-remove
    KRYS_NODISCARD static ExceptionOr<void> PreRemove(Node &node, ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove
    KRYS_NODISCARD static ExceptionOr<void> Remove(Node &node, ContainerNode &parent,
                                                   SuppressObservers suppressObservers) noexcept;
  };
}