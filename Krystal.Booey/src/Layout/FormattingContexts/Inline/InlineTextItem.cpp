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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo::layout
{
  static_assert(sizeof(InlineItem) == sizeof(InlineTextItem));

  InlineTextItem::InlineTextItem(const InlineTextBox &inlineTextBox, uint32 start, uint32 length,
                                 UBiDiLevel bidiLevel, bool hasTrailingSoftHyphen, bool isWordSeparator,
                                 Maybe<InlineLayoutUnit> width, TextItemType textItemType) noexcept
      : InlineItem(inlineTextBox, Type::Text, bidiLevel)
  {
    _startOrPosition = start;
    _length = length;
    _hasWidth = width.has_value();
    _hasTrailingSoftHyphen = hasTrailingSoftHyphen;
    _isWordSeparator = isWordSeparator;
    _width = width.value_or(0);
    _textItemType = textItemType;
  }

  InlineTextItem::InlineTextItem(const InlineTextBox &inlineTextBox) noexcept
      : InlineItem(inlineTextBox, Type::Text, UBIDI_DEFAULT_LTR)
  {
  }

  InlineTextItem InlineTextItem::Left(uint32 length) const noexcept
  {
    krys_assert(length <= Length());
    krys_debug_assert(_textItemType != TextItemType::Undefined);
    krys_debug_assert(length);
    return {GetInlineTextBox(), Start(), length, BidiLevel(), false, IsWordSeparator(), null, _textItemType};
  }

  InlineTextItem InlineTextItem::Right(uint32 length, Maybe<InlineLayoutUnit> width) const noexcept
  {
    krys_assert(length <= Length());
    krys_debug_assert(_textItemType != TextItemType::Undefined);
    krys_debug_assert(length);
    return {GetInlineTextBox(),      End() - length,    length, BidiLevel(),
            HasTrailingSoftHyphen(), IsWordSeparator(), width,  _textItemType};
  }

  InlineTextItem InlineTextItem::Split(size_t leftSideLength) noexcept
  {
    krys_assert(Length() > 1);
    krys_assert(leftSideLength && leftSideLength < Length());
    auto rightSide = Right(static_cast<uint32>(Length() - leftSideLength), {});
    _length = Length() - rightSide.Length();
    _hasWidth = false;
    _width = {};
    return rightSide;
  }

  bool InlineTextItem::IsZeroWidthSpaceSeparator() const noexcept
  {
    // FIXME: We should check for more zero width content and not just U+200B.
    if (!_length)
    {
      return true;
    }
    auto content = Content();
    auto decoded = text::DecodeUTF8(Span<const char8>(content.data(), content.size()));
    return decoded.Length == _length && decoded.CodePoint == text::ZeroWidthSpace;
  }

  bool InlineTextItem::IsQuirkNonBreakingSpace() const noexcept
  {
    // Only `nbsp-mode: space` makes a no-break space breakable, and that mode isn't supported.
    return false;
  }

  bool InlineTextItem::IsFullyTrimmable() const noexcept
  {
    return IsWhitespace() && !TextUtil::ShouldPreserveSpacesAndTabs(LayoutBox());
  }

  bool InlineTextItem::ShouldPreserveSpacesAndTabs(const InlineTextItem &inlineTextItem) noexcept
  {
    krys_debug_assert(inlineTextItem.IsWhitespace());
    return TextUtil::ShouldPreserveSpacesAndTabs(inlineTextItem.LayoutBox());
  }
}
