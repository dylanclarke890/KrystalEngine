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

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  namespace css
  {
    class PrimitiveValue;
    class ValuePair;
  }

  namespace style
  {
    // <single-view-timeline-inset-item> = [ [ auto | <length-percentage> ]{1,2} ]
    struct ViewTimelineInsetItem
    {
      struct Length : LengthWrapperBase<LengthPercentage<>, css::keywords::Auto>
      {
        using Base::Base;

        KRYS_NODISCARD bool IsAuto() const noexcept
        {
          return HoldsAlternative<css::keywords::Auto>();
        }
      };

      css::MinimallySerialisingSpaceSeparatedPair<Length> Value;

      ViewTimelineInsetItem(css::keywords::Auto keyword) noexcept : Value {Length {keyword}, Length {keyword}}
      {
      }

      ViewTimelineInsetItem(Length &&length) noexcept : Value {length, length}
      {
      }

      ViewTimelineInsetItem(Length &&first, Length &&second) noexcept
          : Value {krys::move(first), krys::move(second)}
      {
      }

      KRYS_NODISCARD const Length &Start() const noexcept
      {
        return Value.first();
      }
      KRYS_NODISCARD const Length &End() const noexcept
      {
        return Value.second();
      }

      KRYS_NODISCARD bool operator==(const ViewTimelineInsetItem &) const noexcept = default;
    };
    DEFINE_TYPE_WRAPPER_GET(ViewTimelineInsetItem, Value);

    // <view-timeline-inset-list> = <single-view-timeline-inset-item>#
    using ViewTimelineInsetList = css::CommaSeparatedFixedList<ViewTimelineInsetItem>;

    // Default value for <'view-timeline-inset'> is 'auto'.
    struct ViewTimelineInsetDefaulter
    {
      KRYS_NODISCARD const ViewTimelineInsetItem &operator()() const noexcept;
      KRYS_NODISCARD bool operator==(const ViewTimelineInsetDefaulter &) const noexcept = default;
    };

    // <'view-timeline-inset'> = <view-timeline-inset-list>
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset
    struct ViewTimelineInsets : css::ListOrDefault<ViewTimelineInsetList, ViewTimelineInsetDefaulter>
    {
      using css::ListOrDefault<ViewTimelineInsetList, ViewTimelineInsetDefaulter>::ListOrDefault;

      // Special constructor for use constructing initial 'auto' value.
      ViewTimelineInsets(css::keywords::Auto) noexcept
          : css::ListOrDefault<ViewTimelineInsetList, ViewTimelineInsetDefaulter> {DefaultValue}
      {
      }
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<ViewTimelineInsetItem>
    {
      KRYS_NODISCARD ViewTimelineInsetItem operator()(BuilderState &, const css::Value &) noexcept;
      KRYS_NODISCARD ViewTimelineInsetItem operator()(BuilderState &, const css::PrimitiveValue &) noexcept;
      KRYS_NODISCARD ViewTimelineInsetItem operator()(BuilderState &, const css::ValuePair &) noexcept;
    };
    template <>
    struct CSSValueConversion<ViewTimelineInsets>
    {
      KRYS_NODISCARD ViewTimelineInsets operator()(BuilderState &, const css::Value &) noexcept;
    };

  }
}

DEFINE_RANGE_LIKE_CONFORMANCE_FOR_LIST_OR_DEFAULT_DERIVED_TYPE(krys::boo::style::ViewTimelineInsets)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ViewTimelineInsetItem::Length)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::ViewTimelineInsetItem)
