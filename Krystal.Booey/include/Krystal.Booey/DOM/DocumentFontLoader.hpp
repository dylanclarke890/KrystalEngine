#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/LoadedFromOpaqueSource.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  namespace url
  {
    class URL;
  }

  class CachedFont;

  namespace dom
  {
    class Document;

    /// @brief Requests a document's web fonts, and begins loading them soon after something first needs them.
    /// @note Owned by its document.
    // FIXME: Counting the fonts waiting to load as requests that hold back the load event, once there's one.
    class DocumentFontLoader : public CanMakeWeakPtr<DocumentFontLoader>
    {
      KRYS_NON_COPY_MOVABLE(DocumentFontLoader);

    private:
      Document &_document;
      Timer _fontLoadingTimer;
      List<CachedResourceHandle<CachedFont>> _fontsToBeginLoading;
      bool _isFontLoadingSuspended {false};
      bool _isStopped {false};

    public:
      explicit DocumentFontLoader(Document &document) noexcept;
      ~DocumentFontLoader() noexcept;

      /// @brief The cached font for the file at @p url, or null if it can't be requested. Its file doesn't
      /// load until `BeginLoadingFontSoon`.
      KRYS_NODISCARD CachedFont *GetCachedFont(url::URL &&url, bool isSVG,
                                               bool isInitiatingElementInUserAgentShadowTree,
                                               LoadedFromOpaqueSource loadedFromOpaqueSource) noexcept;
      void BeginLoadingFontSoon(CachedFont &font) noexcept;

      void LoadPendingFonts() noexcept;
      void StopLoadingAndClearFonts() noexcept;

      void SuspendFontLoading() noexcept;
      void ResumeFontLoading() noexcept;

    private:
      void FontLoadingTimerFired() noexcept;
    };
  }
}
