#pragma once

#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Net/Url.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceClient.hpp"

namespace krys::web
{
  class CachedCSSStyleSheet;

  /// @brief What a style sheet load is for (a `<link>`, an `@import` or an `xml-stylesheet` processing
  /// instruction), given the sheet once the load finishes, whether it succeeded or not.
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

    /// @brief Called when @p sheet has finished loading, or straight away if it already had.
    /// @param href The URL the sheet was requested from.
    /// @param baseUrl The URL of the response, which the sheet's relative URLs resolve against.
    /// @param charset The encoding the sheet was decoded from.
    /// @param sheet The loaded sheet, whose text is null if it can't be used.
    virtual void SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                                  const CachedCSSStyleSheet *sheet) noexcept = 0;

  protected:
    CachedStyleSheetClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedStyleSheetClient, StyleSheetType)
