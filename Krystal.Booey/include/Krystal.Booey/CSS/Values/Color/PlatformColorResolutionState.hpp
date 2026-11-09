/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class LightDarkColorAppearance : bool;

  // Used to resolve a css::Color to a boo::Color (platform color) without going through style::Color / style
  // building.

  class PlatformColorResolutionDelegate
  {
  public:
    virtual ~PlatformColorResolutionDelegate() noexcept;

    // Colors to use that usually get resolved dynamically using Document & RenderStyle.

    /// @brief For `ValueId::Currentcolor`.
    KRYS_NODISCARD virtual boo::Color CurrentColor() const noexcept;

    /// @brief For `ValueId::InternalDocumentTextColor`.
    KRYS_NODISCARD virtual boo::Color InternalDocumentTextColor() const noexcept;

    /// @brief For `ValueId::WebkitLink`.
    KRYS_NODISCARD virtual boo::Color WebkitLink() const noexcept;

    /// @brief For `ValueId::WebkitActivelink`.
    KRYS_NODISCARD virtual boo::Color WebkitActiveLink() const noexcept;

    /// @brief For `ValueId::WebkitFocusRingColor`.
    KRYS_NODISCARD virtual boo::Color WebkitFocusRingColor() const noexcept;
  };

  struct PlatformColorResolutionState
  {
    // Delegate for lazily computing color values.
    PlatformColorResolutionDelegate *Delegate {nullptr};

    // Level of nesting inside other colors the resolution currently is.
    uint32 NestingLevel {0};

    // Conversion data needed to evaluate `calc()` expressions with relative length units.
    // If unset, colors that require conversion data will return the invalid Color.
    Maybe<ToLengthConversionData> ConversionData {};

    // Options to pass when resolving any other keyword with css::ColorFromKeyword()
    style::ColorOptions KeywordOptions {style::ColorOptions::None};

    // Appearance used to select from a light-dark() color function.
    // If unset, light-dark() colors will return the invalid Color.
    Maybe<LightDarkColorAppearance> Appearance {};

    // Colors are resolved:
    //   1. Checking if the color is set below, and if it is, returning it.
    //   2. If a delegate has been set, calling the associated delegate function,
    //      storing the result below, and returning that color.
    //   3. Returning the invalid `Color` value.
    mutable Maybe<boo::Color> ResolvedCurrentColor {};
    mutable Maybe<boo::Color> ResolvedInternalDocumentTextColor {};
    mutable Maybe<boo::Color> ResolvedWebkitLink {};
    mutable Maybe<boo::Color> ResolvedWebkitActiveLink {};
    mutable Maybe<boo::Color> ResolvedWebkitFocusRingColor {};

    KRYS_NODISCARD boo::Color CurrentColor() const noexcept
    {
      return ResolveColor(ResolvedCurrentColor, &PlatformColorResolutionDelegate::CurrentColor);
    }

    KRYS_NODISCARD boo::Color InternalDocumentTextColor() const noexcept
    {
      return ResolveColor(ResolvedInternalDocumentTextColor,
                          &PlatformColorResolutionDelegate::InternalDocumentTextColor);
    }

    KRYS_NODISCARD boo::Color WebkitLink() const noexcept
    {
      return ResolveColor(ResolvedWebkitLink, &PlatformColorResolutionDelegate::WebkitLink);
    }

    KRYS_NODISCARD boo::Color WebkitActiveLink() const noexcept
    {
      return ResolveColor(ResolvedWebkitActiveLink, &PlatformColorResolutionDelegate::WebkitActiveLink);
    }

    KRYS_NODISCARD boo::Color WebkitFocusRingColor() const noexcept
    {
      return ResolveColor(ResolvedWebkitFocusRingColor,
                          &PlatformColorResolutionDelegate::WebkitFocusRingColor);
    }

  private:
    KRYS_NODISCARD boo::Color ResolveColor(Maybe<boo::Color> &existing,
                                           boo::Color (PlatformColorResolutionDelegate::*resolver)()
                                             const noexcept) const noexcept
    {
      if (existing)
      {
        return *existing;
      }

      if (Delegate)
      {
        auto resolved = (Delegate->*resolver)();
        existing = resolved;
        return resolved;
      }

      return {};
    }
  };

  /// @brief RAII helper to increment/decrement nesting level.
  struct PlatformColorResolutionStateNester
  {
    KRYS_NON_COPY_MOVABLE(PlatformColorResolutionStateNester);

    PlatformColorResolutionState &State;

    explicit PlatformColorResolutionStateNester(PlatformColorResolutionState &state) noexcept : State {state}
    {
      State.NestingLevel++;
    }

    ~PlatformColorResolutionStateNester() noexcept
    {
      State.NestingLevel--;
    }
  };
}
