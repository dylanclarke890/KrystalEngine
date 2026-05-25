#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Element;
  class HTMLElement;
  class Node;
  class Text;

  class TreeTraversal
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

    KRYS_NODISCARD static RawPtr<Node> NextShadowIncluding(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Node> NextShadowIncluding(const Node &current,
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

    KRYS_NODISCARD static RawPtr<const Element> NextElementSibling(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Element> NextElementSibling(Node &current) noexcept;

    KRYS_NODISCARD static RawPtr<const Element> PreviousElementSibling(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Element> PreviousElementSibling(Node &current) noexcept;

    KRYS_NODISCARD static RawPtr<const Element> FirstElementChild(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static RawPtr<Element> FirstElementChild(ContainerNode &node) noexcept;

    KRYS_NODISCARD static RawPtr<const Element> LastElementChild(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static RawPtr<Element> LastElementChild(ContainerNode &node) noexcept;

    KRYS_NODISCARD static RawPtr<const Text> NextExclusiveTextNode(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<Text> NextExclusiveTextNode(Node &current) noexcept;

    KRYS_NODISCARD static RawPtr<Text> NextExclusiveTextNode(const Node &current,
                                                             RawPtr<const Node> stayWithin) noexcept;

    KRYS_NODISCARD static RawPtr<const HTMLElement> NextHTMLElementSibling(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<HTMLElement> NextHTMLElementSibling(Node &current) noexcept;

    KRYS_NODISCARD static RawPtr<const HTMLElement> PreviousHTMLElementSibling(const Node &current) noexcept;
    KRYS_NODISCARD static RawPtr<HTMLElement> PreviousHTMLElementSibling(Node &current) noexcept;

    KRYS_NODISCARD static RawPtr<const HTMLElement> FirstHTMLElementChild(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static RawPtr<HTMLElement> FirstHTMLElementChild(ContainerNode &node) noexcept;

    KRYS_NODISCARD static RawPtr<const HTMLElement> LastHTMLElementChild(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static RawPtr<HTMLElement> LastHTMLElementChild(ContainerNode &node) noexcept;
  };
}