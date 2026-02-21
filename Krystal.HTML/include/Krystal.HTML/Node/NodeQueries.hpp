#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  class Node;

  struct NodeQueries
  {
    KRYS_NODISCARD static bool HasSameRoot(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsFollowing(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsAncestorOf(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsChildOf(const Node &parent, const Node &child) noexcept;
    KRYS_NODISCARD static bool IsHostIncludingAncestorOf(Node &node, Node &other) noexcept;
    KRYS_NODISCARD static bool IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept;
  };
}