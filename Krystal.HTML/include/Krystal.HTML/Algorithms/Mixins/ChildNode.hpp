#pragma once

#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.HTML/Utils/NodeOrString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Element;
  class Node;
}

namespace Krys::HTML::Mixins
{
  class ChildNode
  {
  public:
    KRYS_NODISCARD static ExceptionOr<void> Before(Node &childNode, const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> After(Node &childNode, const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> ReplaceWith(Node &childNode,
                                                        const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> Remove(Node &childNode) noexcept;
  };
}