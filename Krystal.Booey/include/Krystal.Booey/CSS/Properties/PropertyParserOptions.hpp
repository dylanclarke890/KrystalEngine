/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class AllowAnchor : bool
  {
    No,
    Yes,
  };

  enum class AllowAnchorSize : bool
  {
    No,
    Yes,
  };

  enum class AllowUnitlessZero : bool
  {
    No,
    Yes,
  };

  struct PropertyParserOptions
  {
    AllowAnchor AnchorPolicy {AllowAnchor::No};

    AllowAnchorSize AnchorSizePolicy {AllowAnchorSize::No};

    /// @brief Generally, unitless zero is forbidden for <angle> values, but in a few legacy cases, it is
    /// it can be allowed.
    /// @see https://drafts.csswg.org/css-values-4/#angles
    AllowUnitlessZero UnitlessZeroAngle {AllowUnitlessZero::No};

    /// @brief Generally, unitless zero is allowed for <length> values, but in a few cases, when it is
    /// ambiguous with a <number> production, it can be forbidden.
    /// @see https://drafts.csswg.org/css-values-4/#lengths
    AllowUnitlessZero UnitlessZeroLength {AllowUnitlessZero::Yes};

    Maybe<ParserMode> OverrideParserMode {null};
  };
}