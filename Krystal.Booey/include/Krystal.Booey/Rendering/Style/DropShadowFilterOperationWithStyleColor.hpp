/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo::style
{
  class DropShadowFilterOperationWithStyleColor : public DropShadowFilterOperationBase
  {
  public:
    KRYS_NODISCARD static Ref<DropShadowFilterOperationWithStyleColor>
      Create(const IntPoint &location, int stdDeviation, const style::Color &color) noexcept
    {
      return AdoptRef(*new DropShadowFilterOperationWithStyleColor(location, stdDeviation, color));
    }

    KRYS_NODISCARD Ref<DropShadowFilterOperation>
      CreateEquivalentWithResolvedColor(const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new DropShadowFilterOperationWithStyleColor(Location(), StdDeviation(), _styleColor));
    }

    KRYS_NODISCARD const style::Color &StyleColor() const noexcept
    {
      return _styleColor;
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;
    KRYS_NODISCARD bool RequiresRepaintForCurrentColorChange() const noexcept override
    {
      return _styleColor.ContainsCurrentColor();
    }

    DropShadowFilterOperationWithStyleColor(const IntPoint &location, int stdDeviation,
                                            const style::Color &color) noexcept
        : DropShadowFilterOperationBase(Type::DropShadowWithStyleColor, location, stdDeviation),
          _styleColor(color)
    {
    }

    style::Color _styleColor;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::style::DropShadowFilterOperationWithStyleColor,
                                             GetType()
                                               == krys::boo::FilterOperation::Type::DropShadowWithStyleColor)
