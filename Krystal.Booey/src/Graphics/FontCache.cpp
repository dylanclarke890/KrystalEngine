#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  size_t
    FontCache::FontPlatformDataCacheKeyHash::operator()(const FontPlatformDataCacheKey &key) const noexcept
  {
    return Hash::Combine(FontDescriptionKeyHash {}(key.DescriptionKey), key.Family.Hash());
  }

  namespace
  {
    // FIXME: The smaller limits under memory pressure, once the host reports it.
    constexpr uint32 MaxInactiveFontData = 225;
    constexpr uint32 TargetInactiveFontData = 200;
  }

  FontCache::FontCache() noexcept : _purgeTimer(*this, &FontCache::PurgeInactiveFontDataIfNeeded)
  {
  }

  FontCache::~FontCache() noexcept = default;

  FontCache &FontCache::Singleton() noexcept
  {
    static MainThreadNeverDestroyed<FontCache> cache;
    return cache;
  }

  utf8_stringview FontCache::AlternateFamilyName(utf8_stringview familyName) noexcept
  {
    switch (familyName.size())
    {
      case 5:
        if (text::IsASCIICaselessEqual(familyName, utf8_stringview {u8"arial"}))
        {
          return u8"Helvetica";
        }
        if (text::IsASCIICaselessEqual(familyName, utf8_stringview {u8"times"}))
        {
          return u8"Times New Roman";
        }
        break;
      case 7:
        if (text::IsASCIICaselessEqual(familyName, utf8_stringview {u8"courier"}))
        {
          return u8"Courier New";
        }
        break;
      case 9:
        if (text::IsASCIICaselessEqual(familyName, utf8_stringview {u8"helvetica"}))
        {
          return u8"Arial";
        }
        break;
      case 15:
        if (text::IsASCIICaselessEqual(familyName, utf8_stringview {u8"times new roman"}))
        {
          return u8"Times";
        }
        break;
    }

    return {};
  }

  FontPlatformData *FontCache::CachedFontPlatformData(const FontDescription &description,
                                                      const StringAtom &family,
                                                      EnumSet<FontLookupOptions> options) noexcept
  {
    FontPlatformDataCacheKey key {FontDescriptionKey {description}, FontFamilyName {family}};
    auto [it, isNewEntry] = _platformData.try_emplace(key, nullptr);
    if (isNewEntry)
    {
      it->second = CreateFontPlatformData(description, family, options);

      if (!it->second && !options.Contains(FontLookupOptions::ExactFamilyNameMatch))
      {
        // We were unable to find a font. We have a small set of fonts that we alias to other names,
        // e.g., Arial/Helvetica, Courier/Courier New, etc. Try looking up the font under the aliased name.
        if (auto alternateName = AlternateFamilyName(family.View()); !alternateName.empty())
        {
          auto alternateOptions = options;
          alternateOptions.Add(FontLookupOptions::ExactFamilyNameMatch);
          auto *alternateData =
            CachedFontPlatformData(description, StringAtom {alternateName}, alternateOptions);
          // Look up the key in the hash table again as the previous iterator may have
          // been invalidated by the recursive call to cachedFontPlatformData().
          it = _platformData.find(key);
          krys_debug_assert(it != _platformData.end());
          if (alternateData)
          {
            it->second = CreateUnique<FontPlatformData>(*alternateData);
          }
        }
      }
    }

    return it->second.get();
  }

  RefPtr<Font> FontCache::FontForFamily(const FontDescription &description, const StringAtom &family,
                                        EnumSet<FontLookupOptions> options) noexcept
  {
    if (!_purgeTimer.IsActive())
    {
      _purgeTimer.StartOneShot({});
    }

    if (auto *platformData = CachedFontPlatformData(description, family, options))
    {
      return FontForPlatformData(*platformData);
    }

    return nullptr;
  }

  Ref<Font> FontCache::FontForPlatformData(const FontPlatformData &platformData) noexcept
  {
    auto it = _data.find(platformData);
    if (it == _data.end())
    {
      it = _data.emplace(platformData, Font::Create(platformData, FontOrigin::Local)).first;
    }
    krys_debug_assert(it->second->PlatformData() == platformData);
    return it->second;
  }

  RefPtr<Font> FontCache::SimilarFont(const FontDescription &, const StringAtom &) noexcept
  {
    return nullptr;
  }

  size_t FontCache::FontCount() const noexcept
  {
    return _data.size();
  }

  size_t FontCache::InactiveFontCount() const noexcept
  {
    size_t count = 0;
    for (auto &[platformData, font] : _data)
    {
      if (font->HasOneRef())
      {
        ++count;
      }
    }
    return count;
  }

  void FontCache::ReleaseNoncriticalMemory() noexcept
  {
    PurgeInactiveFontData();
    _fontCascadeCache.ClearWidthCaches();
  }

  void FontCache::PurgeInactiveFontDataIfNeeded() noexcept
  {
    if (_data.size() < MaxInactiveFontData)
    {
      return;
    }
    size_t inactiveCount = InactiveFontCount();
    if (inactiveCount <= MaxInactiveFontData)
    {
      return;
    }

    PurgeInactiveFontData(static_cast<uint32>(inactiveCount - TargetInactiveFontData));
  }

  void FontCache::PurgeInactiveFontData(uint32 purgeCount) noexcept
  {
    _fontCascadeCache.PruneUnreferencedEntries();
    _fontCascadeCache.PruneSystemFallbackFonts();

    while (purgeCount)
    {
      List<Ref<Font>> fontsToDelete;
      for (auto &[platformData, font] : _data)
      {
        if (!font->HasOneRef())
        {
          continue;
        }
        fontsToDelete.push_back(font);
        if (!--purgeCount)
        {
          break;
        }
      }
      // Fonts may ref other fonts so we loop until there are no changes.
      if (fontsToDelete.empty())
      {
        break;
      }
      for (auto &font : fontsToDelete)
      {
        [[maybe_unused]] bool success = _data.erase(font->PlatformData());
        krys_debug_assert(success);
      }
    }

    std::erase_if(_platformData, [&](auto &entry) { return entry.second && !_data.contains(*entry.second); });
  }

  void FontCache::UpdateFontCascade(const FontCascade &fontCascade) noexcept
  {
    fontCascade.UpdateFonts(_fontCascadeCache.RetrieveOrAddCachedFonts(fontCascade.GetFontDescription(),
                                                                       fontCascade.GetFontSelector()));
  }

  void FontCache::AddClient(FontSelector &client) noexcept
  {
    krys_debug_assert(
      std::ranges::none_of(_clients, [&](auto &existing) { return existing.get() == &client; }));
    _clients.push_back(WeakPtr<FontSelector>::WithRef(client));
  }

  void FontCache::RemoveClient(FontSelector &client) noexcept
  {
    std::erase_if(_clients, [&](auto &existing) { return !existing || existing.get() == &client; });
  }

  void FontCache::Invalidate() noexcept
  {
    _platformData.clear();
    _fontCascadeCache.Invalidate();
    SystemFontDatabase::Singleton().Invalidate();

    ++_generation;

    auto clients = _clients;
    for (auto &client : clients)
    {
      if (client)
      {
        Ref protectedClient = ShareRef(*client);
        protectedClient->FontCacheInvalidated();
      }
    }

    PurgeInactiveFontData();
  }

  void FontCache::InvalidateAllFontCaches() noexcept
  {
    Singleton().Invalidate();
  }

  bool FontCache::UseBackslashAsYenSignForFamily(const StringAtom &family) noexcept
  {
    if (family.View().empty())
    {
      return false;
    }
    if (_familiesUsingBackslashAsYenSign.empty())
    {
      for (auto name : {u8"MS PGothic", u8"MS PMincho", u8"MS Gothic", u8"MS Mincho", u8"Meiryo",
                        u8"\uFF2D\uFF33 \uFF30\u30B4\u30B7\u30C3\u30AF", u8"\uFF2D\uFF33 \uFF30\u660E\u671D",
                        u8"\uFF2D\uFF33 \u30B4\u30B7\u30C3\u30AF", u8"\uFF2D\uFF33 \u660E\u671D",
                        u8"\u30E1\u30A4\u30EA\u30AA"})
      {
        _familiesUsingBackslashAsYenSign.insert(name);
      }
    }
    return _familiesUsingBackslashAsYenSign.contains(utf8_string {family.View()});
  }

  // MARK: - Platform

  /// @brief Whether a face of @p capabilities shows @p description's weight and style, or needs synthesis.
  static void GetFontPropertiesFromFace(const FontSelectionCapabilities &capabilities,
                                        const FontDescription &description,
                                        EnumSet<FontLookupOptions> options, bool &syntheticBold,
                                        bool &syntheticOblique) noexcept
  {
    syntheticBold = false;
    bool allowSyntheticBold =
      description.HasAutoFontSynthesisWeight() && !options.Contains(FontLookupOptions::DisallowBoldSynthesis);
    if (allowSyntheticBold && IsFontWeightBold(description.Weight()))
    {
      // A face lighter than semi-bold is emboldened.
      syntheticBold = capabilities.Weight.Maximum < FontSelectionValue {600};
    }

    // We requested an italic font, but the face is neither oblique nor italic.
    syntheticOblique = false;
    bool allowSyntheticOblique = description.HasAutoFontSynthesisStyle()
                                 && !options.Contains(FontLookupOptions::DisallowObliqueSynthesis);
    if (allowSyntheticOblique && description.FontStyleSlope())
    {
      syntheticOblique = capabilities.Slope.Maximum == NormalItalicValue();
    }
  }

  KRYS_NODISCARD static FontPlatformData PlatformDataForFace(text::FreeTypeFace &face,
                                                             const FontDescription &description,
                                                             EnumSet<FontLookupOptions> options) noexcept
  {
    bool syntheticBold = false;
    bool syntheticOblique = false;
    GetFontPropertiesFromFace(CapabilitiesOfFace(face), description, options, syntheticBold,
                              syntheticOblique);
    return FontPlatformData(ShareRef(face), description.ComputedSize(), syntheticBold, syntheticOblique,
                            description.Orientation(), description.WidthVariant(),
                            description.GetTextRenderingMode());
  }

  UniquePtr<FontPlatformData> FontCache::CreateFontPlatformData(const FontDescription &description,
                                                                const StringAtom &family,
                                                                EnumSet<FontLookupOptions> options) noexcept
  {
    // The CSS font matching algorithm (http://www.w3.org/TR/css3-fonts/#font-matching-algorithm)
    // says that we must find an exact match for font family, then pick the face that best matches the
    // width, style and weight.
    auto *face = FontDatabase::Singleton().MatchFace(family.View(), description.GetFontSelectionRequest());
    if (!face)
    {
      return nullptr;
    }

    auto platformData = CreateUnique<FontPlatformData>(PlatformDataForFace(*face, description, options));
    // FIXME: Adjust the size for font-size-adjust (FontPlatformData::updateSizeWithFontSizeAdjust).
    return platformData;
  }

  Ref<Font> FontCache::LastResortFallbackFont(const FontDescription &description) noexcept
  {
    return FontForPlatformData(PlatformDataForFace(FontDatabase::Singleton().BundledFace(), description, {}));
  }

  KRYS_NODISCARD static bool FaceHasCharacters(const text::FreeTypeFace &face,
                                               utf8_stringview characters) noexcept
  {
    bool hasAny = false;
    for (text::CodePointIterator it {characters}; !it.AtEnd(); ++it)
    {
      // Variation selectors only select a presentation.
      if (IsVariationSelector(*it))
      {
        continue;
      }
      if (!face.GlyphIndex(*it))
      {
        return false;
      }
      hasAny = true;
    }
    return hasAny;
  }

  RefPtr<Font> FontCache::SystemFallbackForCharacterCluster(const FontDescription &description, const Font &,
                                                            IsForPlatformFont, PreferColoredFont,
                                                            utf8_stringview characterCluster) noexcept
  {
    auto &database = FontDatabase::Singleton();

    // The registered faces that have every character, the best match for the description first (the earliest
    // registered when they match as well); the bundled faces are the last to try: the text face, then the
    // Arabic face, then the math face.
    List<text::FreeTypeFace *> candidates;
    List<FontSelectionCapabilities> capabilities;
    for (auto &face : database.Faces())
    {
      if (face.get() == &database.BundledFace() || face.get() == &database.BundledArabicFace()
          || face.get() == &database.BundledMathFace() || !FaceHasCharacters(*face, characterCluster))
      {
        continue;
      }
      candidates.push_back(face.get());
      capabilities.push_back(CapabilitiesOfFace(*face));
    }

    text::FreeTypeFace *face = nullptr;
    if (!candidates.empty())
    {
      FontSelectionAlgorithm algorithm {description.GetFontSelectionRequest(), capabilities};
      face = candidates[algorithm.IndexOfBestCapabilities()];
    }
    else if (FaceHasCharacters(database.BundledFace(), characterCluster))
    {
      face = &database.BundledFace();
    }
    else if (FaceHasCharacters(database.BundledArabicFace(), characterCluster))
    {
      face = &database.BundledArabicFace();
    }
    else if (FaceHasCharacters(database.BundledMathFace(), characterCluster))
    {
      face = &database.BundledMathFace();
    }

    if (!face)
    {
      return nullptr;
    }
    return FontForPlatformData(PlatformDataForFace(*face, description, {}));
  }

  List<FontSelectionCapabilities> FontCache::GetFontSelectionCapabilitiesInFamily(
    const StringAtom &familyName, AllowUserInstalledFonts allowUserInstalledFonts) const noexcept
  {
    // Every registered font is the host's, so none is installed by the user.
    (void)allowUserInstalledFonts;

    List<FontSelectionCapabilities> result;
    for (auto &face : FontDatabase::Singleton().FacesInFamily(familyName.View()))
    {
      result.push_back(CapabilitiesOfFace(*face));
    }
    return result;
  }

  List<utf8_string> FontCache::SystemFontFamilies() const noexcept
  {
    return FontDatabase::Singleton().Families();
  }
}
