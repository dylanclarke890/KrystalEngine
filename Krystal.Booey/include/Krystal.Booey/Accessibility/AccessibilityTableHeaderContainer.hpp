/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityMockObject.hpp"

namespace krys::boo::ax
{
  /// @brief An exposed table's column headers, together.
  /// @note It's ignored, so it's never in the tree, as on WebKit's ports other than Apple's: the table
  /// exposes it through `TableHeaderContainer()`.
  class AccessibilityTableHeaderContainer final : public AccessibilityMockObject
  {
  private:
    LayoutRect _headerRect;

  public:
    KRYS_NODISCARD static Ref<AccessibilityTableHeaderContainer> Create(AXID id,
                                                                        AXObjectCache &cache) noexcept;
    ~AccessibilityTableHeaderContainer() noexcept override;

    void AddChildren() noexcept override;
    /// @brief The headers' bounds, worked out when the children are added.
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override
    {
      return _headerRect;
    }

  private:
    AccessibilityTableHeaderContainer(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::TableHeaderContainer;
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityTableHeaderContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMockObject() && object.Role() == krys::boo::ax::AccessibilityRole::TableHeaderContainer;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
