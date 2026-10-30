#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  /// @brief A client of a cached SVG document.
  class CachedSVGDocumentClient : public CachedResourceClient
  {
  public:
    ~CachedSVGDocumentClient() noexcept override = default;

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return SVGDocumentType;
    }
    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

  protected:
    CachedSVGDocumentClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedSVGDocumentClient, SVGDocumentType)
