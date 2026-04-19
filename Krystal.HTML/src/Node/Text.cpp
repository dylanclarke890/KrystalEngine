#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SlotAssignmentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  Text::Text(Document &document, DOMString &&data, NodeType type, NodeFlag flags) noexcept
      : CharacterData(document, Krys::Move(data), type, flags | NodeFlag::IsTextNode)
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

    auto newNode = CreateRef<Text>(*OwnerDocument(), newData.ReleaseValue());
    if (auto parent = ShareRefPtr(ParentNode()))
    {
      if (auto insertResult = parent->InsertBefore(*newNode, ShareRefPtr(NextSibling()));
          insertResult.HasException())
      {
        return insertResult.ReleaseException();
      }

      auto index = TreeQueries::Index(*this);
      for (auto &range : NodeDocument().LiveRanges())
      {
        if (range->StartContainer() == this && range->StartOffset() > offset)
        {
          range->SetStart(*newNode, range->StartOffset() - offset);
        }

        if (range->EndContainer() == this)
        {
          range->SetEnd(*newNode, range->EndOffset() - offset);
        }

        if (range->StartContainer() == ParentNode() && range->StartOffset() == index + 1)
        {
          range->SetStart(*range->StartContainer(), range->StartOffset() + 1);
        }

        if (range->EndContainer() == ParentNode() && range->EndOffset() == index + 1)
        {
          range->SetEnd(*range->EndContainer(), range->EndOffset() + 1);
        }
      }
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
    return SlotAssignmentAlgorithms::FindSlot(*this, true);
  }

#pragma endregion
}