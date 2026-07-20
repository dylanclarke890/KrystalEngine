#pragma once

#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/NodeOrString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#interface-documenttype
  class DocumentType final : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DocumentType);

    friend class DOMImplementation;
    friend class NodeAlgorithms;

  private:
    DOMString _name;
    DOMString _publicId;
    DOMString _systemId;

  protected:
    DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                 const DOMString &systemId) noexcept;

  public:
#pragma region DocumentType

    /// @see https://dom.spec.whatwg.org/#dom-documenttype-name
    KRYS_NODISCARD const DOMString &Name() const noexcept
    {
      return _name;
    }

    /// @see https://dom.spec.whatwg.org/#dom-documenttype-publicid
    KRYS_NODISCARD const DOMString &PublicId() const noexcept
    {
      return _publicId;
    }

    /// @see https://dom.spec.whatwg.org/#dom-documenttype-systemid
    KRYS_NODISCARD const DOMString &SystemId() const noexcept
    {
      return _systemId;
    }

#pragma endregion

#pragma region Node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return _name;
    }

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    /// @brief Inserts `nodes` just before `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-before
    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    ExceptionOr<void> Remove() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::DocumentType)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentTypeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();