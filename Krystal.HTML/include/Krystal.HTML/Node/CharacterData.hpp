#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
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
                  NodeFlag flags = NodeFlag::None) noexcept;

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

    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;

    /// Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;

    /// Returns the first following sibling that is an element; otherwise null.
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