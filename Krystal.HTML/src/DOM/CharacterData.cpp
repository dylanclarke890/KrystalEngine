#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Mixins/ChildNode.hpp"
#include "Krystal.HTML/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  CharacterData::CharacterData(Document &document, DOMString &&data, HTML::NodeType type,
                               NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsCharacterData), _data {Krys::Move(data)}
  {
  }

#pragma region CharacterData

  ExceptionOr<void> CharacterData::Data(DOMString &&data) noexcept
  {
    return TextAlgorithms::Replace(*this, 0uz, Length(), Krys::Move(data));
  }

  ExceptionOr<DOMString> CharacterData::SubstringData(size_t offset, size_t count) const noexcept
  {
    return TextAlgorithms::Substring(*this, offset, count);
  }

  ExceptionOr<void> CharacterData::AppendData(DOMString &&data) noexcept
  {
    return TextAlgorithms::Replace(*this, _data.size(), 0uz, Krys::Move(data));
  }

  ExceptionOr<void> CharacterData::InsertData(size_t offset, DOMString &&data) noexcept
  {
    return TextAlgorithms::Replace(*this, offset, 0uz, Krys::Move(data));
  }

  ExceptionOr<void> CharacterData::DeleteData(size_t offset, size_t count) noexcept
  {
    return TextAlgorithms::Replace(*this, offset, count, u8"");
  }

  ExceptionOr<void> CharacterData::ReplaceData(size_t offset, size_t count, DOMString &&data) noexcept
  {
    return TextAlgorithms::Replace(*this, offset, count, Krys::Move(data));
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