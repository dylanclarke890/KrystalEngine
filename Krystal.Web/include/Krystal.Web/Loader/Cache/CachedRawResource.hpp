#pragma once

#include "Krystal.Web/Loader/Cache/CachedResource.hpp"

namespace krys::web
{
  /// @brief A resource kept as its bytes (`ResourceBuffer`), for its client to make what it likes of: an
  /// icon.
  class CachedRawResource final : public CachedResource
  {
  public:
    CachedRawResource(CachedResourceRequest &&request, Type type) noexcept;

  private:
    void FinishLoading(const SharedBuffer *data) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CachedRawResource)
  KRYS_NODISCARD static bool IsType(const krys::web::CachedResource &resource) noexcept
  {
    return resource.GetType() == krys::web::CachedResource::Type::RawResource
           || resource.GetType() == krys::web::CachedResource::Type::Icon;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
