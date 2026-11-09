/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief A system font keyword of the `font` shorthand.
  enum class SystemFontKeyword : uint8
  {
    // This needs to be kept in sync with the system font keywords and `css::LowerFontShorthand()`.
    Caption,
    Icon,
    Menu,
    MessageBox,
    SmallCaption,
    StatusBar,
  };

  /// @brief The font a system font keyword stands for.
  struct SystemFont
  {
    /// @brief The family, or null for the standard family.
    StringAtom Family {StringAtom::Null()};
    float Size {16};
    FontSelectionValue Weight {NormalWeightValue()};
  };

  /// @brief The fonts of the system font keywords (`caption`, `menu`, ...), which the host gives rather than
  /// the operating system: Booey's fonts are the host's (see DIFFERENCES.md). By default each is the standard
  /// family, at Chrome's size on Windows (12px for `menu`, `small-caption` and `status-bar`, 16px for the
  /// others), and normal weight.
  struct SystemFonts
  {
    Array<SystemFont, static_cast<size_t>(SystemFontKeyword::StatusBar) + 1> Fonts {{
      {},
      {},
      {StringAtom::Null(), 12},
      {},
      {StringAtom::Null(), 12},
      {StringAtom::Null(), 12},
    }};

    KRYS_NODISCARD const SystemFont &operator[](SystemFontKeyword keyword) const noexcept
    {
      return Fonts[static_cast<size_t>(keyword)];
    }

    KRYS_NODISCARD SystemFont &operator[](SystemFontKeyword keyword) noexcept
    {
      return Fonts[static_cast<size_t>(keyword)];
    }
  };
}
