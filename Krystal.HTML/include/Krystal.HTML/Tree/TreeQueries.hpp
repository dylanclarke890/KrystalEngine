#pragma once

#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;

  struct TreeQueries
  {
    KRYS_NODISCARD static bool HasSameRoot(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsFollowing(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsAncestorOf(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsChildOf(const Node &parent, const Node &child) noexcept;
    KRYS_NODISCARD static bool IsHostIncludingAncestorOf(Node &node, Node &other) noexcept;
    KRYS_NODISCARD static bool IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept;

    static void CollectChildNodes(const ContainerNode &parent, SmallNodeList &collection) noexcept;
  };
}