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

#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageOutset.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageRepeat.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSlice.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSource.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageWidth.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'border-image'> = <'border-image-source'> || <'border-image-slice'> [ / <'border-image-width'> | /
  // <'border-image-width'>? / <'border-image-outset'> ]? || <'border-image-repeat'>
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image
  struct BorderImage
  {
    BorderImageSource Source;
    BorderImageSlice Slice;
    BorderImageWidth Width;
    BorderImageOutset Outset;
    BorderImageRepeat Repeat;

    BorderImage() noexcept;
    BorderImage(BorderImageSource &&, BorderImageSlice &&, BorderImageWidth &&, BorderImageOutset &&,
                BorderImageRepeat &&) noexcept;

    KRYS_NODISCARD bool operator==(const BorderImage &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueCreation<BorderImage>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BorderImage &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BorderImage>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const BorderImage &) noexcept;
  };
}
