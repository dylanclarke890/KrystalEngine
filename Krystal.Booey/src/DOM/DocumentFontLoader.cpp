#include "Krystal.Booey/DOM/DocumentFontLoader.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"

namespace krys::boo::dom
{
  DocumentFontLoader::DocumentFontLoader(Document &document) noexcept
      : _document(document), _fontLoadingTimer(*this, &DocumentFontLoader::FontLoadingTimerFired)
  {
  }

  DocumentFontLoader::~DocumentFontLoader() noexcept
  {
    StopLoadingAndClearFonts();
  }

  CachedFont *DocumentFontLoader::GetCachedFont(url::URL &&url, bool isSVG,
                                                bool isInitiatingElementInUserAgentShadowTree,
                                                LoadedFromOpaqueSource loadedFromOpaqueSource) noexcept
  {
    ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();
    options.ContentSecurityPolicy = isInitiatingElementInUserAgentShadowTree
                                      ? ContentSecurityPolicyImposition::SkipPolicyCheck
                                      : ContentSecurityPolicyImposition::DoPolicyCheck;
    options.IsLoadedFromOpaqueSource = loadedFromOpaqueSource;
    // FIXME: The same-origin data URL flag, once there are origins.

    CachedResourceRequest request(ResourceRequest(krys::move(url)), options);
    request.SetInitiatorType(CachedResourceRequestInitiatorTypes::CSS);
    auto font = _document.GetCachedResourceLoader().RequestFont(krys::move(request), isSVG);
    return font ? font->get() : nullptr;
  }

  void DocumentFontLoader::BeginLoadingFontSoon(CachedFont &font) noexcept
  {
    if (_isStopped)
    {
      return;
    }

    _fontsToBeginLoading.push_back(CachedResourceHandle<CachedFont>(font));
    // FIXME: Increment the loader's request count now, in order to prevent the load event from being
    // dispatched after this font has been requested but before it began loading, once there's one.
    if (!_isFontLoadingSuspended && !_fontLoadingTimer.IsActive())
    {
      _fontLoadingTimer.StartOneShot({});
    }
  }

  void DocumentFontLoader::LoadPendingFonts() noexcept
  {
    if (_isFontLoadingSuspended)
    {
      return;
    }

    List<CachedResourceHandle<CachedFont>> fontsToBeginLoading;
    fontsToBeginLoading.swap(_fontsToBeginLoading);

    Ref<CachedResourceLoader> cachedResourceLoader = ShareRef(_document.GetCachedResourceLoader());
    for (auto &fontHandle : fontsToBeginLoading)
    {
      fontHandle->BeginLoadIfNeeded(*cachedResourceLoader);
    }
  }

  void DocumentFontLoader::FontLoadingTimerFired() noexcept
  {
    LoadPendingFonts();
    // FIXME: Tell the frame loader that the loads are done, so it can check whether the document has loaded,
    // once there's one.
  }

  void DocumentFontLoader::StopLoadingAndClearFonts() noexcept
  {
    if (_isStopped)
    {
      return;
    }

    _fontLoadingTimer.Stop();
    _fontsToBeginLoading.clear();
    if (auto *fontSelector = _document.FontSelectorIfExists())
    {
      fontSelector->ClearFonts();
    }

    _isFontLoadingSuspended = true;
    _isStopped = true;
  }

  void DocumentFontLoader::SuspendFontLoading() noexcept
  {
    if (_isFontLoadingSuspended)
    {
      return;
    }

    _fontLoadingTimer.Stop();
    _isFontLoadingSuspended = true;
  }

  void DocumentFontLoader::ResumeFontLoading() noexcept
  {
    if (!_isFontLoadingSuspended || _isStopped)
    {
      return;
    }

    _isFontLoadingSuspended = false;
    if (!_fontsToBeginLoading.empty())
    {
      _fontLoadingTimer.StartOneShot({});
    }
  }
}
