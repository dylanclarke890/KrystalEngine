#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.HTML/DOM/Mixins/ChildNode.hpp"
#include "Krystal.HTML/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
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

  ExceptionOr<void> CharacterData::Data(DOMString &&data) noexcept
  {
    return ReplaceData(0, Length(), Krys::Move(data));
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-substring
  ExceptionOr<DOMString> CharacterData::SubstringData(size_t offset, size_t count) const noexcept
  {
    auto length = Length();
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

  ExceptionOr<void> CharacterData::AppendData(DOMString &&data) noexcept
  {
    return ReplaceData(_data.size(), 0, Krys::Move(data));
  }

  ExceptionOr<void> CharacterData::InsertData(size_t offset, DOMString &&data) noexcept
  {
    return ReplaceData(offset, 0, Krys::Move(data));
  }

  ExceptionOr<void> CharacterData::DeleteData(size_t offset, size_t count) noexcept
  {
    return ReplaceData(offset, count, u8"");
  }

  /// @see https://dom.spec.whatwg.org/#concept-cd-replace
  ExceptionOr<void> CharacterData::ReplaceData(size_t offset, size_t count, DOMString &&data) noexcept
  {
    auto length = Length();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    if (offset + count > length)
    {
      count = length - offset;
    }

    MutationObserverAlgorithms::QueueMutationRecord(MutationRecordType::CharacterData, ShareRef(*this),
                                                    DOMStringAtom::Null(), DOMStringAtom::Null(), _data, {},
                                                    {}, nullptr, nullptr);
    _data.insert(offset, data);
    auto deleteOffset = offset + data.size();
    _data.erase(deleteOffset, count);

    LiveRangeUpdater::CharacterDataReplaced(*this, offset, count, data.size());

    if (auto parent = ParentNode())
    {
      ExtensibilityHooks::NodeChildrenChanged(*parent);
    }

    return {};
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> CharacterData::Before(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> CharacterData::After(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> CharacterData::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> CharacterData::Remove() noexcept
  {
    return Mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> CharacterData::PreviousElementSibling() const noexcept
  {
    return Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> CharacterData::PreviousElementSibling() noexcept
  {
    return Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> CharacterData::NextElementSibling() const noexcept
  {
    return Mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> CharacterData::NextElementSibling() noexcept
  {
    return Mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion
}