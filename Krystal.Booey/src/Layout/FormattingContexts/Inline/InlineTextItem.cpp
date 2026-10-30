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
