#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/FontRanges.hpp"
#include "Krystal.Booey/Graphics/GlyphPage.hpp"
#include "Krystal.Booey/Graphics/WidthCache.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class FontCascadeDescription;
  class FontPlatformData;
  class FontSelector;
  class MixedFontGlyphPage;

  /// @brief The fonts a font cascade description resolves to: the font ranges of each family in turn
  /// (realized on demand), the primary font, and the glyphs of the characters looked up so far, with the
  /// system fallback fonts they needed.
  class FontCascadeFonts : public RefCounted<FontCascadeFonts>
  {
    KRYS_NON_COPY_MOVABLE(FontCascadeFonts);

  private:
    class GlyphPageCacheEntry
    {
    private:
      // Only one of these is non-null.
      RefPtr<GlyphPage> _singleFont;
      UniquePtr<MixedFontGlyphPage> _mixedFont;

    public:
      GlyphPageCacheEntry() noexcept;
      explicit GlyphPageCacheEntry(RefPtr<GlyphPage> &&singleFont) noexcept;
      GlyphPageCacheEntry(GlyphPageCacheEntry &&) noexcept;
      GlyphPageCacheEntry &operator=(GlyphPageCacheEntry &&) noexcept;
      ~GlyphPageCacheEntry() noexcept;

      KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 character) const noexcept;

      void SetSingleFontPage(RefPtr<GlyphPage> &&page) noexcept;
      void SetGlyphDataForCharacter(char32 character, GlyphData glyphData) noexcept;

      KRYS_NODISCARD bool IsNull() const noexcept
      {
        return !_singleFont && !_mixedFont;
      }

      KRYS_NODISCARD bool IsMixedFont() const noexcept
      {
        return !!_mixedFont;
      }
    };

    static constexpr size_t ResolvedEmojiPolicyCount = 3;

    SmallList<FontRanges, 1> _realizedFallbackRanges;
    uint32 _lastRealizedFallbackIndex {0};

    Array<HashMap<uint32, GlyphPageCacheEntry>, ResolvedEmojiPolicyCount> _cachedPages;

    HashSet<RefPtr<Font>> _systemFallbackFontSet;

    WidthCache _widthCache;

    WeakPtr<const Font> _cachedPrimaryFont;

    uint16 _generation {0};
    Pitch _pitch {UnknownPitch};
    bool _isForPlatformFont {false};
    Maybe<bool> _canTakeFixedPitchFastContentMeasuring;

    FontCascadeFonts() noexcept;
    explicit FontCascadeFonts(const FontPlatformData &platformData) noexcept;

  public:
    ~FontCascadeFonts() noexcept;

    KRYS_NODISCARD static Ref<FontCascadeFonts> Create() noexcept
    {
      return AdoptRef(*new FontCascadeFonts());
    }

    KRYS_NODISCARD static Ref<FontCascadeFonts>
      CreateForPlatformFont(const FontPlatformData &platformData) noexcept
    {
      return AdoptRef(*new FontCascadeFonts(platformData));
    }

    KRYS_NODISCARD bool IsForPlatformFont() const noexcept
    {
      return _isForPlatformFont;
    }

    KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 character,
                                                   const FontCascadeDescription &description,
                                                   FontSelector *fontSelector, FontVariant variant,
                                                   ResolvedEmojiPolicy resolvedEmojiPolicy) noexcept;

    KRYS_NODISCARD bool IsFixedPitch(const FontCascadeDescription &description,
                                     FontSelector *fontSelector) noexcept;
    KRYS_NODISCARD bool CanTakeFixedPitchFastContentMeasuring(const FontCascadeDescription &description,
                                                              FontSelector *fontSelector) noexcept;

    KRYS_NODISCARD bool IsLoadingCustomFonts() const noexcept;

    KRYS_NODISCARD WidthCache &GetWidthCache() noexcept
    {
      return _widthCache;
    }
    KRYS_NODISCARD const WidthCache &GetWidthCache() const noexcept
    {
      return _widthCache;
    }

    // FIXME: It should be possible to combine fontSelectorVersion and generation.
    KRYS_NODISCARD uint16 Generation() const noexcept
    {
      return _generation;
    }

    KRYS_NODISCARD const Font &PrimaryFont(const FontCascadeDescription &description,
                                           FontSelector *fontSelector) noexcept;
    KRYS_NODISCARD const FontRanges &RealizeFallbackRangesAt(const FontCascadeDescription &description,
                                                             FontSelector *fontSelector,
                                                             uint32 fallbackIndex) noexcept;

    void PruneSystemFallbacks() noexcept;

  private:
    KRYS_NODISCARD GlyphData GlyphDataForSystemFallback(char32 character,
                                                        const FontCascadeDescription &description,
                                                        FontSelector *fontSelector, FontVariant variant,
                                                        ResolvedEmojiPolicy resolvedEmojiPolicy,
                                                        bool systemFallbackShouldBeInvisible) noexcept;
    KRYS_NODISCARD GlyphData GlyphDataForVariant(char32 character, const FontCascadeDescription &description,
                                                 FontSelector *fontSelector, FontVariant variant,
                                                 ResolvedEmojiPolicy resolvedEmojiPolicy,
                                                 uint32 fallbackIndex = 0) noexcept;

    void DeterminePitch(const FontCascadeDescription &description, FontSelector *fontSelector) noexcept;
    void DetermineCanTakeFixedPitchFastContentMeasuring(const FontCascadeDescription &description,
                                                        FontSelector *fontSelector) noexcept;
  };
}
