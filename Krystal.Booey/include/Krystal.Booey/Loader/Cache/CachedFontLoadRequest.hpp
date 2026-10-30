#pragma once

#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/FontLoadRequest.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <utility>

namespace krys::boo
{
  /// @brief A request for a web font loaded through the memory cache.
  class CachedFontLoadRequest final : public FontLoadRequest,
                                      public CachedFontClient,
                                      public RefCounted<CachedFontLoadRequest>
  {
  private:
    CachedResourceHandle<CachedFont> _font;
    WeakPtr<FontLoadRequestClient> _fontLoadRequestClient;
    bool _fontLoadedProcessed {false};

  public:
    KRYS_NODISCARD static Ref<CachedFontLoadRequest> Create(CachedFont &font) noexcept
    {
      return AdoptRef(*new CachedFontLoadRequest(font));
    }

    ~CachedFontLoadRequest() noexcept override
    {
      if (_fontLoadRequestClient)
      {
        _font->RemoveClient(*this);
      }
    }

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }
    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    KRYS_NODISCARD CachedFont &GetCachedFont() const noexcept
    {
      return *_font;
    }

  private:
    explicit CachedFontLoadRequest(CachedFont &font) noexcept : _font(&font)
    {
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept final
    {
      return _font->URL();
    }
    KRYS_NODISCARD bool IsPending() const noexcept final
    {
      return _font->GetStatus() == CachedResource::Status::Pending;
    }
    KRYS_NODISCARD bool IsLoading() const noexcept final
    {
      return _font->IsLoading();
    }
    KRYS_NODISCARD bool ErrorOccurred() const noexcept final
    {
      return _font->ErrorOccurred();
    }

    bool EnsureCustomFontData() noexcept final
    {
      return _font->EnsureCustomFontData();
    }

    KRYS_NODISCARD RefPtr<Font> CreateFont(const FontDescription &description, bool syntheticBold,
                                           bool syntheticItalic,
                                           const FontCreationContext &fontCreationContext) noexcept final
    {
      return _font->CreateFont(description, syntheticBold, syntheticItalic, fontCreationContext);
    }

    void SetClient(FontLoadRequestClient *client) noexcept final
    {
      auto oldClient = std::exchange(_fontLoadRequestClient, WeakPtr<FontLoadRequestClient>::WithRef(client));
      if (!client && oldClient)
      {
        _font->RemoveClient(*this);
      }
      else if (client && !oldClient)
      {
        _font->AddClient(*this);
      }
    }

    KRYS_NODISCARD bool IsCachedFontLoadRequest() const noexcept final
    {
      return true;
    }

    void FontLoaded(CachedFont &font) noexcept final
    {
      if (_fontLoadedProcessed)
      {
        return;
      }

      _fontLoadedProcessed = true;
      krys_debug_assert(&font == _font.get());
      (void)font;
      if (auto *client = _fontLoadRequestClient.get())
      {
        client->FontLoaded(
          *this); // fontLoaded() might destroy this object. Don't deref its members after it.
      }
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FONT_LOAD_REQUEST(CachedFontLoadRequest, IsCachedFontLoadRequest())
