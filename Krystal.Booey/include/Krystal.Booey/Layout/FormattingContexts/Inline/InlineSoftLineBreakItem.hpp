#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::layout
{
  /// @brief A preserved segment break (newline) in an inline text box, at a byte offset into its content.
  class InlineSoftLineBreakItem : public InlineItem
  {
  public:
    KRYS_NODISCARD static InlineSoftLineBreakItem
      CreateSoftLineBreakItem(const InlineTextBox &inlineTextBox, uint32 position,
                              UBiDiLevel bidiLevel = UBIDI_DEFAULT_LTR) noexcept
    {
      return {inlineTextBox, position, bidiLevel};
    }

    KRYS_NODISCARD uint32 Position() const noexcept
    {
      return _startOrPosition;
    }
    KRYS_NODISCARD const InlineTextBox &GetInlineTextBox() const noexcept
    {
      return Downcast<InlineTextBox>(LayoutBox());
    }

  private:
    InlineSoftLineBreakItem(const InlineTextBox &inlineTextBox, uint32 position,
                            UBiDiLevel bidiLevel) noexcept
        : InlineItem(inlineTextBox, Type::SoftLineBreak, bidiLevel)
    {
      _startOrPosition = position;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::InlineSoftLineBreakItem)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::InlineItem &inlineItem) noexcept
  {
    return inlineItem.IsSoftLineBreak();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
