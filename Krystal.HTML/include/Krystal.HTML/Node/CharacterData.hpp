#pragma once

#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.HTML/Types/NodeOrString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;

  class CharacterData : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CharacterData);

  private:
    DOMString _data;

  protected:
    CharacterData(Document &document, DOMString &&data, HTML::NodeType type,
                  NodeFlags flags = NodeFlags::None) noexcept;

  public:
#pragma region CharacterData - https://dom.spec.whatwg.org/#interface-characterdata

    KRYS_NODISCARD const DOMString &Data() const noexcept
    {
      return _data;
    }
    ExceptionOr<void> Data(DOMString &&data) noexcept;
    size_t Length() const noexcept
    {
      return _data.size();
    }

    ExceptionOr<DOMString> SubstringData(size_t offset, size_t count) const noexcept;
    ExceptionOr<void> AppendData(DOMString &&data) noexcept;
    ExceptionOr<void> InsertData(size_t offset, DOMString &&data) noexcept;
    ExceptionOr<void> DeleteData(size_t offset, size_t count) noexcept;
    ExceptionOr<void> ReplaceData(size_t offset, size_t count, DOMString &&data) noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD Maybe<DOMString> NodeValue() const noexcept final
    {
      return _data;
    }

    ExceptionOr<void> NodeValue(DOMString &&value) noexcept final
    {
      return Data(Krys::Move(value));
    }

    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final
    {
      return _data;
    }

    ExceptionOr<void> TextContent(DOMString &&value) noexcept final
    {
      return Data(Krys::Move(value));
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

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<Element> NextElementSibling() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CharacterData)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsCharacterDataNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();