#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo::css
{
  struct ParserContext;
  class StyleSheetContents;
}

namespace krys::boo
{
  /// @brief A style sheet resource: the text a load brought, and the sheet last parsed from it, kept so
  /// another `<link>` to the same URL can reuse it rather than parse the text again.
  // Style sheets are UTF-8 only, whatever the request, the response or `@charset` say (see DIFFERENCES.md).
  class CachedCSSStyleSheet final : public CachedResource
  {
  private:
    /// @brief The text, decoded while the clients are told the load finished, and dropped after.
    Maybe<utf8_string> _decodedSheetText;
    RefPtr<css::StyleSheetContents> _parsedStyleSheetCache;

  public:
    explicit CachedCSSStyleSheet(const CachedResourceRequest &request) noexcept;

    ~CachedCSSStyleSheet() noexcept override;

    /// @brief The sheet's text, or null if the response isn't a [usable style
    /// sheet](https://html.spec.whatwg.org/#link-type-stylesheet:process-the-linked-resource): the load
    /// failed, the HTTP status isn't OK, or the type isn't CSS.
    KRYS_NODISCARD Maybe<utf8_string> SheetText() const noexcept;

    /// @brief The sheet saved by SaveParsedStyleSheet(), or null if there's none, it can't be reused or it
    /// was parsed with a context other than @p context.
    KRYS_NODISCARD RefPtr<css::StyleSheetContents>
      RestoreParsedStyleSheet(const css::ParserContext &context) noexcept;

    /// @brief Keeps @p sheet, parsed from this resource's text, for RestoreParsedStyleSheet().
    void SaveParsedStyleSheet(const Ref<css::StyleSheetContents> &sheet) noexcept;

    /// @brief Whether the response is CSS or doesn't say
    /// [`X-Content-Type-Options: nosniff`](https://fetch.spec.whatwg.org/#x-content-type-options-header).
    KRYS_NODISCARD bool MimeTypeAllowedByNosniff() const noexcept;

  private:
    /// @brief The encoding the clients are told the text was decoded from.
    KRYS_NODISCARD static utf8_stringview Encoding() noexcept;

    /// @brief Whether the response can be used as a style sheet (see SheetText()).
    KRYS_NODISCARD bool CanUseSheet() const noexcept;

    void DidAddClient(CachedResourceClient &client) noexcept override;

    void FinishLoading(const SharedBuffer *data) noexcept override;

    void DestroyDecodedData() noexcept override;

    void CheckNotify() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedCSSStyleSheet, CachedResource::Type::CSSStyleSheet)
