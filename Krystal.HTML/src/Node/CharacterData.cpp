#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"
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

#pragma region ChildNode

  ExceptionOr<void> CharacterData::Before(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> CharacterData::After(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> CharacterData::ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> CharacterData::Remove() noexcept
  {
    if (auto parent = ShareRefPtr(ParentNode()))
    {
      return TreeMutationAlgorithms::Remove(*this, *parent, SuppressObservers(false));
    }

    return {};
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RawPtr<Element> CharacterData::PreviousElementSibling() const noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<Element> CharacterData::NextElementSibling() const noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

#pragma endregion

}