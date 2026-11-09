/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
