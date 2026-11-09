/*
 * Copyright (C) 2021 Metrological Group B.V.
 * Copyright (C) 2021 Igalia S.L.
 * Copyright (C) 2007, 2008, 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
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
    // FIXME: Counting the fonts waiting to load as requests that hold back the load event.
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
      KRYS_NODISCARD CachedFont *GetCachedFont(url::URL url, bool isSVG,
                                               bool isInitiatingElementInUserAgentShadowTree) noexcept;
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
