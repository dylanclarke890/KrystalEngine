#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
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

  CSSFontSelector::CSSFontSelector(dom::Document &document) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document)), _uniqueId(++FontSelectorId)
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
    // FIXME: Whether there are any @font-face rules, once there's a font face set.
    return true;
  }

  void CSSFontSelector::BuildStarted() noexcept
  {
    _buildIsUnderway = true;
    ++_version;
    // FIXME: Purge the font face set and note the faces from rules, once there are @font-face rules.
  }

  void CSSFontSelector::BuildCompleted() noexcept
  {
    if (!_buildIsUnderway)
    {
      return;
    }

    _buildIsUnderway = false;
    // FIXME: Remove the faces whose rules are gone and add the staged @font-face rules, once there are any.
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

  void CSSFontSelector::OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &,
                                                                 const StringAtom &) noexcept
  {
    // FIXME: Start loading the data: URLs of the family's @font-face rules, once there are any.
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

    // FIXME: Look the family up in the @font-face rules (resolving the standard family first), once there are
    // any.
    resolveAndAssignGenericFamily();

    auto font = FontCache::Singleton().FontForFamily(description, familyForLookup);
    return {FontRanges {krys::move(font)}, isGenericFontFamily};
  }

  void CSSFontSelector::ClearFonts() noexcept
  {
    _isStopped = true;
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

    // FIXME: Pending @font-face rules, faces, feature values and palettes make it complex, once there are
    // any.
    return true;
  }
}
