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

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'font-variant-alternates'> = normal | [ stylistic(<feature-value-name>) || historical-forms ||
  // styleset(<feature-value-name>#) || character-variant(<feature-value-name>#) ||
  // swash(<feature-value-name>) || ornaments(<feature-value-name>) || annotation(<feature-value-name>) ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variant-alternates
  struct FontVariantAlternates
  {
    using Platform = boo::FontVariantAlternates;

    FontVariantAlternates(css::keywords::Normal) noexcept : _platform {Platform::Normal()}
    {
    }

    FontVariantAlternates(css::keywords::HistoricalForms) noexcept : _platform {Platform::Normal()}
    {
      _platform.ValuesRef().HistoricalForms = true;
    }

    FontVariantAlternates(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD const Platform &GetPlatform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD Platform TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return _platform.IsNormal();
    }

    KRYS_NODISCARD bool operator==(const FontVariantAlternates &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantAlternates>
  {
    KRYS_NODISCARD FontVariantAlternates operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontVariantAlternates>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontVariantAlternates &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontVariantAlternates>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const FontVariantAlternates &) noexcept;
  };
}
