#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Web/Dom/Mixins/Slottable.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Rendering/RenderCombineText.hpp"
#include "Krystal.Web/Rendering/RenderText.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGForeignObjectElement.hpp"

namespace krys::web
{
  Text::Text(Document &document, DOMString data, CDATAConstructorTag) noexcept
      : CharacterData(document, krys::move(data), dom::NodeType::CDATA_SECTION_NODE)
  {
  }

#pragma region Text

  Text::Text(Document &document, DOMString data) noexcept
      : CharacterData(document, krys::move(data), dom::NodeType::TEXT_NODE)
  {
    // Create a text node:
    // 1. Let text be the result of creating a node that implements Text, given document.
    // 2. Set text's data to data.
    // 3. Return text.
  }

  ExceptionOr<Ref<Text>> Text::SplitText(size_t offset) noexcept
  {
    return dom::TextAlgorithms::Split(*this, offset);
  }

  DOMString Text::WholeText() const noexcept
  {
    // Return the concatenation of the data of the contiguous Text nodes of this, in tree order.
    auto contiguousTextNodes = dom::TextAlgorithms::ContiguousTextNodes(*this);

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
    return dom::mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

  RenderText *Text::Renderer() const noexcept
  {
    return Downcast<RenderText>(Node::Renderer());
  }

  namespace
  {
    /// @brief Whether @p text is in SVG content (not in a `foreignObject`), where it's laid out as SVG text.
    KRYS_NODISCARD bool IsSVGText(const Text &text) noexcept
    {
      krys_debug_assert(text.ParentNode());
      auto *parentElement = DynamicDowncast<SVGElement>(*text.ParentNode());
      return parentElement && !Is<SVGForeignObjectElement>(*parentElement);
    }
  }

  RenderPtr<RenderText> Text::CreateTextRenderer(const RenderStyle &style) noexcept
  {
    if (IsSVGText(*this))
    {
      return CreateRenderer<RenderSVGInlineText>(*this, Data());
    }

    // Text in a `text-combine-upright` box is squeezed into one character's space.
    if (style.HasTextCombine())
    {
      return CreateRenderer<RenderCombineText>(*this, Data());
    }
    return CreateRenderer<RenderText>(RenderObject::Type::Text, *this, Data());
  }

  void Text::UpdateRendererAfterContentChange(size_t offsetOfReplacedData,
                                              size_t lengthOfReplacedData) noexcept
  {
    if (!IsConnected())
    {
      return;
    }

    // A renderer that's going to be rebuilt will get the new text anyway.
    if (HasInvalidRenderer())
    {
      return;
    }

    NodeDocument().UpdateTextRenderer(*this, static_cast<uint32>(offsetOfReplacedData),
                                      static_cast<uint32>(lengthOfReplacedData));
  }
}
