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

  public:
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

    KRYS_NODISCARD DOMString NodeValue() const noexcept final
    {
      return _data;
    }
    KRYS_NODISCARD ExceptionOr<void> SetNodeValue(DOMStringView value) noexcept final;

  protected:
    CharacterData(Document &document, DOMString &&data, NodeType type, NodeFlags flags = NodeFlags::None) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CharacterData)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsCharacterDataNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()