/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2018 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGForeignObjectElement.hpp"

namespace krys::boo::dom
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
    return TextAlgorithms::Split(*this, offset);
  }

  DOMString Text::WholeText() const noexcept
  {
    // Return the concatenation of the data of the contiguous Text nodes of this, in tree order.
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

  RefPtr<html::HTMLSlotElement> Text::AssignedSlot() noexcept
  {
    return mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

  RenderText *Text::Renderer() const noexcept
  {
    return Downcast<RenderText>(Node::Renderer());
  }

  namespace
  {
    KRYS_NODISCARD bool IsSVGText(const Text &text) noexcept
    {
      krys_debug_assert(text.ParentNode());
      auto *parentElement = DynamicDowncast<svg::SVGElement>(*text.ParentNode());
      return parentElement && !Is<svg::SVGForeignObjectElement>(*parentElement);
    }
  }

  RenderPtr<RenderText> Text::CreateTextRenderer(const RenderStyle &) noexcept
  {
    if (IsSVGText(*this))
    {
      return CreateRenderer<RenderSVGInlineText>(*this, Data());
    }

    // FIXME: text-combine-upright (RenderCombineText), once there's that renderer.
    return CreateRenderer<RenderText>(RenderObject::Type::Text, *this, Data());
  }

  void Text::UpdateRendererAfterContentChange(size_t offsetOfReplacedData,
                                              size_t lengthOfReplacedData) noexcept
  {
    if (!IsConnected())
    {
      return;
    }

    if (HasInvalidRenderer())
    {
      return;
    }

    NodeDocument().UpdateTextRenderer(*this, static_cast<uint32>(offsetOfReplacedData),
                                      static_cast<uint32>(lengthOfReplacedData));
  }
}
