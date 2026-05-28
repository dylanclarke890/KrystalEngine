#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Mixins/Slottable.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  Text::Text(Document &document, DOMString &&data, HTML::NodeType type, NodeFlags flags) noexcept
      : CharacterData(document, Krys::Move(data), type, flags)
  {
  }

#pragma region Text

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