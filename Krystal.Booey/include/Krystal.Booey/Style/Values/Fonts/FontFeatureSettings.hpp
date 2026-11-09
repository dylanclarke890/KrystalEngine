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
  // <feature-tag-value> = <opentype-tag> [ <integer [0,∞]> | on | off ]?
  // <'font-feature-settings'> = normal | <feature-tag-value>#
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-feature-settings
  struct FontFeatureSettings
  {
    using Value = Integer<css::NonNegative>;

    FontFeatureSettings(css::keywords::Normal) noexcept : _platform {}
    {
    }
    FontFeatureSettings(boo::FontFeatureSettings &&platform) noexcept : _platform {krys::move(platform)}
    {
    }
    FontFeatureSettings(const boo::FontFeatureSettings &platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD const boo::FontFeatureSettings &Platform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD boo::FontFeatureSettings TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool operator==(const FontFeatureSettings &) const noexcept = default;

  private:
    boo::FontFeatureSettings _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontFeatureSettings>
  {
    KRYS_NODISCARD FontFeatureSettings operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontFeatureSettings>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontFeatureSettings &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontFeatureSettings>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const FontFeatureSettings &) noexcept;
  };
}
