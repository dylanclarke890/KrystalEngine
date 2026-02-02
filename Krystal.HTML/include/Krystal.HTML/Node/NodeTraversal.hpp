#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;
  class Text;

  class NodeTraversal
  {
  public:
    KRYS_NODISCARD static const Node &Root(const Node &node) noexcept;
    KRYS_NODISCARD static Node &Root(Node &node) noexcept;

    KRYS_NODISCARD static RawPtr<Node> Next(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Next(const Text &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Next(const ContainerNode &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Next(const Node &current, RawPtr<const Node> stayWithin) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Next(const Text &current, RawPtr<const Node> stayWithin) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Next(const ContainerNode &current,
                                            RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> NextSkippingChildren(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> NextSkippingChildren(const Node &current,
                                                            RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> NextAncestorSibling(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> NextAncestorSibling(const Node &current,
                                                           RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> DeepLastChild(Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Last(const ContainerNode &current) noexcept;

    KRYS_NODISCARD static RawPtr<Node> Previous(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> Previous(const Node &current, RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildren(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildren(const Node &current,
                                                                RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> NextPostOrder(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> NextPostOrder(const Node &current,
                                                     RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> PreviousPostOrder(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> PreviousPostOrder(const Node &current,
                                                         RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildrenPostOrder(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node>
      PreviousSkippingChildrenPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<Node> PreviousAncestorSiblingPostOrder(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node>
      PreviousAncestorSiblingPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept;
  };
}