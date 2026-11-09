/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief The used value of `font-palette`.
  struct FontPalette
  {
    enum class Type : uint8
    {
      Normal,
      Light,
      Dark,
      Custom
    };

    Type PaletteType {Type::Normal};
    StringAtom Identifier {StringAtom::Null()};

    KRYS_NODISCARD bool operator==(const FontPalette &other) const noexcept
    {
      if (PaletteType == Type::Custom)
      {
        return other.PaletteType == Type::Custom && Identifier == other.Identifier;
      }
      return PaletteType == other.PaletteType;
    }
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontPalette>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontPalette &palette) noexcept
    {
      if (palette.PaletteType == boo::FontPalette::Type::Custom)
      {
        return krys::Hash::Combine(static_cast<uint8>(palette.PaletteType),
                                   std::hash<StringAtom> {}(palette.Identifier));
      }
      return DefaultHash<uint8>::Hash(static_cast<uint8>(palette.PaletteType));
    }

    KRYS_NODISCARD static bool Equal(const boo::FontPalette &a, const boo::FontPalette &b) noexcept
    {
      return a == b;
    }
  };
}
