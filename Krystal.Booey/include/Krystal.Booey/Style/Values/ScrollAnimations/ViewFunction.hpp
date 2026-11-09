/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/Timelines/ScrollAxis.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    class ViewValue;
  }

  namespace style
  {
    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://www.w3.org/TR/scroll-animations-1/#funcdef-view
    struct ViewFunctionParameters
    {
      ScrollAxis Axis;
      ViewTimelineInsetItem Insets;

      KRYS_NODISCARD bool operator==(const ViewFunctionParameters &) const noexcept = default;
    };
    using ViewFunction = css::FunctionNotation<css::ValueId::View, ViewFunctionParameters>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<ViewFunction>
    {
      KRYS_NODISCARD ViewFunction operator()(BuilderState &, const css::Value &) noexcept;
      KRYS_NODISCARD ViewFunction operator()(BuilderState &, const css::ViewValue &) noexcept;
    };

    template <>
    struct CSSValueCreation<ViewFunction>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const ViewFunction &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<ViewFunctionParameters>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const ViewFunctionParameters &) noexcept;
    };

  }
}
