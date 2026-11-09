/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2012-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ParserMode : uint8
  {
    HTMLStandard,
    HTMLQuirks,
    SVGAttribute,
    UASheet,
  };

  KRYS_NODISCARD constexpr ParserMode StrictToParserMode(bool inStrictMode) noexcept
  {
    return inStrictMode ? ParserMode::HTMLStandard : ParserMode::HTMLQuirks;
  }

  KRYS_NODISCARD constexpr bool IsQuirksModeBehavior(ParserMode mode) noexcept
  {
    return mode == ParserMode::HTMLQuirks;
  }

  KRYS_NODISCARD constexpr bool IsUASheetBehavior(ParserMode mode) noexcept
  {
    return mode == ParserMode::UASheet;
  }

  KRYS_NODISCARD constexpr bool IsUnitlessValueParsingForcedForMode(ParserMode mode) noexcept
  {
    return mode == ParserMode::SVGAttribute;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ParserMode, 4uz);