#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  Text::Text(Document &document, DOMString &&data, HTML::NodeType type, NodeFlag flags) noexcept
      : CharacterData(document, Krys::Move(data), type, flags)
  {
  }

#pragma region Text

  /// @see https://dom.spec.whatwg.org/#concept-text-split
  ExceptionOr<Ref<Text>> Text::SplitText(size_t offset) noexcept
  {
    auto length = Data().size();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    auto count = length - offset;
    auto newData = SubstringData(offset, count);
    if (newData.HasException())
    {
      return newData.ReleaseException();
    }

    auto newNode = CreateRef<Text>(NodeDocument(), newData.ReleaseValue());
    if (auto parent = ShareRefPtr(ParentNode()))
    {
      if (auto insertResult = parent->InsertBefore(*newNode, NextSibling()); insertResult.HasException())
      {
        return insertResult.ReleaseException();
      }

      LiveRangeUpdater::SplitTextNode(*this, *newNode, offset);
    }

    ReplaceData(offset, count, u8"");

    return newNode;
  }

  /// @see https://dom.spec.whatwg.org/#dom-text-wholetext
  DOMString Text::WholeText() const noexcept
  {
    return TreeQueries::ContiguousTextContent(*this);
  }

#pragma endregion

#pragma region Slottables

  RawPtr<HTMLSlotElement> Text::AssignedSlot() noexcept
  {
    return SlotAlgorithms::FindSlot(*this, true);
  }

#pragma endregion
}