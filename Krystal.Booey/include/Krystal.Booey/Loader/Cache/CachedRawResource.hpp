#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"

namespace krys::boo
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CachedRawResource)
  KRYS_NODISCARD static bool IsType(const krys::boo::CachedResource &resource) noexcept
  {
    return resource.GetType() == krys::boo::CachedResource::Type::RawResource
           || resource.GetType() == krys::boo::CachedResource::Type::Icon;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
