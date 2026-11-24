#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Net/Http/HttpParsers.hpp"
#include "Krystal.Text/Encodings/Decode.hpp"
#include "Krystal.Text/Encodings/Encode.hpp"
#include "Krystal.Text/Encodings/UTF.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr text::DecodePolicy SheetDecodePolicy {.OnInvalidSequence = text::EncodingErrorAction::Replace,
                                                    .OnIncompleteSequence =
                                                      text::EncodingErrorAction::Replace,
                                                    .CheckByteOrderMark = text::CheckForBOM::Yes};

    /// @brief Decodes a sheet's bytes as UTF-8, dropping a byte order mark and replacing invalid sequences.
    KRYS_NODISCARD utf8_string DecodeSheet(Span<const uint8> bytes) noexcept
    {
      if (bytes.empty())
      {
        return {};
      }
      auto decoded = text::Decode<text::UTF8, SheetDecodePolicy>(
        utf8_stringview(reinterpret_cast<const char8 *>(bytes.data()), bytes.size()));
      auto encoded = text::Encode<text::UTF8>(decoded.Output);
      return utf8_string(encoded.Output.begin(), encoded.Output.end());
    }
  }

  CachedCSSStyleSheet::CachedCSSStyleSheet(const CachedResourceRequest &request) noexcept
      : CachedResource(CachedResourceRequest(request), Type::CSSStyleSheet), _charset(request.Charset())
  {
  }

  CachedCSSStyleSheet::~CachedCSSStyleSheet() noexcept
  {
    if (_parsedStyleSheetCache)
    {
      _parsedStyleSheetCache->RemovedFromMemoryCache();
    }
  }

#pragma region Text

  utf8_stringview CachedCSSStyleSheet::Encoding() noexcept
  {
    return u8"UTF-8";
  }

  Maybe<utf8_string> CachedCSSStyleSheet::SheetText() const noexcept
  {
    if (!CanUseSheet())
    {
      return null;
    }
    const auto *data = ResourceBuffer();
    if (!data || data->IsEmpty())
    {
      return utf8_string();
    }
    if (_decodedSheetText)
    {
      return _decodedSheetText;
    }
    // The text isn't kept: it can be large, and it's cheap to decode again.
    return DecodeSheet(data->GetSpan());
  }

  bool CachedCSSStyleSheet::MimeTypeAllowedByNosniff() const noexcept
  {
    return !net::http::IsNosniffContentTypeOptions(Response().HttpHeaderField(u8"X-Content-Type-Options"))
           || text::IsASCIICaselessEqual(Response().MimeType(), u8"text/css");
  }

  // https://html.spec.whatwg.org/#link-type-stylesheet:process-the-linked-resource
  bool CachedCSSStyleSheet::CanUseSheet() const noexcept
  {
    if (ErrorOccurred())
    {
      return false;
    }

    // An unsuccessful HTTP response is no style sheet. (A host that gives no status, 0, isn't saying it
    // failed.)
    // https://html.spec.whatwg.org/#fetching-and-processing-a-resource-from-a-link-element:processresponseconsumebody
    if (Response().Url().ProtocolIsInHttpFamily() && Response().HttpStatusCode()
        && !Response().IsSuccessful())
    {
      return false;
    }

    if (!MimeTypeAllowedByNosniff())
    {
      return false;
    }

    // The type the response declared, not one sniffed from the content, must be CSS, missing or unknown.
    const auto mimeType = Response().MimeType();
    return mimeType.empty() || text::IsASCIICaselessEqual(mimeType, u8"text/css")
           || text::IsASCIICaselessEqual(mimeType, u8"application/x-unknown-content-type");
  }

#pragma endregion

#pragma region Loading

  void CachedCSSStyleSheet::DidAddClient(CachedResourceClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedStyleSheetClient::ExpectedType());
    // The client is registered before it's given the sheet, as being given the sheet can destroy it.
    CachedResource::DidAddClient(client);

    if (!IsLoading())
    {
      Downcast<CachedStyleSheetClient>(client).SetCSSStyleSheet(GetResourceRequest().Url().String(),
                                                                Response().Url(), Encoding(), this);
    }
  }

  void CachedCSSStyleSheet::FinishLoading(const SharedBuffer *data) noexcept
  {
    if (data)
    {
      SetEncodedSize(static_cast<uint32>(data->Size()));
      // Kept while the clients are told below, as they read it through SheetText().
      _decodedSheetText = DecodeSheet(data->GetSpan());
      _data = ShareRefPtr(const_cast<SharedBuffer *>(data));
    }
    else
    {
      _data = nullptr;
      SetEncodedSize(0);
    }
    SetLoading(false);
    CheckNotify();
    _decodedSheetText = null;
  }

  void CachedCSSStyleSheet::CheckNotify() noexcept
  {
    if (IsLoading())
    {
      return;
    }

    CachedResourceClientWalker<CachedStyleSheetClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->SetCSSStyleSheet(GetResourceRequest().Url().String(), Response().Url(), Encoding(), this);
    }
  }

#pragma endregion

#pragma region Parsed sheet

  RefPtr<css::StyleSheetContents>
    CachedCSSStyleSheet::RestoreParsedStyleSheet(const css::ParserContext &context) noexcept
  {
    if (!_parsedStyleSheetCache)
    {
      return nullptr;
    }
    if (!_parsedStyleSheetCache->SubresourcesAllowReuse())
    {
      _parsedStyleSheetCache->RemovedFromMemoryCache();
      _parsedStyleSheetCache = nullptr;
      return nullptr;
    }

    krys_debug_assert(_parsedStyleSheetCache->IsCacheable());
    krys_debug_assert(_parsedStyleSheetCache->IsInMemoryCache());

    // A sheet parsed with another context could have parsed differently.
    if (_parsedStyleSheetCache->GetParserContext() != context)
    {
      return nullptr;
    }

    DidAccessDecodedData(MonotonicTime::Now());
    return _parsedStyleSheetCache;
  }

  void CachedCSSStyleSheet::SaveParsedStyleSheet(const Ref<css::StyleSheetContents> &sheet) noexcept
  {
    krys_debug_assert(sheet->IsCacheable());

    if (_parsedStyleSheetCache)
    {
      _parsedStyleSheetCache->RemovedFromMemoryCache();
    }
    _parsedStyleSheetCache = sheet;
    sheet->AddedToMemoryCache();

    SetDecodedSize(sheet->EstimatedSizeInBytes());
  }

  void CachedCSSStyleSheet::DestroyDecodedData() noexcept
  {
    if (!_parsedStyleSheetCache)
    {
      return;
    }

    _parsedStyleSheetCache->RemovedFromMemoryCache();
    _parsedStyleSheetCache = nullptr;
    SetDecodedSize(0);
  }

#pragma endregion
}
