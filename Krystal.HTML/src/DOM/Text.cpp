#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Mixins/Slottable.hpp"

namespace Krys::HTML
{
  Text::Text(Document &document, DOMString &&data, CDATAConstructorTag) noexcept
      : CharacterData(document, Krys::Move(data), HTML::NodeType::CDATA_SECTION_NODE)
  {
  }

#pragma region Text

  Text::Text(Document &document, DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), HTML::NodeType::TEXT_NODE)
  {
  }

  ExceptionOr<Ref<Text>> Text::SplitText(size_t offset) noexcept
  {
    return TextAlgorithms::Split(*this, offset);
  }

  DOMString Text::WholeText() const noexcept
  {
    auto contiguousTextNodes = TextAlgorithms::ContiguousTextNodes(*this);

    size_t capacity = 0uz;
    for (auto &node : contiguousTextNodes)
    {
      capacity += node->Length();
    }

    DOMString wholeText;
    wholeText.reserve(capacity);

    for (auto &node : contiguousTextNodes)
    {
      wholeText += node->Data();
    }

    return wholeText;
  }

#pragma endregion

#pragma region Slottables

  RefPtr<HTMLSlotElement> Text::AssignedSlot() noexcept
  {
    return Mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion
}