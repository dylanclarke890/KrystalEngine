#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  CharacterData::CharacterData(Document &document, DOMString &&data, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsCharacterData), _data {Krys::Move(data)}
  {
  }

  void CharacterData::SetData(const DOMString &data) noexcept
  {
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-substring
  ExceptionOr<DOMString> CharacterData::SubstringData(size_t offset, size_t count) const noexcept
  {
    size_t length = Length();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    size_t availableCount = length - offset;
    if (count < availableCount)
    {
      return _data.substr(offset, count);
    }

    return _data.substr(offset);
  }

  void CharacterData::AppendData(const DOMString &data) noexcept
  {
  }

  ExceptionOr<void> CharacterData::InsertData(size_t offset, const DOMString &data) noexcept
  {
    return {};
  }

  ExceptionOr<void> CharacterData::DeleteData(size_t offset, size_t count) noexcept
  {
    return {};
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-replace
  ExceptionOr<void> CharacterData::ReplaceData(size_t offset, size_t count, const DOMString &data) noexcept
  {
    // TODO (IMPL): needs mutation records and live ranges
    return {};
  }

  ExceptionOr<void> CharacterData::SetNodeValue(DOMStringView value) noexcept
  {
    // TODO (IMPL): 
    return {};
  }
}