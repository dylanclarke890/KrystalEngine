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

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo::style
{
  // Utility type that wraps a StyleImage for use with the strong style type system.
  struct ImageWrapper
  {
    Ref<StyleImage> Value;

    KRYS_NODISCARD bool operator==(const ImageWrapper &other) const noexcept
    {
      return ArePointingToEqualData(Value, other.Value);
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueCreation<ImageWrapper>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ImageWrapper &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<ImageWrapper>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const ImageWrapper &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ImageWrapper>
  {
    KRYS_NODISCARD ImageWrapper Blend(const ImageWrapper &, const ImageWrapper &,
                                      const BlendingContext &) noexcept;
  };
}
