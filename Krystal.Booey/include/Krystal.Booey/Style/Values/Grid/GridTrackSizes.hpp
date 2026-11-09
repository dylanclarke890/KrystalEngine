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

#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  using GridTrackSizeList = css::SpaceSeparatedFixedList<GridTrackSize>;

  // Default values for <'grid-auto-columns'>/<'grid-auto-rows'> is 'auto'.
  struct GridTrackSizeDefaulter
  {
    KRYS_NODISCARD const GridTrackSize &operator()() const noexcept;
    KRYS_NODISCARD bool operator==(const GridTrackSizeDefaulter &) const noexcept = default;
  };

  // <'grid-auto-columns'>/<'grid-auto-rows'> = <track-size>+
  // https://www.w3.org/TR/css-grid-2/#propdef-grid-auto-rows
  struct GridTrackSizes : css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter>
  {
    using css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter>::ListOrDefault;

    // Special constructor for use constructing initial 'auto' value.
    GridTrackSizes(css::keywords::Auto) noexcept
        : css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter> {DefaultValue}
    {
    }
  };
}

DEFINE_RANGE_LIKE_CONFORMANCE_FOR_LIST_OR_DEFAULT_DERIVED_TYPE(krys::boo::style::GridTrackSizes)
