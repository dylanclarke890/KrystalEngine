#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::web
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
      RawResourceType,
    };

    virtual ~CachedResourceClient() noexcept = default;

    /// @brief Called when the resource's load is over, whether it succeeded or not.
    virtual void NotifyFinished(CachedResource &) noexcept
    {
    }

    /// @brief The client's kind, for the type casts: each kind of client overrides both.
    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return BaseResourceType;
    }

    KRYS_NODISCARD virtual CachedResourceClientType ResourceClientType() const noexcept
    {
      return ExpectedType();
    }

  protected:
    CachedResourceClient() noexcept = default;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(ToClassName, CachedResourceTypeValue)             \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::web::CachedResourceClient &client) noexcept                \
    {                                                                                                        \
      return client.ResourceClientType() == krys::web::CachedResourceClient::CachedResourceTypeValue;        \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
