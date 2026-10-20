#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFaceSource.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleRangeValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/UnicodeRangeValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCreationContext.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Graphics/FontRanges.hpp"
#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::css
{
  namespace
  {
    constexpr MonotonicTime::duration InfinitePeriod = MonotonicTime::duration::max();
  }

  template <typename TFunc>
  void CSSFontFace::IterateClients(TFunc &&callback) noexcept
  {
    auto clients = _clients;
    for (auto &client : clients)
    {
      if (auto *liveClient = client.get())
      {
        callback(*liveClient);
      }
    }
  }

  void CSSFontFace::AppendSources(CSSFontFace &fontFace, ValueList &srcList, dom::Document *context,
                                  bool isInitiatingElementInUserAgentShadowTree) noexcept
  {
    // FIXME: A setting for the trusted types of downloadable fonts, once there's a safe font parser.
    bool allowDownloading = context;
    for (auto &src : srcList)
    {
      // An item in the list either specifies a string (local font name) or a URL (remote font to download).
      if (const auto *local = DynamicDowncast<FontFaceSrcLocalValue>(src))
      {
        // FIXME: Local sources that are SVG `<font-face>` elements, once there are SVG fonts.
        fontFace.AdoptSource(
          UniquePtr<CSSFontFaceSource>(new CSSFontFaceSource(fontFace, local->FontFaceName())));
      }
      else
      {
        if (allowDownloading)
        {
          if (RefPtr<FontLoadRequest> request =
                Downcast<FontFaceSrcResourceValue>(const_cast<Value &>(src))
                  .GetFontLoadRequest(*context, isInitiatingElementInUserAgentShadowTree))
          {
            Ref<FontLoadRequest> fontRequest = krys::move(request);
            fontFace.AdoptSource(UniquePtr<CSSFontFaceSource>(
              new CSSFontFaceSource(fontFace, context->GetFontSelector(), krys::move(fontRequest))));
          }
        }
      }
    }
    fontFace.SourcesPopulated();
  }

  Ref<CSSFontFace> CSSFontFace::Create(CSSFontSelector &fontSelector, FontFaceRule *cssConnection,
                                       bool isLocalFallback) noexcept
  {
    auto result = AdoptRef(*new CSSFontFace(cssConnection, isLocalFallback));
    result->AddClient(fontSelector.FontFaceClient());
    return result;
  }

  CSSFontFace::CSSFontFace(FontFaceRule *connection, bool isLocalFallback) noexcept
      : _cssConnection(ShareRefPtr(connection)),
        _properties(connection ? nullptr : RefPtr<MutableStyleProperties>(MutableStyleProperties::Create())),
        _isLocalFallback(isLocalFallback), _timeoutTimer(*this, &CSSFontFace::TimeoutFired)
  {
  }

  CSSFontFace::~CSSFontFace() noexcept = default;

  const StyleProperties &CSSFontFace::Properties() const noexcept
  {
    if (_cssConnection)
    {
      return _cssConnection->Properties();
    }
    return *_properties;
  }

  MutableStyleProperties &CSSFontFace::MutableProperties() noexcept
  {
    if (_cssConnection)
    {
      return _cssConnection->MutableProperties();
    }
    return *_properties;
  }

  FontFaceRule *CSSFontFace::CssConnection() const noexcept
  {
    return _cssConnection.get();
  }

  void CSSFontFace::SetFamily(Value &family) noexcept
  {
    auto oldFamily = std::exchange(_family, ShareRefPtr(&family));
    MutableProperties().SetProperty(PropertyId::FontFamily, ShareRef(family));

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this, oldFamily.get()); });
  }

  namespace
  {
    KRYS_NODISCARD FontSelectionRange CalculateWeightRange(const Value &value) noexcept
    {
      if (const auto *valueList = DynamicDowncast<ValueList>(value))
      {
        krys_debug_assert(valueList->Length() == 2);
        if (valueList->Length() != 2)
        {
          return {NormalWeightValue(), NormalWeightValue()};
        }
        krys_debug_assert(valueList->Item(0)->IsPrimitiveValue());
        krys_debug_assert(valueList->Item(1)->IsPrimitiveValue());
        auto result0 = style::FontWeightFromCSSValueDeprecated(*valueList->Item(0));
        auto result1 = style::FontWeightFromCSSValueDeprecated(*valueList->Item(1));
        return {result0, result1};
      }

      FontSelectionValue result = style::FontWeightFromCSSValueDeprecated(value);
      return {result, result};
    }

    KRYS_NODISCARD FontSelectionRange CalculateWidthRange(const Value &value) noexcept
    {
      if (const auto *valueList = DynamicDowncast<ValueList>(value))
      {
        krys_debug_assert(valueList->Length() == 2);
        if (valueList->Length() != 2)
        {
          return {NormalWidthValue(), NormalWidthValue()};
        }
        krys_debug_assert(valueList->Item(0)->IsPrimitiveValue());
        krys_debug_assert(valueList->Item(1)->IsPrimitiveValue());
        auto result0 = style::FontStretchFromCSSValueDeprecated(*valueList->Item(0));
        auto result1 = style::FontStretchFromCSSValueDeprecated(*valueList->Item(1));
        return {result0, result1};
      }

      FontSelectionValue result = style::FontStretchFromCSSValueDeprecated(value);
      return {result, result};
    }

    KRYS_NODISCARD FontSelectionRange CalculateItalicRange(const Value &value) noexcept
    {
      const auto *rangeValue = DynamicDowncast<FontStyleRangeValue>(value);
      if (!rangeValue)
      {
        return FontSelectionRange {
          style::FontStyleFromCSSValueDeprecated(value).value_or(NormalItalicValue())};
      }

      auto keyword = rangeValue->FontStyleValue->ValueId();
      if (!rangeValue->ObliqueValues)
      {
        if (keyword == ValueId::Normal)
        {
          return FontSelectionRange {NormalItalicValue()};
        }
        krys_debug_assert(keyword == ValueId::Italic || keyword == ValueId::Oblique);
        return FontSelectionRange {ItalicValue()};
      }
      krys_debug_assert(keyword == ValueId::Oblique);
      auto length = rangeValue->ObliqueValues->Length();
      krys_debug_assert(length == 1 || length == 2);
      auto angleAtIndex = [rangeValue](uint32 index)
      {
        return style::FontStyleAngleFromCSSValueDeprecated(
          *rangeValue->ObliqueValues->ItemWithoutBoundsCheck(index));
      };
      if (length == 1)
      {
        return FontSelectionRange {angleAtIndex(0)};
      }
      return {angleAtIndex(0), angleAtIndex(1)};
    }
  }

  void CSSFontFace::SetWeight(Value &weight) noexcept
  {
    MutableProperties().SetProperty(PropertyId::FontWeight, ShareRef(weight));

    auto range = CalculateWeightRange(weight);
    if (_fontSelectionCapabilities.Weight == range)
    {
      return;
    }

    _fontSelectionCapabilities.Weight = range;

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetWidth(Value &width) noexcept
  {
    MutableProperties().SetProperty(PropertyId::FontWidth, ShareRef(width));

    auto range = CalculateWidthRange(width);
    if (_fontSelectionCapabilities.Width == range)
    {
      return;
    }

    _fontSelectionCapabilities.Width = range;

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetStyle(Value &style) noexcept
  {
    MutableProperties().SetProperty(PropertyId::FontStyle, ShareRef(style));

    auto range = CalculateItalicRange(style);
    if (_fontSelectionCapabilities.Slope == range)
    {
      return;
    }

    _fontSelectionCapabilities.Slope = range;

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetUnicodeRange(ValueList &list) noexcept
  {
    MutableProperties().SetProperty(PropertyId::UnicodeRange, ShareRef(list));

    List<UnicodeRange> ranges;
    ranges.reserve(list.size());
    for (auto &rangeValue : list)
    {
      const auto &range = Downcast<UnicodeRangeValue>(rangeValue);
      ranges.push_back({range.From(), range.To()});
    }

    if (ranges == _ranges)
    {
      return;
    }

    _ranges = krys::move(ranges);

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetFeatureSettings(Value &featureSettings) noexcept
  {
    // Can only call this with a primitive value of normal, or a value list containing font feature values.
    krys_debug_assert(Is<PrimitiveValue>(featureSettings) || Is<ValueList>(featureSettings));

    MutableProperties().SetProperty(PropertyId::FontFeatureSettings, ShareRef(featureSettings));

    FontFeatureSettings settings;

    if (const auto *list = DynamicDowncast<ValueList>(featureSettings))
    {
      for (auto &rangeValue : *list)
      {
        const auto &feature = Downcast<FontFeatureValue>(rangeValue);
        if (const auto *primitiveValue = DynamicDowncast<PrimitiveValue>(feature.GetValue()))
        {
          settings.insert({feature.Tag(), primitiveValue->ResolveAsIntegerDeprecated<int>()});
        }
      }
    }

    if (_featureSettings == settings)
    {
      return;
    }

    _featureSettings = krys::move(settings);

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetSizeAdjust(Value &value) noexcept
  {
    MutableProperties().SetProperty(PropertyId::SizeAdjust, ShareRef(value));

    const auto &sizeAdjustValue = Downcast<PrimitiveValue>(value);
    auto sizeAdjust = sizeAdjustValue.ResolveAsPercentageDeprecated<float>() / 100;

    if (_sizeAdjust == sizeAdjust)
    {
      return;
    }

    _sizeAdjust = sizeAdjust;

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  void CSSFontFace::SetDisplay(Value &loadingBehaviorValue) noexcept
  {
    MutableProperties().SetProperty(PropertyId::FontDisplay, ShareRef(loadingBehaviorValue));

    auto loadingBehavior = FromCSSValue<FontLoadingBehavior>(loadingBehaviorValue);

    if (_loadingBehavior == loadingBehavior)
    {
      return;
    }

    _loadingBehavior = loadingBehavior;

    IterateClients([&](CSSFontFaceClient &client) { client.FontPropertyChanged(*this); });
  }

  RefPtr<Value> CSSFontFace::FamilyCSSValue() const noexcept
  {
    return _family;
  }

  bool CSSFontFace::RangesMatchCodePoint(char32 character) const noexcept
  {
    if (_ranges.empty())
    {
      return true;
    }

    return std::ranges::any_of(_ranges, [character](const UnicodeRange &range)
                               { return range.From <= character && character <= range.To; });
  }

  void CSSFontFace::FontLoadEventOccurred() noexcept
  {
    // If the font is already in the cache, CSSFontFaceSource may report it's loaded before it is added here
    // as a source. Let's not pump the state machine until we've got all our sources. font() and load() are
    // smart enough to act correctly when a source is failed or succeeded before we have asked it to load.
    if (_sourcesPopulated)
    {
      Pump(ExternalResourceDownloadPolicy::Forbid);
    }

    IterateClients([&](CSSFontFaceClient &client) { client.FontLoaded(*this); });
  }

  void CSSFontFace::TimeoutFired() noexcept
  {
    Ref<CSSFontFace> protectedThis = ShareRef(*this);

    switch (GetStatus())
    {
      case Status::Loading: SetStatus(Status::TimedOut); break;
      case Status::TimedOut:
        // Cancelling the network request here could lead to a situation where a site's font never gets
        // shown as the user navigates around to different pages on the site. This would occur if the
        // download always takes longer than the timeout (even though the user may spend substantial time
        // on each page). Therefore, we shouldn't cancel the network request here, but should use the
        // loading infrastructure's timeout policies instead.
        SetStatus(Status::Failure);
        break;
      default: krys_debug_assert(false); break;
    }

    FontLoadEventOccurred();
  }

  bool CSSFontFace::ComputeFailureState() const noexcept
  {
    if (GetStatus() == Status::Failure)
    {
      return true;
    }
    return std::ranges::all_of(_sources, [](const UniquePtr<CSSFontFaceSource> &source)
                               { return source->GetStatus() == CSSFontFaceSource::Status::Failure; });
  }

  void CSSFontFace::AddClient(CSSFontFaceClient &client) noexcept
  {
    _clients.push_back(WeakPtr<CSSFontFaceClient>::WithRef(client));
  }

  void CSSFontFace::RemoveClient(CSSFontFaceClient &client) noexcept
  {
    auto found = std::ranges::find_if(_clients, [&](const WeakPtr<CSSFontFaceClient> &existing)
                                      { return existing.get() == &client; });
    krys_debug_assert(found != _clients.end());
    if (found != _clients.end())
    {
      _clients.erase(found);
    }
  }

  void CSSFontFace::AdoptSource(UniquePtr<CSSFontFaceSource> &&source) noexcept
  {
    _sources.push_back(krys::move(source));

    // We should never add sources in the middle of loading.
    krys_debug_assert(!_sourcesPopulated);
  }

  CSSFontFace::FontLoadTiming CSSFontFace::GetFontLoadTiming() const noexcept
  {
    using namespace std::chrono_literals;

    // FIXME: The font load timing override setting, once there are settings for it.
    switch (_loadingBehavior)
    {
      case FontLoadingBehavior::Auto:
      case FontLoadingBehavior::Block:    return {3s, InfinitePeriod};
      case FontLoadingBehavior::Swap:     return {0s, InfinitePeriod};
      case FontLoadingBehavior::Fallback: return {100ms, 3s};
      case FontLoadingBehavior::Optional: return {100ms, 0s};
    }
    krys_assert_msg(false, "Not reached");
    return {3s, InfinitePeriod};
  }

  void CSSFontFace::SetStatus(Status newStatus) noexcept
  {
    switch (newStatus)
    {
      case Status::Pending:  krys_debug_assert(false); break;
      case Status::Loading:  krys_debug_assert(_status == Status::Pending); break;
      case Status::TimedOut: krys_debug_assert(_status == Status::Loading); break;
      case Status::Success:
        krys_debug_assert(_status == Status::Loading || _status == Status::TimedOut);
        break;
      case Status::Failure:
        krys_debug_assert(_status == Status::Loading || _status == Status::TimedOut);
        break;
    }

    IterateClients([&](CSSFontFaceClient &client) { client.FontStateChanged(*this, _status, newStatus); });

    _status = newStatus;

    auto timeouts = GetFontLoadTiming();
    auto blockPeriodTimeout = timeouts.BlockPeriod;
    auto swapPeriodTimeout = timeouts.SwapPeriod;

    // Transfer across 0-delay timers synchronously. Layouts/script may
    // take arbitrarily long time, and we shouldn't be in a 0-duration
    // state for an arbitrarily long time. Also it's necessary for
    // testing so we don't have a race with the font load.
    switch (newStatus)
    {
      case Status::Pending: krys_debug_assert(false); break;
      case Status::Loading:
        if (blockPeriodTimeout == MonotonicTime::duration::zero())
        {
          SetStatus(Status::TimedOut);
        }
        else if (blockPeriodTimeout != InfinitePeriod)
        {
          _timeoutTimer.StartOneShot(blockPeriodTimeout);
        }
        break;
      case Status::TimedOut:
        if (swapPeriodTimeout == MonotonicTime::duration::zero())
        {
          SetStatus(Status::Failure);
        }
        else if (swapPeriodTimeout != InfinitePeriod)
        {
          _timeoutTimer.StartOneShot(swapPeriodTimeout);
        }
        break;
      case Status::Success:
      case Status::Failure: _timeoutTimer.Stop(); break;
    }
  }

  void CSSFontFace::FontLoaded(CSSFontFaceSource &) noexcept
  {
    Ref<CSSFontFace> protectedThis = ShareRef(*this);

    FontLoadEventOccurred();
  }

  void CSSFontFace::OpportunisticallyStartFontDataURLLoading() noexcept
  {
    // We don't want to go crazy here and blow the cache. Usually these data URLs are the first item in the
    // src: list, so let's just check that one.
    if (!_sources.empty())
    {
      _sources[0]->OpportunisticallyStartFontDataURLLoading();
    }
  }

  size_t CSSFontFace::Pump(ExternalResourceDownloadPolicy policy) noexcept
  {
    if (GetStatus() == Status::Failure)
    {
      return 0;
    }

    size_t i;
    for (i = 0; i < _sources.size(); ++i)
    {
      auto &source = _sources[i];

      if (source->GetStatus() == CSSFontFaceSource::Status::Pending)
      {
        krys_debug_assert(_status == Status::Pending || _status == Status::Loading
                          || _status == Status::TimedOut);
        // This is a little tricky. After calling CSSFontFace::font(Forbid), a font must never fail later in
        // this turn of the runloop because the return value of CSSFontFace::font() shouldn't get nulled out
        // from under an existing FontRanges object. Remote fonts are all downloaded asynchronously, so this
        // isn't a problem for them because CSSFontFace::font() will always return the interstitial font.
        // However, local fonts may synchronously fail when you call load() on them. Therefore, we have to
        // call load() here in order to guarantee that, if the font synchronously fails, it happens now during
        // the first call to CSSFontFace::font() and the FontRanges object sees a consistent view of the
        // CSSFontFace. This means we eagerly create some internal font objects when they may not be needed,
        // but it seems that this behavior is a requirement of the design of FontRanges. FIXME: Perhaps
        // rethink this design.
        if (policy == ExternalResourceDownloadPolicy::Allow || !source->RequiresExternalResource())
        {
          if (policy == ExternalResourceDownloadPolicy::Allow && _status == Status::Pending)
          {
            SetStatus(Status::Loading);
          }
          source->Load();
        }
      }

      switch (source->GetStatus())
      {
        case CSSFontFaceSource::Status::Pending:
          krys_debug_assert(policy == ExternalResourceDownloadPolicy::Forbid);
          return i;
        case CSSFontFaceSource::Status::Loading:
          krys_debug_assert(_status == Status::Pending || _status == Status::Loading
                            || _status == Status::TimedOut || _status == Status::Failure);
          if (policy == ExternalResourceDownloadPolicy::Allow && _status == Status::Pending)
          {
            SetStatus(Status::Loading);
          }
          return i;
        case CSSFontFaceSource::Status::Success:
          krys_debug_assert(_status == Status::Pending || _status == Status::Loading
                            || _status == Status::TimedOut || _status == Status::Success
                            || _status == Status::Failure);
          if (_status == Status::Pending)
          {
            SetStatus(Status::Loading);
          }
          if (_status == Status::Loading || _status == Status::TimedOut)
          {
            SetStatus(Status::Success);
          }
          return i;
        case CSSFontFaceSource::Status::Failure:
          if (policy == ExternalResourceDownloadPolicy::Allow && _status == Status::Pending)
          {
            SetStatus(Status::Loading);
          }
          break;
      }
    }
    if (_sources.empty() && _status == Status::Pending)
    {
      SetStatus(Status::Loading);
    }
    if (_status == Status::Loading || _status == Status::TimedOut)
    {
      SetStatus(Status::Failure);
    }
    return _sources.size();
  }

  void CSSFontFace::Load() noexcept
  {
    Pump(ExternalResourceDownloadPolicy::Allow);
  }

  namespace
  {
    KRYS_NODISCARD Font::Visibility GetVisibility(CSSFontFace::Status status,
                                                  CSSFontFace::FontLoadTiming timing) noexcept
    {
      switch (status)
      {
        case CSSFontFace::Status::Pending:
          return timing.BlockPeriod == MonotonicTime::duration::zero() ? Font::Visibility::Visible
                                                                       : Font::Visibility::Invisible;
        case CSSFontFace::Status::Loading:  return Font::Visibility::Invisible;
        case CSSFontFace::Status::TimedOut:
        case CSSFontFace::Status::Failure:
        case CSSFontFace::Status::Success:
        default:                            return Font::Visibility::Visible;
      }
    }
  }

  RefPtr<Font> CSSFontFace::GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                    bool syntheticItalic, ExternalResourceDownloadPolicy policy,
                                    const FontPaletteValues &fontPaletteValues,
                                    RefPtr<FontFeatureValues> fontFeatureValues) noexcept
  {
    if (ComputeFailureState())
    {
      return nullptr;
    }

    Ref<CSSFontFace> protectedThis = ShareRef(*this);

    // Our status is derived from the first non-failed source. However, this source may
    // return null from font(), which means we need to continue looping through the remainder
    // of the sources to try to find a font to use. These subsequent tries should not affect
    // our own state, though.
    size_t startIndex = Pump(policy);

    if (ComputeFailureState())
    {
      return nullptr;
    }

    for (size_t i = startIndex; i < _sources.size(); ++i)
    {
      auto &source = _sources[i];
      if (source->GetStatus() == CSSFontFaceSource::Status::Pending
          && (policy == ExternalResourceDownloadPolicy::Allow || !source->RequiresExternalResource()))
      {
        source->Load();
      }

      switch (source->GetStatus())
      {
        case CSSFontFaceSource::Status::Pending:
        case CSSFontFaceSource::Status::Loading:
        {
          Font::Visibility visibility = GetVisibility(GetStatus(), GetFontLoadTiming());
          return Font::Create(FontCache::Singleton().LastResortFallbackFont(fontDescription)->PlatformData(),
                              Font::Origin::Local, FontIsInterstitial::Yes, visibility);
        }
        case CSSFontFaceSource::Status::Success:
        {
          FontCreationContext fontCreationContext {_featureSettings, _fontSelectionCapabilities,
                                                   fontPaletteValues, fontFeatureValues, _sizeAdjust};
          if (auto result =
                source->GetFont(fontDescription, syntheticBold, syntheticItalic, fontCreationContext))
          {
            return result;
          }
          break;
        }
        case CSSFontFaceSource::Status::Failure: break;
      }
    }

    return nullptr;
  }

  bool CSSFontFace::Purgeable() const noexcept
  {
    return CssConnection() && _mayBePurged;
  }

  void CSSFontFace::UpdateStyleIfNeeded() noexcept
  {
    IterateClients([&](CSSFontFaceClient &client) { client.UpdateStyleIfNeeded(*this); });
  }

  void CSSFontFace::SetErrorState() noexcept
  {
    switch (_status)
    {
      case Status::Pending: SetStatus(Status::Loading); break;
      case Status::Success: krys_debug_assert(false); break;
      case Status::Failure: return;
      default:              break;
    }

    SetStatus(Status::Failure);
  }
}
