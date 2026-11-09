/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontCascadeCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSelector.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys::boo
{
  size_t FontDescriptionKeyHash::operator()(const FontDescriptionKey &key) const noexcept
  {
    auto &description = key.Description;
    return Hash::Combine(description.ComputedSize(), static_cast<float>(description.Weight()),
                         static_cast<int32>(description.Script()),
                         static_cast<uint8>(description.Orientation()),
                         static_cast<uint8>(description.VariantCaps()));
  }

  FontFamilyName::FontFamilyName() noexcept : _name(StringAtom::Null())
  {
  }

  FontFamilyName::FontFamilyName(const StringAtom &name) noexcept : _name {name}
  {
  }

  bool FontFamilyName::operator==(const FontFamilyName &other) const noexcept
  {
    return (_name == StringAtom::Null() || other._name == StringAtom::Null())
             ? _name == other._name
             : FontCascadeDescription::FamilyNamesAreEqual(_name, other._name);
  }

  size_t FontFamilyName::Hash() const noexcept
  {
    // FIXME: Would be better to hash the characters in the name instead of hashing a hash.
    if (_name == StringAtom::Null())
    {
      return 0;
    }
    return FontCascadeDescription::FamilyNameHash(_name);
  }

  size_t FontCascadeCacheKeyHash::operator()(const FontCascadeCacheKey &key) const noexcept
  {
    size_t hash = FontDescriptionKeyHash {}(key.DescriptionKey);
    for (auto &family : key.Families)
    {
      hash = Hash::Combine(hash, family.Hash());
    }
    return Hash::Combine(hash, key.FontSelectorId, key.FontSelectorVersion);
  }

  FontCascadeCache &FontCascadeCache::Singleton() noexcept
  {
    return FontCache::Singleton().GetFontCascadeCache();
  }

  void FontCascadeCache::Invalidate() noexcept
  {
    _entries.clear();
  }

  void FontCascadeCache::ClearWidthCaches() noexcept
  {
    for (auto &[key, fonts] : _entries)
    {
      fonts->GetWidthCache().Clear();
    }
  }

  void FontCascadeCache::PruneUnreferencedEntries() noexcept
  {
    std::erase_if(_entries, [](auto &entry) { return entry.second->HasOneRef(); });
  }

  void FontCascadeCache::PruneSystemFallbackFonts() noexcept
  {
    for (auto &[key, fonts] : _entries)
    {
      fonts->PruneSystemFallbacks();
    }
  }

  KRYS_NODISCARD static FontCascadeCacheKey MakeFontCascadeCacheKey(const FontCascadeDescription &description,
                                                                    FontSelector *fontSelector) noexcept
  {
    uint32 familyCount = description.FamilyCount();
    auto hasComplexFontSelector = fontSelector && !fontSelector->IsSimpleFontSelectorForDescription();
    FontCascadeCacheKey key;
    key.DescriptionKey = FontDescriptionKey(description);
    for (uint32 i = 0; i < familyCount; ++i)
    {
      key.Families.push_back(FontFamilyName(description.FamilyAt(i)));
    }
    key.FontSelectorId = hasComplexFontSelector ? fontSelector->UniqueId() : 0;
    key.FontSelectorVersion = hasComplexFontSelector ? fontSelector->Version() : 0;
    key.HasComplexFontSelector = hasComplexFontSelector;
    return key;
  }

  Ref<FontCascadeFonts> FontCascadeCache::RetrieveOrAddCachedFonts(const FontCascadeDescription &description,
                                                                   FontSelector *fontSelector) noexcept
  {
    auto key = MakeFontCascadeCacheKey(description, fontSelector);

    if (auto it = _entries.find(key); it != _entries.end())
    {
      return it->second;
    }

    Ref<FontCascadeFonts> fonts = FontCascadeFonts::Create();
    _entries.emplace(krys::move(key), fonts);

    static constexpr uint32 UnreferencedPruneInterval = 1'000;
    static constexpr size_t MaximumEntries = 5'000;

    // Referenced FontCascadeFonts would exist anyway so pruning them saves little memory.
    if (!(++_pruneCounter % UnreferencedPruneInterval))
    {
      PruneUnreferencedEntries();
    }
    // Prevent pathological growth.
    if (_entries.size() > MaximumEntries)
    {
      _entries.erase(_entries.begin());
    }
    return fonts;
  }
}
