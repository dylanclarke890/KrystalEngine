/*
 * Copyright (C) 2018-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::layout
{
  /// @brief The root of the layout tree: the rectangle the root element's box lives in.
  class InitialContainingBlock final : public ElementBox
  {
    KRYS_NON_COPY_MOVABLE(InitialContainingBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(InitialContainingBlock);

  public:
    explicit InitialContainingBlock(RenderStyle &&style,
                                    UniquePtr<RenderStyle> &&firstLineStyle = nullptr) noexcept;
    ~InitialContainingBlock() noexcept override = default;

  private:
    const ElementBox &Parent() const = delete;
    const Box *NextSibling() const = delete;
    const Box *NextInFlowSibling() const = delete;
    const Box *NextInFlowOrFloatingSibling() const = delete;
    const Box *PreviousSibling() const = delete;
    const Box *PreviousInFlowSibling() const = delete;
    const Box *PreviousInFlowOrFloatingSibling() const = delete;
    Box *NextSibling() = delete;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::InitialContainingBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::Box &box) noexcept
  {
    return box.IsInitialContainingBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
