#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
  struct ParserContext;
}

namespace krys::boo
{
  /// @brief A cached style sheet: the text a load brought, and the sheet parsed from it, kept for reuse.
  // FIXME: Encodings other than UTF-8 (the request's charset, the response's and `@charset`), HTTP status and
  // `X-Content-Type-Options: nosniff` checks, and revalidating the parsed sheet's subresources, once there
  // are network loads.
  class CachedCSSStyleSheet final : public CachedResource
  {
  private:
    utf8_string _charset;
    utf8_string _decodedSheetText;
    bool _hasDecodedSheetText {false};
    RefPtr<css::StyleSheetContents> _parsedStyleSheetCache;

  public:
    explicit CachedCSSStyleSheet(CachedResourceRequest &&request) noexcept;
    ~CachedCSSStyleSheet() noexcept override;

    enum class MIMETypeCheckHint : bool
    {
      Strict,
      Lax
    };
    /// @brief The sheet's text, or null if it can't be used as a sheet (a load error, or a type that isn't
    /// CSS when @p mimeTypeCheckHint is strict).
    KRYS_NODISCARD Maybe<utf8_string>
      SheetText(MIMETypeCheckHint mimeTypeCheckHint = MIMETypeCheckHint::Strict,
                bool *hasValidMIMEType = nullptr, bool *hasHTTPStatusOK = nullptr) const noexcept;

    KRYS_NODISCARD RefPtr<css::StyleSheetContents>
      RestoreParsedStyleSheet(const css::ParserContext &context) noexcept;
    void SaveParsedStyleSheet(Ref<css::StyleSheetContents> &&sheet) noexcept;

    KRYS_NODISCARD bool MimeTypeAllowedByNosniff() const noexcept;

    /// @brief The encoding the sheet's text is decoded from.
    KRYS_NODISCARD utf8_stringview Encoding() const noexcept;

  private:
    KRYS_NODISCARD utf8_string ResponseMIMEType() const noexcept;
    KRYS_NODISCARD bool CanUseSheet(MIMETypeCheckHint mimeTypeCheckHint, bool *hasValidMIMEType,
                                    bool *hasHTTPStatusOK) const noexcept;

    void DidAddClient(CachedResourceClient &client) noexcept override;
    void FinishLoading(const SharedBuffer *data) noexcept override;
    void DestroyDecodedData() noexcept override;
    void CheckNotify() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedCSSStyleSheet, CachedResource::Type::CSSStyleSheet)
