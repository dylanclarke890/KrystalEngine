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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/CompactVariant.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  struct DynamicRangeLimitMixFunction;

  // <'dynamic-range-limit'> = standard | no-limit | constrained | <dynamic-range-limit-mix()>
  // https://drafts.csswg.org/css-color-hdr/#propdef-dynamic-range-limit
  struct DynamicRangeLimit
  {
  private:
    // The mix function is never null, except after a move.
    using Kind = CompactVariant<keywords::Standard, keywords::Constrained, keywords::NoLimit,
                                UniquePtr<DynamicRangeLimitMixFunction>>;

    Kind _value;

  public:
    DynamicRangeLimit(keywords::Standard) noexcept;
    DynamicRangeLimit(keywords::Constrained) noexcept;
    DynamicRangeLimit(keywords::NoLimit) noexcept;
    DynamicRangeLimit(DynamicRangeLimitMixFunction &&) noexcept;

    DynamicRangeLimit(DynamicRangeLimit &&) noexcept;
    DynamicRangeLimit &operator=(DynamicRangeLimit &&) noexcept;

    // Delete to avoid unnecessary unnecessary allocations on accidental copy.
    DynamicRangeLimit(const DynamicRangeLimit &) = delete;
    DynamicRangeLimit &operator=(const DynamicRangeLimit &) = delete;

    ~DynamicRangeLimit() noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const;

    KRYS_NODISCARD bool operator==(const DynamicRangeLimit &) const noexcept;
  };

  template <typename... F>
  decltype(auto) DynamicRangeLimit::SwitchOn(F &&...f) const
  {
    auto visitor = CreateVisitor(std::forward<F>(f)...);
    using ResultType = decltype(visitor(std::declval<keywords::Standard>()));

    return krys::SwitchOn(
      _value,
      [&]<ValueId Id>(const KeywordValueConstant<Id> &keyword) -> ResultType { return visitor(keyword); },
      [&](const UniquePtr<DynamicRangeLimitMixFunction> &mix) -> ResultType { return visitor(*mix); });
  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::DynamicRangeLimit)
