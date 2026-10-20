#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CachedResource;

  /// @brief Something told when a cached resource finishes loading.
  /// @note A client removes itself from the resources it was added to before it's destroyed.
  class CachedResourceClient
  {
    KRYS_NON_COPYABLE(CachedResourceClient);

  public:
    enum CachedResourceClientType : uint8
    {
      BaseResourceType,
      ImageType,
      FontType,
      StyleSheetType,
      SVGDocumentType,
      RawResourceType
    };

    virtual ~CachedResourceClient() noexcept = default;

    virtual void NotifyFinished(CachedResource &) noexcept
    {
    }

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return BaseResourceType;
    }
    KRYS_NODISCARD virtual CachedResourceClientType ResourceClientType() const noexcept
    {
      return ExpectedType();
    }
    KRYS_NODISCARD virtual bool ShouldMarkAsReferenced() const noexcept
    {
      return true;
    }

  protected:
    CachedResourceClient() noexcept = default;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(ToClassName, CachedResourceTypeValue)             \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::CachedResourceClient &client) noexcept                \
    {                                                                                                        \
      return client.ResourceClientType() == krys::boo::CachedResourceClient::CachedResourceTypeValue;        \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
