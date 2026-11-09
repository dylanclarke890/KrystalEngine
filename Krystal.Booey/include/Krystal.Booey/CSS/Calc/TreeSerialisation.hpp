/*
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct SerialisationContext;
}

namespace krys::boo::css::calc
{
  struct Child;
  struct Tree;

  struct SerialisationOptions
  {
    // `Range` represents the allowed numeric range for the calculated result.
    css::Range Range;

    // `SerialisationContext` is the context used for CSS serialisation state.
    const css::SerialisationContext &SerialisationContext;
  };

  // https://drafts.csswg.org/css-values-4/#serialize-a-math-function
  void SerialisationForCSS(StringBuilder &builder, const Tree &tree,
                           const SerialisationOptions &options) noexcept;

  KRYS_NODISCARD CSSOMString SerialisationForCSS(const Tree &tree,
                                                 const SerialisationOptions &options) noexcept;

  void SerialisationForCSS(StringBuilder &builder, const Child &child,
                           const SerialisationOptions &options) noexcept;

  KRYS_NODISCARD CSSOMString SerialisationForCSS(const Child &child,
                                                 const SerialisationOptions &options) noexcept;
}