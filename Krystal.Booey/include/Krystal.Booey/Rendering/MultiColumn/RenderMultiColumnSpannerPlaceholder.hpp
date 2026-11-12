/*
 * Copyright (c) 2014, Opera Software ASA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Opera Software ASA nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderMultiColumnFlow;

  /// @brief Marks where a spanner (a `column-span: all` box, which is moved out of the multi-column flow to
  /// sit among the column sets) would be in the flow: it separates the content before and after it, and
  /// ends one column set and starts the next.
  class RenderMultiColumnSpannerPlaceholder final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderMultiColumnSpannerPlaceholder);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMultiColumnSpannerPlaceholder);

  private:
    WeakPtr<RenderBox> _spanner;
    WeakPtr<RenderMultiColumnFlow> _fragmentedFlow;

  public:
    /// @brief Use CreateAnonymous.
    RenderMultiColumnSpannerPlaceholder(RenderMultiColumnFlow &fragmentedFlow, RenderBox &spanner,
                                        RenderStyle &&style) noexcept;
    ~RenderMultiColumnSpannerPlaceholder() noexcept override;

    KRYS_NODISCARD static RenderPtr<RenderMultiColumnSpannerPlaceholder>
      CreateAnonymous(RenderMultiColumnFlow &fragmentedFlow, RenderBox &spanner,
                      const RenderStyle &parentStyle) noexcept;

    KRYS_NODISCARD RenderBox *Spanner() const noexcept
    {
      return _spanner.get();
    }
    KRYS_NODISCARD RenderMultiColumnFlow *FragmentedFlow() const noexcept;

  private:
    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
      krys_debug_assert_msg(false, "A spanner placeholder has no intrinsic widths.");
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override
    {
    }
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMultiColumnSpannerPlaceholder";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMultiColumnSpannerPlaceholder)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMultiColumnSpannerPlaceholder();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
