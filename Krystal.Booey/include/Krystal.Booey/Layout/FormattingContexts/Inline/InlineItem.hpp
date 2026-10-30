#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <unicode/ubidi.h>

namespace krys::boo::layout
{
  class InlineItemsBuilder;

  /// @brief One entry of the flat list an inline formatting context's content is turned into: a run of text,
  /// a line break, an atomic inline box, the start or end of an inline box, a float, a block or an opaque
  /// box.
  class InlineItem
  {
  public:
    enum class Type : uint8
    {
      Text,
      HardLineBreak,
      SoftLineBreak,
      WordBreakOpportunity,
      AtomicInlineBox,
      InlineBoxStart,
      InlineBoxEnd,
      Float,
      Block,
      Opaque
    };

  private:
    friend class InlineItemsBuilder;

    const Box *_layoutBox {nullptr};

  protected:
    InlineLayoutUnit _width {};
    uint32 _length {0};

    // For InlineTextItem and InlineSoftLineBreakItem
    uint32 _startOrPosition {0};

  private:
    UBiDiLevel _bidiLevel {UBIDI_DEFAULT_LTR};

    Type _type : 4 {};

  protected:
    // For InlineTextItem
    enum class TextItemType : uint8
    {
      Undefined,
      Whitespace,
      NonWhitespace
    };

    TextItemType _textItemType : 2 {TextItemType::Undefined};
    bool _hasWidth : 1 {false};
    bool _hasTrailingSoftHyphen : 1 {false};
    bool _isWordSeparator : 1 {false};

  public:
    InlineItem(const Box &layoutBox, Type type, UBiDiLevel bidiLevel = UBIDI_DEFAULT_LTR) noexcept
        : _layoutBox(&layoutBox), _bidiLevel(bidiLevel), _type(type)
    {
    }

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
    static constexpr UBiDiLevel OpaqueBidiLevel = 0xff;
    KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
    {
      return _bidiLevel;
    }
    KRYS_NODISCARD const Box &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return LayoutBox().Style();
    }
    KRYS_NODISCARD const RenderStyle &FirstLineStyle() const noexcept
    {
      return LayoutBox().FirstLineStyle();
    }

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return GetType() == Type::Text;
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return GetType() == Type::AtomicInlineBox;
    }
    KRYS_NODISCARD bool IsFloat() const noexcept
    {
      return GetType() == Type::Float;
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return IsSoftLineBreak() || IsHardLineBreak();
    }
    KRYS_NODISCARD bool IsWordBreakOpportunity() const noexcept
    {
      return GetType() == Type::WordBreakOpportunity;
    }
    KRYS_NODISCARD bool IsSoftLineBreak() const noexcept
    {
      return GetType() == Type::SoftLineBreak;
    }
    KRYS_NODISCARD bool IsHardLineBreak() const noexcept
    {
      return GetType() == Type::HardLineBreak;
    }
    KRYS_NODISCARD bool IsInlineBoxStart() const noexcept
    {
      return GetType() == Type::InlineBoxStart;
    }
    KRYS_NODISCARD bool IsInlineBoxEnd() const noexcept
    {
      return GetType() == Type::InlineBoxEnd;
    }
    KRYS_NODISCARD bool IsInlineBoxStartOrEnd() const noexcept
    {
      return IsInlineBoxStart() || IsInlineBoxEnd();
    }
    KRYS_NODISCARD bool IsOpaque() const noexcept
    {
      return GetType() == Type::Opaque;
    }
    KRYS_NODISCARD bool IsBlock() const noexcept
    {
      return GetType() == Type::Block;
    }

  private:
    void SetBidiLevel(UBiDiLevel bidiLevel) noexcept
    {
      _bidiLevel = bidiLevel;
    }
    void SetWidth(InlineLayoutUnit width) noexcept
    {
      _width = width;
      _hasWidth = true;
    }
  };

  using InlineItemList = List<InlineItem>;
}
