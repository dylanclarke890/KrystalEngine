/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  enum class BidiEmbeddingSource : uint8
  {
    FromStyleOrDOM,
    FromUnicode
  };

  /// @brief Used to keep track of explicit embeddings.
  class BidiContext : public ThreadSafeRefCounted<BidiContext>
  {
  private:
    // The maximium bidi level is 62: http://unicode.org/reports/tr9/#Explicit_Levels_and_Directions
    uint32 _level : 6;
    uint32 _direction : 5;
    uint32 _override : 1;
    uint32 _source : 1; // BidiEmbeddingSource
    RefPtr<BidiContext> _parent;

  public:
    KRYS_NODISCARD static Ref<BidiContext> Create(uint8 level, UCharDirection, bool override = false,
                                                  BidiEmbeddingSource = BidiEmbeddingSource::FromStyleOrDOM,
                                                  BidiContext *parent = nullptr) noexcept;

    KRYS_NODISCARD BidiContext *Parent() const noexcept
    {
      return _parent.get();
    }
    KRYS_NODISCARD uint8 Level() const noexcept
    {
      return static_cast<uint8>(_level);
    }
    KRYS_NODISCARD UCharDirection Dir() const noexcept
    {
      return static_cast<UCharDirection>(_direction);
    }
    KRYS_NODISCARD bool Override() const noexcept
    {
      return _override;
    }
    KRYS_NODISCARD BidiEmbeddingSource Source() const noexcept
    {
      return static_cast<BidiEmbeddingSource>(_source);
    }

    KRYS_NODISCARD Ref<BidiContext> CopyStackRemovingUnicodeEmbeddingContexts() noexcept;

  private:
    BidiContext(uint8 level, UCharDirection, bool override, BidiEmbeddingSource,
                BidiContext *parent) noexcept;

    KRYS_NODISCARD static Ref<BidiContext> CreateUncached(uint8 level, UCharDirection, bool override,
                                                          BidiEmbeddingSource, BidiContext *parent) noexcept;
  };

  KRYS_NODISCARD constexpr uint8 NextGreaterOddLevel(uint8 level) noexcept
  {
    return static_cast<uint8>((level + 1) | 1);
  }

  KRYS_NODISCARD constexpr uint8 NextGreaterEvenLevel(uint8 level) noexcept
  {
    return static_cast<uint8>((level + 2) & ~1);
  }

  KRYS_NODISCARD bool operator==(const BidiContext &, const BidiContext &) noexcept;
}
