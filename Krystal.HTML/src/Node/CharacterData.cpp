#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  CharacterData::CharacterData(Document &document, DOMString &&data, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsCharacterData), _data {Krys::Move(data)}
  {
  }

#pragma region CharacterData

  ExceptionOr<void> CharacterData::SetData(const DOMString &data) noexcept
  {
    return ReplaceData(0, _data.size(), data);
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-substring
  ExceptionOr<DOMString> CharacterData::SubstringData(size_t offset, size_t count) const noexcept
  {
    auto length = _data.size();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    if (offset + count > length)
    {
      return _data.substr(offset);
    }

    return _data.substr(offset, count);
  }

  ExceptionOr<void> CharacterData::AppendData(const DOMString &data) noexcept
  {
    return ReplaceData(_data.size(), 0, data);
  }

  ExceptionOr<void> CharacterData::InsertData(size_t offset, const DOMString &data) noexcept
  {
    return ReplaceData(offset, 0, data);
  }

  ExceptionOr<void> CharacterData::DeleteData(size_t offset, size_t count) noexcept
  {
    return ReplaceData(offset, count, u8"");
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-replace
  ExceptionOr<void> CharacterData::ReplaceData(size_t offset, size_t count, const DOMString &data) noexcept
  {
    auto length = _data.size();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    if (offset + count > length)
    {
      count = length - offset;
    }

    TreeMutationDispatcher::QueueMutationRecord(u8"characterData", *this, std::nullopt, std::nullopt, _data,
                                                {}, {}, nullptr, nullptr);
    _data.insert(offset, data);
    _data.erase(offset, count);

    // TODO(IMPL):
    // For each live range whose start node is node and start offset is greater than offset but less than or
    // equal to offset + count: set its start offset to offset.

    // TODO(IMPL):
    // For each live range whose end node is node and end offset is greater than offset but less than or equal
    // to offset + count: set its end offset to offset.

    // TODO(IMPL):
    // For each live range whose start node is node and start offset is greater than offset + count: increase
    // its start offset by data’s length and decrease it by count.

    // TODO(IMPL):
    // For each live range whose end node is node and end offset is greater than offset + count: increase its
    // end offset by data’s length and decrease it by count.

    if (auto parent = ShareRefPtr(ParentNode()))
    {
      TreeMutationDispatcher::ChildrenChanged(*parent);
    }

    return {};
  }

#pragma endregion

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

  RawPtr<const Element> CharacterData::PreviousElementSibling() const noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<Element> CharacterData::PreviousElementSibling() noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<const Element> CharacterData::NextElementSibling() const noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

  RawPtr<Element> CharacterData::NextElementSibling() noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

#pragma endregion

}