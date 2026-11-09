/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontCascadeFonts.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FontCascadeDescription;
  class FontSelector;

  /// @brief The parts of a font description other than the family that matter when fetching fonts.
  struct FontDescriptionKey
  {
    FontDescription Description;

    FontDescriptionKey() noexcept = default;

    explicit FontDescriptionKey(const FontDescription &description) noexcept : Description(description)
    {
    }

    KRYS_NODISCARD bool operator==(const FontDescriptionKey &) const noexcept = default;
  };

  struct FontDescriptionKeyHash
  {
    KRYS_NODISCARD size_t operator()(const FontDescriptionKey &key) const noexcept;
  };

  /// @brief The name of a font family, hashed and compared with the rules for font family names (ASCII
  /// case-insensitively) instead of as a string.
  class FontFamilyName
  {
  private:
    StringAtom _name;

  public:
    FontFamilyName() noexcept;
    FontFamilyName(const StringAtom &name) noexcept;

    KRYS_NODISCARD const StringAtom &String() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD bool operator==(const FontFamilyName &other) const noexcept;
    KRYS_NODISCARD size_t Hash() const noexcept;
  };

  struct FontCascadeCacheKey
  {
    // Shared with the lower level FontCache (caching Font objects)
    FontDescriptionKey DescriptionKey;
    SmallList<FontFamilyName, 3> Families;
    uint32 FontSelectorId {0};
    uint32 FontSelectorVersion {0};
    bool HasComplexFontSelector {true};

    KRYS_NODISCARD bool operator==(const FontCascadeCacheKey &) const noexcept = default;
  };

  struct FontCascadeCacheKeyHash
  {
    KRYS_NODISCARD size_t operator()(const FontCascadeCacheKey &key) const noexcept;
  };

  /// @brief The fonts of every font cascade description in use, shared by the cascades with equal
  /// descriptions.
  class FontCascadeCache
  {
    KRYS_NON_COPY_MOVABLE(FontCascadeCache);

  private:
    HashMap<FontCascadeCacheKey, Ref<FontCascadeFonts>, FontCascadeCacheKeyHash> _entries;
    uint32 _pruneCounter {0};

  public:
    FontCascadeCache() noexcept = default;

    KRYS_NODISCARD static FontCascadeCache &Singleton() noexcept;

    void Invalidate() noexcept;
    void ClearWidthCaches() noexcept;
    void PruneUnreferencedEntries() noexcept;
    void PruneSystemFallbackFonts() noexcept;

    KRYS_NODISCARD Ref<FontCascadeFonts> RetrieveOrAddCachedFonts(const FontCascadeDescription &description,
                                                                  FontSelector *fontSelector) noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _entries.size();
    }
  };
}
