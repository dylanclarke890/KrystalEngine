#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/Ref.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode : public Node
  {
  private:
    List<Ref<Node>> _childNodes;

  protected:
    ContainerNode(Document &document, NodeType type, NodeFlags flags) noexcept;

  public:
    KRYS_NODISCARD ExceptionOr<void> InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> ReplaceChild(Node &newChild, Node &oldChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> RemoveChild(Node &child) noexcept;
    KRYS_NODISCARD ExceptionOr<void> AppendChild(Node &newChild) noexcept;


  private:
    /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
    KRYS_NODISCARD static ExceptionOr<void> EnsurePreInsertValidity(ContainerNode &parent, Node &newChild,
                                                                    RefPtr<Node> &&refChild) noexcept;

    KRYS_NODISCARD static bool IsHostIncludingAncestorOf(Node &node, Node &other) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ContainerNode)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()