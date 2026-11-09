/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::layout
{
  class InlineItemsBuilder;

  /// @brief A run of whitespace or non-whitespace text of an inline text box. Its start and length are byte
  /// offsets into the box's content.
  class InlineTextItem : public InlineItem
  {
    friend class InlineItemsBuilder;
    using InlineItem::TextItemType;

  public:
    KRYS_NODISCARD static InlineTextItem CreateWhitespaceItem(const InlineTextBox &inlineTextBox,
                                                              uint32 start, uint32 length,
                                                              UBiDiLevel bidiLevel, bool isWordSeparator,
                                                              Maybe<InlineLayoutUnit> width) noexcept
    {
      return {inlineTextBox, start,           length, bidiLevel,
              false,         isWordSeparator, width,  TextItemType::Whitespace};
    }

    KRYS_NODISCARD static InlineTextItem CreateNonWhitespaceItem(const InlineTextBox &inlineTextBox,
                                                                 uint32 start, uint32 length,
                                                                 UBiDiLevel bidiLevel,
                                                                 bool hasTrailingSoftHyphen,
                                                                 Maybe<InlineLayoutUnit> width) noexcept
    {
      // FIXME: Use the following list of non-whitespace characters to set the "isWordSeparator" bit:
      // noBreakSpace, ethiopicWordspace, aegeanWordSeparatorLine aegeanWordSeparatorDot ugariticWordDivider.
      return {inlineTextBox,         start, length, bidiLevel,
              hasTrailingSoftHyphen, false, width,  TextItemType::NonWhitespace};
    }

    KRYS_NODISCARD static InlineTextItem CreateEmptyItem(const InlineTextBox &inlineTextBox) noexcept
    {
      krys_debug_assert(inlineTextBox.Content().empty());
      return InlineTextItem {inlineTextBox};
    }

    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return _startOrPosition;
    }
    KRYS_NODISCARD uint32 End() const noexcept
    {
      return Start() + Length();
    }
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return _length;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !Length() && _textItemType == TextItemType::Undefined;
    }
    KRYS_NODISCARD bool IsWhitespace() const noexcept
    {
      return _textItemType == TextItemType::Whitespace;
    }
    KRYS_NODISCARD bool IsWordSeparator() const noexcept
    {
      return _isWordSeparator;
    }
    KRYS_NODISCARD bool IsZeroWidthSpaceSeparator() const noexcept;
    KRYS_NODISCARD bool IsQuirkNonBreakingSpace() const noexcept;
    KRYS_NODISCARD bool IsFullyTrimmable() const noexcept;
    KRYS_NODISCARD bool HasTrailingSoftHyphen() const noexcept
    {
      return _hasTrailingSoftHyphen;
    }
    KRYS_NODISCARD Maybe<InlineLayoutUnit> Width() const noexcept
    {
      return _hasWidth ? Maybe<InlineLayoutUnit> {_width} : Maybe<InlineLayoutUnit> {};
    }

    KRYS_NODISCARD const InlineTextBox &GetInlineTextBox() const noexcept
    {
      return Downcast<InlineTextBox>(LayoutBox());
    }

    KRYS_NODISCARD utf8_stringview Content() const noexcept
    {
      return utf8_stringview {GetInlineTextBox().Content()}.substr(Start(), Length());
    }
    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
    }

    KRYS_NODISCARD InlineTextItem Left(uint32 length) const noexcept;
    KRYS_NODISCARD InlineTextItem Right(uint32 length, Maybe<InlineLayoutUnit> width) const noexcept;

    KRYS_NODISCARD static bool ShouldPreserveSpacesAndTabs(const InlineTextItem &inlineTextItem) noexcept;

  private:
    InlineTextItem(const InlineTextBox &inlineTextBox, uint32 start, uint32 length, UBiDiLevel bidiLevel,
                   bool hasTrailingSoftHyphen, bool isWordSeparator, Maybe<InlineLayoutUnit> width,
                   TextItemType textItemType) noexcept;
    explicit InlineTextItem(const InlineTextBox &inlineTextBox) noexcept;

    KRYS_NODISCARD InlineTextItem Split(size_t leftSideLength) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::InlineTextItem)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::InlineItem &inlineItem) noexcept
  {
    return inlineItem.IsText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
