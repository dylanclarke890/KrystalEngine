#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

    friend class ExtensibilityHooks;
    friend class MutationAlgorithms;

  private:
    CheckedPtr<Node> _firstChild;
    CheckedPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, HTML::NodeType type, NodeFlag flags) noexcept;

  public:
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept
    {
      return _firstChild.get();
    }

    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept
    {
      return _lastChild.get();
    }

    ExceptionOr<Node &> InsertBefore(Node &newChild, RawPtr<Node> refChild) noexcept;
    ExceptionOr<Node &> ReplaceChild(Node &newChild, Node &oldChild) noexcept;
    ExceptionOr<Node &> RemoveChild(Node &child) noexcept;
    ExceptionOr<Node &> AppendChild(Node &newChild) noexcept;

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