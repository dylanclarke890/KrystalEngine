#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
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

      // TODO(IMPL): Update live ranges.
      // For each live range whose start node is node and start offset is greater than offset, set its start
      // node to newNode and decrease its start offset by offset.

      // TODO(IMPL): Update live ranges.
      // For each live range whose end node is node and end offset is greater than offset, set its end node to
      // newNode and decrease its end offset by offset.

      // TODO(IMPL): Update live ranges.
      // For each live range whose start node is parent and start offset is equal to the index of node plus 1,
      // increase its start offset by 1.

      // TODO(IMPL): Update live ranges.
      // For each live range whose end node is parent and end offset is equal to the index of node plus 1,
      // increase its end offset by 1.
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
}