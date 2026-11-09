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

#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"

namespace krys::boo::layout
{
  Rect BoxGeometry::MarginBox() const noexcept
  {
    auto borderBox = BorderBox();

    Rect marginBox;
    marginBox.SetTop(borderBox.Top() - MarginBefore());
    marginBox.SetLeft(borderBox.Left() - MarginStart());
    marginBox.SetHeight(borderBox.Height() + MarginBefore() + MarginAfter());
    marginBox.SetWidth(borderBox.Width() + MarginStart() + MarginEnd());
    return marginBox;
  }

  Rect BoxGeometry::BorderBox() const noexcept
  {
    Rect borderBox;
    borderBox.SetTopLeft({});
    borderBox.SetSize({BorderBoxWidth(), BorderBoxHeight()});
    return borderBox;
  }

  Rect BoxGeometry::PaddingBox() const noexcept
  {
    auto borderBox = BorderBox();

    Rect paddingBox;
    paddingBox.SetTop(borderBox.Top() + BorderBefore());
    paddingBox.SetLeft(borderBox.Left() + BorderStart());
    paddingBox.SetHeight(borderBox.Bottom() - VerticalSpaceForScrollbar() - BorderAfter() - BorderBefore());
    paddingBox.SetWidth(borderBox.Width() - BorderEnd() - HorizontalSpaceForScrollbar() - BorderStart());
    return paddingBox;
  }

  Rect BoxGeometry::ContentBox() const noexcept
  {
    Rect contentBox;
    contentBox.SetTop(ContentBoxTop());
    contentBox.SetLeft(ContentBoxLeft());
    contentBox.SetWidth(ContentBoxWidth());
    contentBox.SetHeight(ContentBoxHeight());
    return contentBox;
  }

  void BoxGeometry::Reset() noexcept
  {
    SetTopLeft({});
    SetHorizontalMargin({});
    SetVerticalMargin({});
    SetBorder({});
    SetPadding({});
    SetContentBoxSize({});
    SetVerticalSpaceForScrollbar({});
    SetHorizontalSpaceForScrollbar({});
  }
}
