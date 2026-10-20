#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  class CachedFont;

  /// @brief A client of a cached font, told when its file has loaded (or failed).
  class CachedFontClient : public CachedResourceClient
  {
  public:
    ~CachedFontClient() noexcept override = default;

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return FontType;
    }
    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

    virtual void FontLoaded(CachedFont &) noexcept
    {
    }

  protected:
    CachedFontClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedFontClient, FontType)
