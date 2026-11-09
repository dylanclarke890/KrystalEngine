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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  /// @brief A font palette: `light`, `dark` or a palette index.
  struct FontPaletteIndex
  {
    enum class Type : uint8
    {
      Light,
      Dark,
      Integer
    };

    Type IndexType {Type::Integer};
    uint32 Integer {0};

    FontPaletteIndex() noexcept = default;

    FontPaletteIndex(Type type) noexcept : IndexType(type)
    {
      krys_debug_assert(type == Type::Light || type == Type::Dark);
    }

    FontPaletteIndex(uint32 integer) noexcept : IndexType(Type::Integer), Integer(integer)
    {
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return IndexType != Type::Integer || Integer;
    }

    KRYS_NODISCARD bool operator==(const FontPaletteIndex &other) const noexcept
    {
      if (IndexType != other.IndexType)
      {
        return false;
      }
      if (IndexType == Type::Integer)
      {
        return Integer == other.Integer;
      }
      return true;
    }
  };

  /// @brief The palette described by `@font-palette-values`: a base palette and colour overrides.
  class FontPaletteValues
  {
  public:
    using OverriddenColor = Pair<uint32, Color>;

  private:
    Maybe<FontPaletteIndex> _basePalette;
    List<OverriddenColor> _overrideColors;

  public:
    FontPaletteValues() noexcept = default;

    FontPaletteValues(Maybe<FontPaletteIndex> basePalette, List<OverriddenColor> &&overrideColors) noexcept
        : _basePalette(basePalette), _overrideColors(krys::move(overrideColors))
    {
    }

    KRYS_NODISCARD Maybe<FontPaletteIndex> BasePalette() const noexcept
    {
      return _basePalette;
    }

    KRYS_NODISCARD const List<OverriddenColor> &OverrideColors() const noexcept
    {
      return _overrideColors;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _basePalette || !_overrideColors.empty();
    }

    KRYS_NODISCARD bool operator==(const FontPaletteValues &other) const noexcept = default;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontPaletteIndex>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const boo::FontPaletteIndex &index) noexcept
    {
      if (index.IndexType == boo::FontPaletteIndex::Type::Integer)
      {
        return krys::Hash::Combine(index.IndexType, index.Integer);
      }
      return krys::Hash::Combine(index.IndexType);
    }

    KRYS_NODISCARD static bool Equal(const boo::FontPaletteIndex &a, const boo::FontPaletteIndex &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct DefaultHash<boo::FontPaletteValues>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const boo::FontPaletteValues &values) noexcept
    {
      auto basePalette = values.BasePalette();
      size_t hash = basePalette ? krys::Hash::Combine(true, *basePalette) : krys::Hash::Combine(false);
      for (const auto &[index, color] : values.OverrideColors())
      {
        hash = krys::Hash::Combine(hash, index, color);
      }
      return hash;
    }

    KRYS_NODISCARD static bool Equal(const boo::FontPaletteValues &a,
                                     const boo::FontPaletteValues &b) noexcept
    {
      return a == b;
    }
  };
}
