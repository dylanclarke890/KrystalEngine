/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::inline_display
{
  bool Box::IsHorizontal() const noexcept
  {
    return GetWritingMode().IsHorizontal();
  }

  bool Box::IsVisible() const noexcept
  {
    return !IsFullyTruncated() && Style().UsedVisibility() == Visibility::Visible;
  }

  bool Box::IsVisibleIgnoringUsedVisibility() const noexcept
  {
    return !IsFullyTruncated() && Style().GetVisibility() == Visibility::Visible;
  }

  boo::WritingMode Box::GetWritingMode() const noexcept
  {
    return Style().GetWritingMode();
  }

  bool operator==(const Box &a, const Box &b) noexcept
  {
    return &a.LayoutBox() == &b.LayoutBox() && a.Style() == b.Style() && a.LineIndex() == b.LineIndex()
           && a.BidiLevel() == b.BidiLevel()
           && a.VisualRectIgnoringBlockDirection() == b.VisualRectIgnoringBlockDirection()
           && a.InkOverflow() == b.InkOverflow() && a.IsFullyTruncated() == b.IsFullyTruncated()
           && (!a.IsTextOrSoftLineBreak() || (a.GetText() == b.GetText()));
  }
}
