/*
 * Copyright (C) 2007-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Font/FontFaceSource.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCreationContext.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::css
{
  void CSSFontFaceSource::SetStatus(Status newStatus) noexcept
  {
    switch (newStatus)
    {
      case Status::Pending: krys_debug_assert(false); break;
      case Status::Loading: krys_debug_assert(GetStatus() == Status::Pending); break;
      case Status::Success: krys_debug_assert(GetStatus() == Status::Loading); break;
      case Status::Failure: krys_debug_assert(GetStatus() == Status::Loading); break;
    }

    _status = newStatus;
  }

  CSSFontFaceSource::CSSFontFaceSource(CSSFontFace &owner, StringAtom fontFaceName) noexcept
      : _fontFaceName(krys::move(fontFaceName)), _owningCSSFontFace(owner)
  {
  }

  CSSFontFaceSource::CSSFontFaceSource(CSSFontFace &owner, CSSFontSelector &fontSelector,
                                       Ref<FontLoadRequest> &&request) noexcept
      : _owningCSSFontFace(owner), _fontSelector(CreateWeakPtr(&fontSelector)),
        _fontRequest(krys::move(request))
  {
    // This may synchronously call fontLoaded().
    _fontRequest->SetClient(this);

    if (GetStatus() == Status::Pending && !_fontRequest->IsPending())
    {
      SetStatus(Status::Loading);
      if (!ShouldIgnoreFontLoadCompletions())
      {
        if (_fontRequest->ErrorOccurred())
        {
          SetStatus(Status::Failure);
        }
        else
        {
          SetStatus(Status::Success);
        }
      }
    }
  }

  CSSFontFaceSource::~CSSFontFaceSource() noexcept
  {
    if (_fontRequest)
    {
      _fontRequest->SetClient(nullptr);
    }
  }

  void CSSFontFaceSource::AddRef() const noexcept
  {
    _owningCSSFontFace.AddRef();
  }

  void CSSFontFaceSource::SubRef() const noexcept
  {
    _owningCSSFontFace.SubRef();
  }

  uint32 CSSFontFaceSource::GetRefCount() const noexcept
  {
    return _owningCSSFontFace.GetRefCount();
  }

  bool CSSFontFaceSource::ShouldIgnoreFontLoadCompletions() const noexcept
  {
    return _owningCSSFontFace.ShouldIgnoreFontLoadCompletions();
  }

  void CSSFontFaceSource::OpportunisticallyStartFontDataURLLoading() noexcept
  {
    constexpr size_t MB = 1'024 * 1'024;
    if (GetStatus() == Status::Pending && _fontRequest && _fontRequest->URL().ProtocolIsData()
        && _fontRequest->URL().String().size() < MB)
    {
      Load();
    }
  }

  void CSSFontFaceSource::FontLoaded(FontLoadRequest &fontRequest) noexcept
  {
    krys_debug_assert(&fontRequest == _fontRequest.get());
    (void)fontRequest;

    if (ShouldIgnoreFontLoadCompletions())
    {
      return;
    }

    // If the font is in the cache, this will be synchronously called from FontLoadRequest::addClient().
    if (_status == Status::Pending)
    {
      SetStatus(Status::Loading);
    }
    else if (_status == Status::Failure)
    {
      // This function may be called twice if loading was cancelled.
      krys_debug_assert(_fontRequest->ErrorOccurred());
      return;
    }

    if (_fontRequest->ErrorOccurred() || !_fontRequest->EnsureCustomFontData())
    {
      SetStatus(Status::Failure);
    }
    else
    {
      SetStatus(Status::Success);
    }

    Ref<CSSFontFace> protectedFace = ShareRef(_owningCSSFontFace);
    protectedFace->FontLoaded(*this);
  }

  void CSSFontFaceSource::Load(dom::Document *) noexcept
  {
    SetStatus(Status::Loading);

    if (_fontRequest)
    {
      krys_debug_assert(_fontSelector);
      if (auto *context = _fontSelector ? _fontSelector->GetDocument() : nullptr)
      {
        context->BeginLoadingFontSoon(*_fontRequest);
      }
    }
    else
    {
      // We are only interested in whether or not fontForFamily() returns null or not. Luckily, none of
      // the values in the FontDescription other than the family name can cause the function to return
      // null if it wasn't going to otherwise (and vice-versa).
      FontCascadeDescription fontDescription;
      fontDescription.SetOneFamily(_fontFaceName);
      fontDescription.SetComputedSize(1);
      fontDescription.SetShouldAllowUserInstalledFonts(_owningCSSFontFace.GetAllowUserInstalledFonts());
      bool success = FontCache::Singleton().FontForFamily(fontDescription, _fontFaceName,
                                                          {FontLookupOptions::ExactFamilyNameMatch})
                     != nullptr;
      SetStatus(success ? Status::Success : Status::Failure);
    }
  }

  RefPtr<Font> CSSFontFaceSource::GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                          bool syntheticItalic,
                                          const FontCreationContext &fontCreationContext) noexcept
  {
    krys_debug_assert(GetStatus() == Status::Success);

    if (!_fontRequest)
    {
      // We're local. Just return a Font from the normal cache.
      // We don't want to check alternate font family names here, so pass true as the exactFamilyNameMatch
      // parameter.
      EnumSet<FontLookupOptions> options {FontLookupOptions::ExactFamilyNameMatch};
      if (!syntheticBold)
      {
        options.Add(FontLookupOptions::DisallowBoldSynthesis);
      }
      if (!syntheticItalic)
      {
        options.Add(FontLookupOptions::DisallowObliqueSynthesis);
      }
      return FontCache::Singleton().FontForFamily(fontDescription, _fontFaceName, options,
                                                  fontCreationContext);
    }

    auto success = _fontRequest->EnsureCustomFontData();
    krys_debug_assert(success);
    (void)success;

    krys_debug_assert(GetStatus() == Status::Success);
    auto result =
      _fontRequest->CreateFont(fontDescription, syntheticBold, syntheticItalic, fontCreationContext);
    krys_debug_assert(result);
    return result;
  }
}
