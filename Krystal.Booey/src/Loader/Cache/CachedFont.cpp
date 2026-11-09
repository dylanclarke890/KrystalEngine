/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile, Inc.
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

#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCreationContext.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCustomPlatformData.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPlatformData.hpp"
#include "Krystal.Booey/Graphics/Fonts/WOFFFileFormat.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"

namespace krys::boo
{
  CachedFont::CachedFont(CachedResourceRequest &&request, Type type) noexcept
      : CachedResource(krys::move(request), type)
  {
  }

  CachedFont::~CachedFont() noexcept = default;

  void CachedFont::Load(CachedResourceLoader &) noexcept
  {
    // Don't load the file yet.  Wait for an access before triggering the load.
    SetLoading(true);
  }

  void CachedFont::DidAddClient(CachedResourceClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedFontClient::ExpectedType());
    if (!IsLoading())
    {
      Downcast<CachedFontClient>(client).FontLoaded(*this);
    }
  }

  void CachedFont::FinishLoading(const SharedBuffer *data) noexcept
  {
    // FIXME: A font parsing policy (fonts the safe font parser must parse, or that are refused), once
    // there's a safe font parser.
    if (data)
    {
      _data = ShareRefPtr(const_cast<SharedBuffer *>(data));
      SetEncodedSize(static_cast<uint32>(_data->Size()));
    }
    else
    {
      _data = nullptr;
      SetEncodedSize(0);
    }
    SetLoading(false);
    CheckNotify();
  }

  void CachedFont::BeginLoadIfNeeded(CachedResourceLoader &loader) noexcept
  {
    if (!_loadInitiated)
    {
      _loadInitiated = true;
      CachedResource::Load(loader);
    }
  }

  bool CachedFont::EnsureCustomFontData() noexcept
  {
    return EnsureCustomFontData(_data.get());
  }

  css::CSSOMString CachedFont::CalculateItemInCollection() const noexcept
  {
    return css::CSSOMString(URL().FragmentIdentifier());
  }

  bool CachedFont::EnsureCustomFontData(SharedBuffer *data) noexcept
  {
    if (!_fontCustomPlatformData && !ErrorOccurred() && !IsLoading() && data)
    {
      bool wrapping = false;
      _fontCustomPlatformData = CreateCustomFontData(*data, CalculateItemInCollection(), wrapping);
      _hasCreatedFontDataWrappingResource = _fontCustomPlatformData && wrapping;
      if (!_fontCustomPlatformData)
      {
        SetStatus(DecodeError);
      }
    }

    return _fontCustomPlatformData.get();
  }

  RefPtr<FontCustomPlatformData> CachedFont::CreateCustomFontData(SharedBuffer &bytes,
                                                                  const css::CSSOMString &itemInCollection,
                                                                  bool &wrapping) noexcept
  {
    RefPtr<SharedBuffer> buffer = ShareRefPtr(&bytes);
    wrapping = !ConvertWOFFToSfntIfNecessary(buffer);
    return buffer ? FontCustomPlatformData::Create(*buffer, itemInCollection) : nullptr;
  }

  RefPtr<Font> CachedFont::CreateFont(const FontDescription &fontDescription, bool syntheticBold,
                                      bool syntheticItalic,
                                      const FontCreationContext &fontCreationContext) noexcept
  {
    return Font::Create(
      PlatformDataFromCustomData(fontDescription, syntheticBold, syntheticItalic, fontCreationContext),
      Font::Origin::Remote);
  }

  FontPlatformData
    CachedFont::PlatformDataFromCustomData(const FontDescription &fontDescription, bool bold, bool italic,
                                           const FontCreationContext &fontCreationContext) noexcept
  {
    krys_debug_assert(_fontCustomPlatformData);
    return PlatformDataFromCustomData(*_fontCustomPlatformData, fontDescription, bold, italic,
                                      fontCreationContext);
  }

  FontPlatformData
    CachedFont::PlatformDataFromCustomData(FontCustomPlatformData &fontCustomPlatformData,
                                           const FontDescription &fontDescription, bool bold, bool italic,
                                           const FontCreationContext &fontCreationContext) noexcept
  {
    return fontCustomPlatformData.GetFontPlatformData(fontDescription, bold, italic, fontCreationContext);
  }

  void CachedFont::AllClientsRemoved() noexcept
  {
    _fontCustomPlatformData = nullptr;
  }

  void CachedFont::CheckNotify() noexcept
  {
    if (IsLoading())
    {
      return;
    }

    CachedResourceClientWalker<CachedFontClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->FontLoaded(*this);
    }
  }
}
