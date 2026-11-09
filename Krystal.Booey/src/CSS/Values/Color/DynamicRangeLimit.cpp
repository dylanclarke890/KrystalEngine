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

#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"

namespace krys::boo::css
{
  // NOTE: All non-template functions must be implemented in the implementation
  // file due to the use of the incomplete type `DynamicRangeLimitMixFunction`.

  DynamicRangeLimit::DynamicRangeLimit(keywords::Standard keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(keywords::Constrained keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(keywords::NoLimit keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(DynamicRangeLimitMixFunction &&mix) noexcept
      : _value {CreateUnique<DynamicRangeLimitMixFunction>(krys::move(mix))}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(DynamicRangeLimit &&) noexcept = default;
  DynamicRangeLimit &DynamicRangeLimit::operator=(DynamicRangeLimit &&) noexcept = default;
  DynamicRangeLimit::~DynamicRangeLimit() noexcept = default;

  bool DynamicRangeLimit::operator==(const DynamicRangeLimit &) const noexcept = default;
}
