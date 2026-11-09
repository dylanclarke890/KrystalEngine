/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Graphics/SourceBrushLogicalGradient.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief What a fill or stroke paints with: a colour or a gradient.
  class SourceBrush
  {
  public:
    using OptionalPatternGradient = Variant<std::monostate, SourceBrushLogicalGradient, Ref<Pattern>>;

  private:
    boo::Color _color {boo::Color::black};
    OptionalPatternGradient _patternGradient;

  public:
    SourceBrush() noexcept = default;
    SourceBrush(const boo::Color &color) noexcept : _color(color)
    {
    }

    // Color should be stored in the variant, in place of std::monostate. Currently a lot of code
    // queries the color unconditionally, color is accessible unconditionally, returning incorrect
    // but defined data.
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }
    void SetColor(const boo::Color color) noexcept
    {
      _color = color;
    }

    // Packed color accessor takes into account the discrimination between color, gradient, pattern.
    KRYS_NODISCARD Maybe<PackedColour::RGBA> PackedColor() const noexcept;

    KRYS_NODISCARD boo::Gradient *GetGradient() const noexcept;
    KRYS_NODISCARD boo::Pattern *GetPattern() const noexcept;
    KRYS_NODISCARD const AffineTransform &GradientSpaceTransform() const noexcept;

    void SetGradient(Ref<boo::Gradient> &&, const AffineTransform &spaceTransform = {}) noexcept;
    void SetPattern(Ref<boo::Pattern> &&) noexcept;

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return HasPatternOrGradient() || _color.IsVisible();
    }
    KRYS_NODISCARD bool HasPatternOrGradient() const noexcept
    {
      return !std::holds_alternative<std::monostate>(_patternGradient);
    }

    KRYS_NODISCARD friend bool operator==(const SourceBrush &a, const SourceBrush &b) noexcept
    {
      // Patterns compare by identity.
      auto patternGradientEqual = [](const OptionalPatternGradient &a, const OptionalPatternGradient &b)
      {
        if (a.index() != b.index())
        {
          return false;
        }
        if (auto *aGradient = std::get_if<SourceBrushLogicalGradient>(&a))
        {
          return *aGradient == std::get<SourceBrushLogicalGradient>(b);
        }
        if (auto *aPattern = std::get_if<Ref<boo::Pattern>>(&a))
        {
          return aPattern->get() == std::get<Ref<boo::Pattern>>(b).get();
        }
        return true;
      };
      return a._color == b._color && patternGradientEqual(a._patternGradient, b._patternGradient);
    }
  };

  inline Maybe<PackedColour::RGBA> SourceBrush::PackedColor() const noexcept
  {
    if (HasPatternOrGradient())
    {
      return std::nullopt;
    }
    return _color.TryGetAsPackedInline();
  }

  TextStream &operator<<(TextStream &ts, const SourceBrush &brush) noexcept;
}
