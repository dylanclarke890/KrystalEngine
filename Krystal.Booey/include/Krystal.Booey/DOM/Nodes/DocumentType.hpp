#pragma once

#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class DOMImplementation;
  class NodeAlgorithms;
}

namespace krys::boo
{
  /// @brief A doctype: a node with a name, a public ID and a system ID.
  /// @see https://dom.spec.whatwg.org/#interface-documenttype
  /// @see https://dom.spec.whatwg.org/#concept-doctype
  class DocumentType final : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DocumentType);

    friend class dom::DOMImplementation;
    friend class dom::NodeAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-doctype-name
    DOMString _name;
    /// @see https://dom.spec.whatwg.org/#concept-doctype-publicid
    DOMString _publicId;
    /// @see https://dom.spec.whatwg.org/#concept-doctype-systemid
    DOMString _systemId;

  protected:
    /// @see https://dom.spec.whatwg.org/#create-a-doctype
    DocumentType(Document &document, DOMString name, DOMString publicId = {},
                 DOMString systemId = {}) noexcept;

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
    dom::ExceptionOr<void> Before(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    dom::ExceptionOr<void> After(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    dom::ExceptionOr<void> ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    dom::ExceptionOr<void> Remove() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::DocumentType)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    return node.IsDocumentTypeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
