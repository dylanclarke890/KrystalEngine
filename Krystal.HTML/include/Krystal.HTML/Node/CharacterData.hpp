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
  private:
    DOMString _data;

  protected:
    CharacterData(Document &document, DOMString &&data, NodeType type,
                  NodeFlag flags = NodeFlag::None) noexcept;

  public:
#pragma region Node

    KRYS_NODISCARD DOMString NodeValue() const noexcept final
    {
      return _data;
    }
    ExceptionOr<void> SetNodeValue(DOMString &&value) noexcept final
    {
      return ReplaceData(0, value.size(), Krys::Move(value));
    }
    KRYS_NODISCARD DOMString TextContent() const noexcept final
    {
      return _data;
    }
    ExceptionOr<void> SetTextContent(DOMString &&value) noexcept final
    {
      return ReplaceData(0, value.size(), Krys::Move(value));
    }

#pragma endregion

    KRYS_NODISCARD const DOMString &Data() const noexcept
    {
      return _data;
    }
    void SetData(const DOMString &data) noexcept;
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _data.size();
    }
    KRYS_NODISCARD ExceptionOr<DOMString> SubstringData(size_t offset, size_t count) const noexcept;
    void AppendData(const DOMString &data) noexcept;
    KRYS_NODISCARD ExceptionOr<void> InsertData(size_t offset, const DOMString &data) noexcept;
    KRYS_NODISCARD ExceptionOr<void> DeleteData(size_t offset, size_t count) noexcept;
    KRYS_NODISCARD ExceptionOr<void> ReplaceData(size_t offset, size_t count, const DOMString &data) noexcept;

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    ExceptionOr<void> Before(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> After(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept;
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    KRYS_NODISCARD RawPtr<Element> PreviousElementSibling() const noexcept;
    KRYS_NODISCARD RawPtr<Element> NextElementSibling() const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CharacterData)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsCharacterDataNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();