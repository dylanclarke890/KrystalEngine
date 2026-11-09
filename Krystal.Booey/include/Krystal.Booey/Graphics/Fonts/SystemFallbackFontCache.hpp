/*
 * Copyright (C) 2005-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov
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

#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class Font;
  class FontDescription;

  enum class IsForPlatformFont : bool;

  struct CharacterFallbackMapKey
  {
    StringAtom Locale;
    utf8_string String;
    bool IsForPlatformFont {false};
    ResolvedEmojiPolicy ResolvedEmojiPolicy {ResolvedEmojiPolicy::NoPreference};

    KRYS_NODISCARD bool operator==(const CharacterFallbackMapKey &other) const noexcept = default;
  };

  struct CharacterFallbackMapKeyHash
  {
    KRYS_NODISCARD size_t operator()(const CharacterFallbackMapKey &key) const noexcept;
  };

  /// @brief The fallback font for each character cluster a font lacks, per font.
  class SystemFallbackFontCache
  {
    KRYS_NON_COPY_MOVABLE(SystemFallbackFontCache);

  private:
    // Fonts are not ref'd to avoid cycles.
    // FIXME: Consider changing these maps to use WeakPtr instead of raw pointers.
    using CharacterFallbackMap = HashMap<CharacterFallbackMapKey, Font *, CharacterFallbackMapKeyHash>;

    HashMap<const Font *, CharacterFallbackMap> _characterFallbackMaps;

  public:
    SystemFallbackFontCache() noexcept = default;

    KRYS_NODISCARD static SystemFallbackFontCache &Singleton() noexcept;

    /// @brief The font to draw @p characterCluster (UTF-8) with when @p font lacks it, or null if no font has
    /// it.
    KRYS_NODISCARD RefPtr<Font> SystemFallbackFontForCharacterCluster(
      const Font *font, utf8_stringview characterCluster, const FontDescription &description,
      boo::ResolvedEmojiPolicy resolvedEmojiPolicy, boo::IsForPlatformFont isForPlatformFont) noexcept;

    void Remove(Font *font) noexcept;
  };
}
