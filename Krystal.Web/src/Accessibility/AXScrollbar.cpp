/*
 * Copyright (C) 2010, 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Accessibility/AXScrollbar.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollableArea.hpp"
#include "Krystal.Web/Platform/Scrolling/Scrollbar.hpp"

namespace krys::web
{
  AXScrollbar::AXScrollbar(AXId id, Scrollbar &scrollbar, AXObjectCache &cache) noexcept
      : AXMockObject(id, cache), _scrollbar(ShareRef(scrollbar))
  {
  }

  AXScrollbar::~AXScrollbar() noexcept = default;

  Ref<AXScrollbar> AXScrollbar::Create(AXId id, Scrollbar &scrollbar, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXScrollbar(id, scrollbar, cache));
  }

  Document *AXScrollbar::GetDocument() const noexcept
  {
    auto *parent = ParentObject();
    return parent ? parent->GetDocument() : nullptr;
  }

  LayoutRect AXScrollbar::ElementRect() const noexcept
  {
    return LayoutRect(_scrollbar->FrameRect());
  }

  Maybe<AXOrientation> AXScrollbar::ExplicitOrientation() const noexcept
  {
    return _scrollbar->Orientation() == ScrollbarOrientation::Horizontal ? AXOrientation::Horizontal
                                                                         : AXOrientation::Vertical;
  }

  bool AXScrollbar::IsEnabled() const noexcept
  {
    return _scrollbar->Enabled();
  }

  float AXScrollbar::ValueForRange() const noexcept
  {
    auto maximum = _scrollbar->Maximum();
    return maximum ? _scrollbar->CurrentPos() / static_cast<float>(maximum) : 0;
  }

  bool AXScrollbar::SetValue(float value) noexcept
  {
    auto newValue = value * static_cast<float>(_scrollbar->Maximum());
    _scrollbar->GetScrollableArea().ScrollToOffsetWithoutAnimation(_scrollbar->Orientation(), newValue);
    return true;
  }
}
