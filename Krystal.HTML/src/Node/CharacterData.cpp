#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ChildNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  CharacterData::CharacterData(Document &document, DOMString &&data, HTML::NodeType type,
                               NodeFlag flags) noexcept
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
    auto deleteOffset = offset + data.size();
    _data.erase(deleteOffset, count);

    for (auto &range : NodeDocument().LiveRanges())
    {
      if (range->StartContainer() == this)
      {
        if (range->StartOffset() > offset && range->StartOffset() <= offset + count)
        {
          range->SetStart(*this, offset);
        }
        else if (range->StartOffset() > offset + count)
        {
          range->SetStart(*this, range->StartOffset() + data.size() - count);
        }
      }

      if (range->EndContainer() == this)
      {
        if (range->EndOffset() > offset && range->EndOffset() <= offset + count)
        {
          range->SetEnd(*this, offset);
        }
        else if (range->EndOffset() > offset + count)
        {
          range->SetEnd(*this, range->EndOffset() + data.size() - count);
        }
      }
    }

    if (auto parent = ShareRefPtr(ParentNode()))
    {
      TreeMutationDispatcher::ChildrenChanged(*parent);
    }

    return {};
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> CharacterData::Before(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::Before(*this, nodes);
  }

  ExceptionOr<void> CharacterData::After(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::After(*this, nodes);
  }

  ExceptionOr<void> CharacterData::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> CharacterData::Remove() noexcept
  {
    return ChildNodeAlgorithms::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> CharacterData::PreviousElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<Element> CharacterData::PreviousElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<const Element> CharacterData::NextElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

  RefPtr<Element> CharacterData::NextElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

#pragma endregion
}