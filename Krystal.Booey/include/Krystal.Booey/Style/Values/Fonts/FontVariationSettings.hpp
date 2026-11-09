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

#include "Krystal.Booey/Graphics/Fonts/FontTaggedSettings.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'font-variation-settings'> = normal | [ <opentype-tag> <number> ]#
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variation-settings
  struct FontVariationSettings
  {
    using Value = style::Number<css::All, float>;

    FontVariationSettings(css::keywords::Normal) noexcept : _platform {}
    {
    }
    FontVariationSettings(boo::FontVariationSettings &&platform) noexcept : _platform {krys::move(platform)}
    {
    }
    FontVariationSettings(const boo::FontVariationSettings &platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD const boo::FontVariationSettings &Platform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD boo::FontVariationSettings TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool operator==(const FontVariationSettings &) const noexcept = default;

  private:
    boo::FontVariationSettings _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariationSettings>
  {
    KRYS_NODISCARD FontVariationSettings operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontVariationSettings>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontVariationSettings &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontVariationSettings>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const FontVariationSettings &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontVariationSettings>
  {
    KRYS_NODISCARD bool CanBlend(const FontVariationSettings &, const FontVariationSettings &) noexcept;
    KRYS_NODISCARD FontVariationSettings Blend(const FontVariationSettings &, const FontVariationSettings &,
                                               const BlendingContext &) noexcept;
  };
}
