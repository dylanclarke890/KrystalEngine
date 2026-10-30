#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCreationContext.hpp"
#include "Krystal.Booey/Graphics/FontCustomPlatformData.hpp"
#include "Krystal.Booey/Graphics/FontPlatformData.hpp"
#include "Krystal.Booey/Graphics/WOFFFileFormat.hpp"
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
