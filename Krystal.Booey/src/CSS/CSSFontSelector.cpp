#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/Font/FontFaceSet.hpp"
#include "Krystal.Booey/CSS/Font/SegmentedFontFace.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>

namespace krys::boo::css
{
  using namespace WebKitFontFamilyNames;

  namespace
  {
    uint32 FontSelectorId = 0;
  }

  Ref<CSSFontSelector> CSSFontSelector::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new CSSFontSelector(document));
  }

  size_t CSSFontSelector::PaletteKeyHash::operator()(const PaletteKey &key) const noexcept
  {
    return Hash::Combine(std::hash<StringAtom> {}(key.Family), std::hash<StringAtom> {}(key.Name));
  }

  CSSFontSelector::CSSFontSelector(dom::Document &document) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document)), _cssFontFaceSet(CSSFontFaceSet::Create(this)),
        _fontFaceClient(*this), _uniqueId(++FontSelectorId)
  {
    FontCache::Singleton().AddClient(*this);
  }

  CSSFontSelector::~CSSFontSelector() noexcept
  {
    ClearFonts();
    FontCache::Singleton().RemoveClient(*this);
  }

  bool CSSFontSelector::IsEmpty() const noexcept
  {
    return !_cssFontFaceSet->FaceCount();
  }

  void CSSFontSelector::EmptyCaches() noexcept
  {
    _cssFontFaceSet->EmptyCaches();
  }

  void CSSFontSelector::BuildStarted() noexcept
  {
    _buildIsUnderway = true;
    _cssFontFaceSet->Purge();
    ++_version;

    krys_debug_assert(_cssConnectionsPossiblyToRemove.empty());
    krys_debug_assert(_cssConnectionsEncounteredDuringBuild.empty());
    krys_debug_assert(_stagingArea.empty());
    for (size_t i = 0; i < _cssFontFaceSet->FaceCount(); ++i)
    {
      auto &face = (*_cssFontFaceSet)[i];
      if (face.CssConnection())
      {
        _cssConnectionsPossiblyToRemove.push_back(ShareRef(face));
      }
    }

    _paletteMap.clear();
  }

  void CSSFontSelector::BuildCompleted() noexcept
  {
    if (!_buildIsUnderway)
    {
      return;
    }

    _buildIsUnderway = false;

    // Some font faces weren't re-added during the build process.
    for (auto &face : _cssConnectionsPossiblyToRemove)
    {
      auto *connection = face->CssConnection();
      krys_debug_assert(connection);
      if (!_cssConnectionsEncounteredDuringBuild.contains(connection))
      {
        _cssFontFaceSet->Remove(*face);
      }
    }

    auto stagingArea = krys::move(_stagingArea);
    _stagingArea.clear();
    for (auto &item : stagingArea)
    {
      AddFontFaceRule(*item.StyleRuleFontFace, item.IsInitiatingElementInUserAgentShadowTree);
    }
    _cssConnectionsEncounteredDuringBuild.clear();
    _cssConnectionsPossiblyToRemove.clear();
  }

  void CSSFontSelector::AddFontFaceRule(FontFaceRule &fontFaceRule,
                                        bool isInitiatingElementInUserAgentShadowTree) noexcept
  {
    if (_buildIsUnderway)
    {
      _cssConnectionsEncounteredDuringBuild.insert(&fontFaceRule);
      _stagingArea.push_back({ShareRef(fontFaceRule), isInitiatingElementInUserAgentShadowTree});
      return;
    }

    const auto &style = fontFaceRule.Properties();
    auto fontFamily = style.GetPropertyCSSValue(PropertyId::FontFamily);
    auto fontStyle = style.GetPropertyCSSValue(PropertyId::FontStyle);
    auto fontWeight = style.GetPropertyCSSValue(PropertyId::FontWeight);
    auto fontWidth = style.GetPropertyCSSValue(PropertyId::FontWidth);
    auto src = style.GetPropertyCSSValue(PropertyId::Src);
    auto *srcList = DynamicDowncast<ValueList>(src.get());
    auto unicodeRange = style.GetPropertyCSSValue(PropertyId::UnicodeRange);
    auto *rangeList = DynamicDowncast<ValueList>(unicodeRange.get());
    auto featureSettings = style.GetPropertyCSSValue(PropertyId::FontFeatureSettings);
    auto display = style.GetPropertyCSSValue(PropertyId::FontDisplay);
    auto sizeAdjust = style.GetPropertyCSSValue(PropertyId::SizeAdjust);
    if (!fontFamily || !srcList || (unicodeRange && !rangeList))
    {
      return;
    }

    if (!srcList->Length())
    {
      return;
    }

    SetForScope creatingFont(_creatingFont, true);
    auto fontFace = CSSFontFace::Create(*this, &fontFaceRule);

    fontFace->SetFamily(*fontFamily);
    if (fontStyle)
    {
      fontFace->SetStyle(*fontStyle);
    }
    if (fontWeight)
    {
      fontFace->SetWeight(*fontWeight);
    }
    if (fontWidth)
    {
      fontFace->SetWidth(*fontWidth);
    }
    if (rangeList)
    {
      fontFace->SetUnicodeRange(*rangeList);
    }
    if (featureSettings)
    {
      fontFace->SetFeatureSettings(*featureSettings);
    }
    if (display)
    {
      fontFace->SetDisplay(Downcast<PrimitiveValue>(*display));
    }
    if (sizeAdjust)
    {
      fontFace->SetSizeAdjust(*sizeAdjust);
    }

    CSSFontFace::AppendSources(*fontFace, *srcList, _document.get(), isInitiatingElementInUserAgentShadowTree);

    if (RefPtr<CSSFontFace> existingFace = ShareRefPtr(_cssFontFaceSet->LookUpByCSSConnection(fontFaceRule)))
    {
      // This adoption is fairly subtle. Script can trigger a purge of _cssFontFaceSet at any time,
      // which will cause us to just rely on the memory cache to retain the bytes of the file the next
      // time we build up the CSSFontFaceSet. However, when the CSS Font Loading API is involved,
      // the FontFace and FontFaceSet objects need to retain state. We create the new CSSFontFace object
      // while the old one is still in scope so that the memory cache will be forced to retain the bytes
      // of the resource. This means that the CachedFont will temporarily have two clients (until the
      // old CSSFontFace goes out of scope, which should happen at the end of this "if" block). Because
      // the CSSFontFaceSource objects will inspect their CachedFonts, the new CSSFontFace is smart enough
      // to enter the correct state() during the next pump(). This approach of making a new CSSFontFace is
      // simpler than computing and applying a diff of the StyleProperties.
      // FIXME: Pass the old face's `FontFace` wrapper to the new face, once there's one.
      _cssFontFaceSet->Remove(*existingFace);
    }

    _cssFontFaceSet->Add(*fontFace);
    ++_version;
  }

  void CSSFontSelector::AddFontPaletteValuesRule(const FontPaletteValuesRule &fontPaletteValuesRule) noexcept
  {
    const auto &name = fontPaletteValuesRule.Name();
    krys_debug_assert(name != StringAtom::Null());

    const auto &fontFamilies = fontPaletteValuesRule.FontFamilies();
    if (fontFamilies.empty())
    {
      return;
    }

    for (const auto &fontFamily : fontFamilies)
    {
      _paletteMap.insert_or_assign(PaletteKey {fontFamily, name}, fontPaletteValuesRule.FontPaletteValues());
    }

    ++_version;
  }

  void CSSFontSelector::AddFontFeatureValuesRule(const FontFeatureValuesRule &fontFeatureValuesRule) noexcept
  {
    Ref<FontFeatureValues> fontFeatureValues = fontFeatureValuesRule.Value();

    for (const auto &fontFamily : fontFeatureValuesRule.FontFamilies())
    {
      // https://www.w3.org/TR/css-fonts-3/#font-family-casing
      auto lowercased = text::ToASCIILower(fontFamily.View());
      if (auto existing = _featureValues.find(lowercased); existing != _featureValues.end())
      {
        existing->second->UpdateOrInsert(*fontFeatureValues);
      }
      else
      {
        _featureValues.insert_or_assign(krys::move(lowercased), fontFeatureValues);
      }
    }

    ++_version;
  }

  void CSSFontSelector::RegisterForInvalidationCallbacks(FontSelectorClient &client) noexcept
  {
    if (std::ranges::find(_clients, &client) == _clients.end())
    {
      _clients.push_back(&client);
    }
  }

  void CSSFontSelector::UnregisterForInvalidationCallbacks(FontSelectorClient &client) noexcept
  {
    std::erase(_clients, &client);
  }

  void CSSFontSelector::DispatchInvalidationCallbacks() noexcept
  {
    ++_version;

    auto clients = _clients;
    for (auto *client : clients)
    {
      if (std::ranges::find(_clients, client) != _clients.end())
      {
        client->FontsNeedUpdate(*this);
      }
    }
  }

  void CSSFontSelector::OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                                 const StringAtom &familyName) noexcept
  {
    auto *segmentedFontFace = _cssFontFaceSet->GetFontFace(description.GetFontSelectionRequest(), familyName);
    if (!segmentedFontFace)
    {
      return;
    }

    if (!_document)
    {
      return;
    }

    for (auto &face : segmentedFontFace->ConstituentFaces())
    {
      face->OpportunisticallyStartFontDataURLLoading();
    }
  }

  void CSSFontSelector::FontFaceClientImpl::FontLoaded(CSSFontFace &) noexcept
  {
    _owner.DispatchInvalidationCallbacks();
  }

  void CSSFontSelector::FontModified() noexcept
  {
    if (!_creatingFont && !_buildIsUnderway)
    {
      DispatchInvalidationCallbacks();
    }
  }

  void CSSFontSelector::FontFaceClientImpl::UpdateStyleIfNeeded(CSSFontFace &) noexcept
  {
    _owner.UpdateStyleIfNeeded();
  }

  void CSSFontSelector::UpdateStyleIfNeeded() noexcept
  {
    if (auto document = ShareRefPtr(_document.get()))
    {
      document->UpdateStyleIfNeeded();
    }
  }

  void CSSFontSelector::FontCacheInvalidated() noexcept
  {
    DispatchInvalidationCallbacks();
  }

  void CSSFontSelector::SettingsChanged() noexcept
  {
    DispatchInvalidationCallbacks();
  }

  Maybe<StringAtom> CSSFontSelector::ResolveGenericFamily(const FontDescription &description,
                                                          const StringAtom &family) noexcept
  {
    if (!_document)
    {
      return null;
    }

    auto &settings = _document->GetSettings().Values;

    auto script = description.Script();
    for (uint8 index = 0; index <= ToUnderlying(FamilyNamesIndex::SystemUiFamily); ++index)
    {
      auto familyNameIndex = static_cast<FamilyNamesIndex>(index);
      if (FamilyName(familyNameIndex) != family)
      {
        continue;
      }
      if (auto *familyString = settings.FontGenericFamilies.FontFamily(familyNameIndex, script))
      {
        return StringAtom {*familyString};
      }
      break;
    }

    return null;
  }

  const FontPaletteValues &
    CSSFontSelector::LookupFontPaletteValues(const StringAtom &familyName,
                                             const FontDescription &description) const noexcept
  {
    static const NeverDestroyed<FontPaletteValues> emptyFontPaletteValues;
    if (description.GetFontPalette().PaletteType != FontPalette::Type::Custom)
    {
      return emptyFontPaletteValues.get();
    }

    const StringAtom &paletteName = description.GetFontPalette().Identifier;

    auto iterator = _paletteMap.find(PaletteKey {familyName, paletteName});
    if (iterator == _paletteMap.end())
    {
      return emptyFontPaletteValues.get();
    }

    return iterator->second;
  }

  RefPtr<FontFeatureValues>
    CSSFontSelector::LookupFontFeatureValues(const StringAtom &familyName) const noexcept
  {
    // https://www.w3.org/TR/css-fonts-3/#font-family-casing
    auto lowercased = text::ToASCIILower(familyName.View());
    auto iterator = _featureValues.find(lowercased);
    if (iterator == _featureValues.end())
    {
      return nullptr;
    }

    return iterator->second;
  }

  // Fonts with appropriate Unicode coverage and OpenType features are required for good math
  // rendering. These requirements as well as the up-to-date list of known math fonts to fulfill
  // these requirements are listed on http://trac.webkit.org/wiki/MathML/Fonts.
  using MathFontList = Array<utf8_stringview, 19>;

  KRYS_NODISCARD static const MathFontList &GetMathFontList() noexcept
  {
    static constexpr MathFontList list = {
      // This font has Computer Modern style and is provided with most TeX & Linux distributions.
      // We put it as the default because its style is familiar to TeX, Wikipedia and math people.
      u8"Latin Modern Math",
      // The following fonts have Times style and are provided with most TeX & Linux distributions.
      // We put XITS & STIX as a second option because they have very good unicode coverage.
      // STIX Two is a complete redesign of STIX that fixes serious bugs in version one so we put it in first
      // position. XITS is a fork of STIX with bug fixes and more Arabic/RTL features so we put it in second
      // position.
      u8"STIX Two Math",
      u8"XITS Math",
      u8"STIX Math",
      u8"Libertinus Math",
      u8"TeX Gyre Termes Math",
      // These fonts respectively have style compatible with Bookman Old and Century Schoolbook.
      // They are provided with most TeX & Linux distributions.
      u8"TeX Gyre Bonum Math",
      u8"TeX Gyre Schola",
      // DejaVu is pre-installed on many Linux distributions and is included in LibreOffice.
      u8"DejaVu Math TeX Gyre",
      // The following fonts have Palatino style and are provided with most TeX & Linux distributions.
      // Asana Math has some rendering issues (e.g. missing italic correction) so we put it after.
      u8"TeX Gyre Pagella Math",
      u8"Asana Math",
      // The following fonts are proprietary and have not much been tested so we put them at the end.
      // Cambria Math it is pre-installed on Windows 7 and higher.
      u8"Cambria Math",
      u8"Lucida Bright Math",
      u8"Minion Math",
      // The following fonts do not satisfy the requirements for good mathematical rendering.
      // These are pre-installed on Mac and iOS so we list them to provide minimal unicode-based
      // mathematical rendering. For more explanation of fallback mechanisms and missing features see
      // http://trac.webkit.org/wiki/MathML/Fonts#ObsoleteFontsandFallbackMechanisms.
      // STIX fonts have best unicode coverage so we put them first.
      u8"STIXGeneral",
      u8"STIXSizeOneSym",
      u8"Symbol",
      u8"Times New Roman",
      // Mathematical fonts generally use "serif" style. Hence we append the generic "serif" family
      // as a fallback in order to increase our chance to find a mathematical font.
      u8"serif",
    };
    return list;
  }

  FontRanges CSSFontSelector::FontRangesForFamily(const FontDescription &description,
                                                  const StringAtom &family) noexcept
  {
    // If this ASSERT() fires, it usually means you forgot a document.updateStyleIfNeeded() somewhere.
    krys_debug_assert(!_buildIsUnderway || _computingRootStyleFontCount);

    // FIXME: The spec (and Firefox) says user specified generic families (sans-serif etc.) should be resolved
    // before the @font-face lookup too.
    bool resolveGenericFamilyFirst = family == StandardFamily();

    StringAtom familyForLookup = family;
    auto isGenericFontFamily = IsGenericFontFamily::No;
    auto resolveAndAssignGenericFamily = [&]
    {
      if (auto genericFamily = ResolveGenericFamily(description, family))
      {
        familyForLookup = *genericFamily;
        isGenericFontFamily = IsGenericFontFamily::Yes;
      }
    };

    const auto &fontPaletteValues = LookupFontPaletteValues(family, description);
    auto fontFeatureValues = LookupFontFeatureValues(family);

    // Handle the generic math font family a bit differently.
    if (family == MathFamily() && _document)
    {
      // First check if the user has defined a preference.
      auto &settings = _document->GetSettings().Values;
      const auto &preferredMathFamily = settings.FontGenericFamilies.MathFontFamily(description.Script());
      if (!preferredMathFamily.empty() && family.View() != preferredMathFamily)
      {
        auto ranges = FontRangesForFamily(description, StringAtom {preferredMathFamily});
        if (!ranges.IsNull())
        {
          return {krys::move(ranges), IsGenericFontFamily::Yes};
        }
      }

      // Otherwise, iterate through the font list to find a valid fallback.
      for (auto mathFamily : GetMathFontList())
      {
        auto ranges = FontRangesForFamily(description, StringAtom {mathFamily});
        if (!ranges.IsNull())
        {
          return {krys::move(ranges), IsGenericFontFamily::Yes};
        }
      }
    }

    if (resolveGenericFamilyFirst)
    {
      resolveAndAssignGenericFamily();
    }
    if (auto *face = _cssFontFaceSet->GetFontFace(description.GetFontSelectionRequest(), familyForLookup))
    {
      Ref<CSSSegmentedFontFace> protectedFace = ShareRef(*face);
      return {protectedFace->GetFontRanges(description, fontPaletteValues, fontFeatureValues),
              isGenericFontFamily};
    }

    if (!resolveGenericFamilyFirst)
    {
      resolveAndAssignGenericFamily();
    }

    // FIXME: The palette and feature values, once the font cache takes them.
    auto font = FontCache::Singleton().FontForFamily(description, familyForLookup);
    return {FontRanges {krys::move(font)}, isGenericFontFamily};
  }

  void CSSFontSelector::ClearFonts() noexcept
  {
    _isStopped = true;
    _cssFontFaceSet->Clear();
    _clients.clear();
  }

  size_t CSSFontSelector::FallbackFontCount() noexcept
  {
    if (_isStopped || !_document)
    {
      return 0;
    }

    return _document->GetSettings().Values.FontFallbackPrefersPictographs ? 1 : 0;
  }

  RefPtr<Font> CSSFontSelector::FallbackFontAt(const FontDescription &description,
                                               [[maybe_unused]] size_t index) noexcept
  {
    krys_debug_assert(!index);

    if (_isStopped || !_document)
    {
      return nullptr;
    }

    auto &settings = _document->GetSettings().Values;
    if (!settings.FontFallbackPrefersPictographs)
    {
      return nullptr;
    }
    auto &pictographFontFamily = settings.FontGenericFamilies.PictographFontFamily();
    return FontCache::Singleton().FontForFamily(description, StringAtom {pictographFontFamily});
  }

  bool CSSFontSelector::IsSimpleFontSelectorForDescription() const noexcept
  {
    // FIXME: remove this when we fix counter style rules mutation.
    if (_document && _document->StyleScope().GetCounterStyleRegistry().HasAuthorCounterStyles())
    {
      return false;
    }

    // Fonts are shared between documents with simple font selectors, so generic families from the
    // document's own settings make it complex.
    if (_document)
    {
      auto &settings = _document->GetSettings().Values;
      if (!settings.FontGenericFamilies.IsEmpty() || settings.FontFallbackPrefersPictographs)
      {
        return false;
      }
    }

    // font face rules still pending
    if (!_stagingArea.empty())
    {
      return false;
    }

    return !_cssFontFaceSet->FaceCount() && _featureValues.empty() && _paletteMap.empty();
  }
}
