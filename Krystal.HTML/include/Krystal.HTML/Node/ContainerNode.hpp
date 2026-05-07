#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class MutationAlgorithms;
  class TreeMutationDispatcher;

  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

    friend class MutationAlgorithms;
    friend class TreeMutationDispatcher;

  private:
    CheckedPtr<Node> _firstChild;
    CheckedPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, HTML::NodeType type, NodeFlag flags) noexcept;

  public:
#pragma region Node

    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept
    {
      return _firstChild.get();
    }

    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept
    {
      return _lastChild.get();
    }

    ExceptionOr<Node &> InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept;
    ExceptionOr<Node &> ReplaceChild(Node &newChild, Node &oldChild) noexcept;
    ExceptionOr<Node &> RemoveChild(Node &child) noexcept;
    ExceptionOr<Node &> AppendChild(Node &newChild) noexcept;

    KRYS_NODISCARD size_t CountChildNodes() const noexcept;

#pragma endregion

  protected:
    virtual void OnChildrenChanged() noexcept
    {
    }

    void SetFirstChild(RawPtr<Node> child) noexcept
    {
      _firstChild = ShareCheckedPtr(child);
    }

    void SetLastChild(RawPtr<Node> child) noexcept
    {
      _lastChild = ShareCheckedPtr(child);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ContainerNode)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();