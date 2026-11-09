/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Style/Values/Masking/MaskBorderOutset.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderRepeat.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderSlice.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderSource.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorderWidth.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'mask-border'> = <'mask-border-source'> || <'mask-border-slice'> [ / <'mask-border-width'>? [ /
  // <'mask-border-outset'> ]? ]? || <'mask-border-repeat'> || <'mask-border-mode'>
  // FIXME: Add support for `mask-border-mode`.
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border
  struct MaskBorder
  {
    MaskBorderSource Source;
    MaskBorderSlice Slice;
    MaskBorderWidth Width;
    MaskBorderOutset Outset;
    MaskBorderRepeat Repeat;

    MaskBorder() noexcept;
    MaskBorder(MaskBorderSource &&, MaskBorderSlice &&, MaskBorderWidth &&, MaskBorderOutset &&,
               MaskBorderRepeat &&) noexcept;

    KRYS_NODISCARD bool operator==(const MaskBorder &) const noexcept = default;
  };

  // MARK: - Conversion

  enum class MaskBorderSliceOverride : bool
  {
    None,
    AlwaysFill
  };

  template <>
  struct CSSValueConversion<MaskBorder>
  {
    KRYS_NODISCARD MaskBorder operator()(BuilderState &, const css::Value &,
                                         MaskBorderSliceOverride = MaskBorderSliceOverride::None) noexcept;
  };

  template <>
  struct CSSValueCreation<MaskBorder>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorder &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<MaskBorder>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const MaskBorder &) noexcept;
  };
}
