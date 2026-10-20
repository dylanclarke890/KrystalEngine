#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class CachedCSSStyleSheet;

  /// @brief A client of a cached style sheet, told when its sheet has loaded (or failed).
  class CachedStyleSheetClient : public CachedResourceClient
  {
  public:
    ~CachedStyleSheetClient() noexcept override = default;

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return StyleSheetType;
    }
    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

    virtual void SetCSSStyleSheet(const utf8_string & /* href */, const url::URL & /* baseURL */,
                                  utf8_stringview /* charset */, const CachedCSSStyleSheet *) noexcept
    {
    }

  protected:
    CachedStyleSheetClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedStyleSheetClient, StyleSheetType)
