/*
 * Copyright (C) 2010-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Accessibility/AXMockObject.hpp"

namespace krys::boo
{
  class Scrollbar;
  /// @brief One of a view's scroll bars, its value the scroll position as a fraction of the most it can
  /// scroll.
  class AXScrollbar final : public AXMockObject
  {
  private:
    Ref<Scrollbar> _scrollbar;

  public:
    KRYS_NODISCARD static Ref<AXScrollbar> Create(AXId id, Scrollbar &scrollbar,
                                                  AXObjectCache &cache) noexcept;
    ~AXScrollbar() noexcept override;

    KRYS_NODISCARD Scrollbar &GetScrollbar() const noexcept
    {
      return *_scrollbar;
    }

    KRYS_NODISCARD dom::Document *GetDocument() const noexcept override;
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;
    KRYS_NODISCARD Maybe<AXOrientation> ExplicitOrientation() const noexcept override;
    KRYS_NODISCARD bool IsEnabled() const noexcept override;
    KRYS_NODISCARD float ValueForRange() const noexcept override;
    KRYS_NODISCARD float MaxValueForRange() const noexcept override
    {
      return 1;
    }
    /// @brief Scrolls to @p value, a fraction of the most the view can scroll.
    KRYS_NODISCARD bool CanSetValueAttribute() const noexcept override
    {
      return true;
    }
    using AXMockObject::SetValue;
    bool SetValue(float value) noexcept override;

  private:
    AXScrollbar(AXId id, Scrollbar &scrollbar, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AXRole DetermineAccessibilityRole() noexcept override
    {
      return AXRole::ScrollBar;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::AXScrollbar)
  KRYS_NODISCARD static bool IsType(const krys::boo::AXObject &object) noexcept
  {
    return object.IsMockObject() && object.Role() == krys::boo::AXRole::ScrollBar;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
