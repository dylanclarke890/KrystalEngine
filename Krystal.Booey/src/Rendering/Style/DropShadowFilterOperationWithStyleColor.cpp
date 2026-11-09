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

#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"

namespace krys::boo::style
{
  Ref<DropShadowFilterOperation> DropShadowFilterOperationWithStyleColor::CreateEquivalentWithResolvedColor(
    const RenderStyle &style) noexcept
  {
    auto resolvedColor = _styleColor.ResolveColor(style.GetColor());
    return DropShadowFilterOperation::Create(Location(), StdDeviation(), resolvedColor);
  }

  bool DropShadowFilterOperationWithStyleColor::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }
    const auto &other = Downcast<DropShadowFilterOperationWithStyleColor>(operation);
    return NonColorEqual(other) && _styleColor == other._styleColor;
  }

  RefPtr<FilterOperation> DropShadowFilterOperationWithStyleColor::Blend(const FilterOperation *from,
                                                                         const BlendingContext &context,
                                                                         bool blendToPassthrough) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    auto toColor = _styleColor.ResolveColor(context.ToCurrentColor);

    if (blendToPassthrough)
    {
      return DropShadowFilterOperationWithStyleColor::Create(
        boo::Blend(_location, IntPoint(), context), boo::Blend(_stdDeviation, 0, context),
        boo::Blend(toColor, boo::Color::transparentBlack, context));
    }

    const auto *fromOperation = Downcast<DropShadowFilterOperationWithStyleColor>(from);
    IntPoint fromLocation = fromOperation ? fromOperation->Location() : IntPoint();
    int fromStdDeviation = fromOperation ? fromOperation->StdDeviation() : 0;

    auto fromColor = fromOperation ? fromOperation->StyleColor().ResolveColor(context.FromCurrentColor)
                                   : boo::Color::transparentBlack;

    return DropShadowFilterOperationWithStyleColor::Create(
      boo::Blend(fromLocation, _location, context),
      std::max(boo::Blend(fromStdDeviation, _stdDeviation, context), 0),
      boo::Blend(fromColor, toColor, context));
  }
}
