#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

  private:
    RawPtr<Node> _firstChild;
    RawPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, NodeType type, NodeFlag flags) noexcept;

  public:
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept
    {
      return _firstChild;
    }
    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept
    {
      return _lastChild;
    }

    KRYS_NODISCARD ExceptionOr<void> InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> ReplaceChild(Node &newChild, Node &oldChild) noexcept;
    KRYS_NODISCARD ExceptionOr<void> RemoveChild(Node &child) noexcept;
    KRYS_NODISCARD ExceptionOr<void> AppendChild(Node &newChild) noexcept;

  private:
    KRYS_NODISCARD ExceptionOr<void> EnsurePreInsertValidity(Node &node, RawPtr<Node> refChild) noexcept;

    KRYS_NODISCARD ExceptionOr<Node &> PreInsert(Node &node, RawPtr<Node> refChild) noexcept;

    KRYS_NODISCARD ExceptionOr<void> Insert(Node &node, RawPtr<Node> refChild,
                                            bool suppressObservers = false) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ContainerNode)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();