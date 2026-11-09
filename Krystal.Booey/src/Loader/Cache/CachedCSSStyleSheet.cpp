/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
    Copyright (C) 2004-2017 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Booey/Platform/Network/HTTPParsers.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr text::DecodePolicy SheetDecodePolicy {.OnInvalidSequence = text::EncodingErrorAction::Replace,
                                                    .OnIncompleteSequence =
                                                      text::EncodingErrorAction::Replace,
                                                    .CheckByteOrderMark = text::CheckForBOM(true)};

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

  CachedCSSStyleSheet::CachedCSSStyleSheet(CachedResourceRequest &&request) noexcept
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

  void CachedCSSStyleSheet::DidAddClient(CachedResourceClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedStyleSheetClient::ExpectedType());
    // CachedResource::didAddClient() must be before setCSSStyleSheet(), because setCSSStyleSheet() may
    // cause the client to be destroyed.
    CachedResource::DidAddClient(client);

    if (!IsLoading())
    {
      Downcast<CachedStyleSheetClient>(client).SetCSSStyleSheet(GetResourceRequest().URL().String(),
                                                                Response().URL(), Encoding(), this);
    }
  }

  utf8_stringview CachedCSSStyleSheet::Encoding() const noexcept
  {
    // Style sheets are UTF-8 only, whatever the request, the response or `@charset` say (see DIFFERENCES.md).
    return u8"UTF-8";
  }

  Maybe<utf8_string> CachedCSSStyleSheet::SheetText(MIMETypeCheckHint mimeTypeCheckHint,
                                                    bool *hasValidMIMEType,
                                                    bool *hasHTTPStatusOK) const noexcept
  {
    // Ensure hasValidMIMEType and hasHTTPStatusOK always get set (even if the data is null or empty) — which
    // in turn ensures that if the MIME type isn't text/css or the HTTP status isn't an OK status, we never
    // load the resource. https://html.spec.whatwg.org/#link-type-stylesheet:process-the-linked-resource
    if (!CanUseSheet(mimeTypeCheckHint, hasValidMIMEType, hasHTTPStatusOK))
    {
      return null;
    }
    auto *data = ResourceBuffer();
    if (!data || data->IsEmpty())
    {
      return utf8_string();
    }

    if (_hasDecodedSheetText)
    {
      return _decodedSheetText;
    }

    // Don't cache the decoded text, regenerating is cheap and it can use quite a bit of memory.
    return DecodeSheet(data->GetSpan());
  }

  void CachedCSSStyleSheet::FinishLoading(const SharedBuffer *data) noexcept
  {
    if (data)
    {
      SetEncodedSize(static_cast<uint32>(data->Size()));
      // Decode the data to keep the sheet text around during CheckNotify().
      _decodedSheetText = DecodeSheet(data->GetSpan());
      _hasDecodedSheetText = true;
      _data = ShareRefPtr(const_cast<SharedBuffer *>(data));
    }
    else
    {
      _data = nullptr;
      SetEncodedSize(0);
    }
    SetLoading(false);
    CheckNotify();
    // Clear the decoded text as it is unlikely to be needed immediately again and is cheap to regenerate.
    _decodedSheetText.clear();
    _hasDecodedSheetText = false;
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
      client->SetCSSStyleSheet(GetResourceRequest().URL().String(), Response().URL(), Encoding(), this);
    }
  }

  utf8_string CachedCSSStyleSheet::ResponseMIMEType() const noexcept
  {
    return Response().MimeType();
  }

  bool CachedCSSStyleSheet::MimeTypeAllowedByNosniff() const noexcept
  {
    // A nosniff response is a style sheet only if it says it's one.
    return !IsNosniffContentTypeOptions(Response().HTTPHeaderField(u8"X-Content-Type-Options"))
           || text::IsASCIICaselessEqual(ResponseMIMEType(), utf8_stringview(u8"text/css"));
  }

  bool CachedCSSStyleSheet::CanUseSheet(MIMETypeCheckHint mimeTypeCheckHint, bool *hasValidMIMEType,
                                        bool *hasHTTPStatusOK) const noexcept
  {
    if (ErrorOccurred())
    {
      return false;
    }

    // An unsuccessful HTTP response is no style sheet. (A host that gives no status, 0, isn't saying it
    // failed.)
    // https://html.spec.whatwg.org/#fetching-and-processing-a-resource-from-a-link-element:processresponseconsumebody
    if (Response().URL().ProtocolIsInHTTPFamily() && Response().HTTPStatusCode()
        && !Response().IsSuccessful())
    {
      if (hasHTTPStatusOK)
      {
        *hasHTTPStatusOK = false;
      }
      return false;
    }

    if (!MimeTypeAllowedByNosniff())
    {
      if (hasValidMIMEType)
      {
        *hasValidMIMEType = false;
      }
      return false;
    }

    if (mimeTypeCheckHint == MIMETypeCheckHint::Lax)
    {
      return true;
    }

    // This check exactly matches Firefox. Note that we grab the Content-Type header directly because we want
    // to see what the value is BEFORE content sniffing. Firefox does this by setting a "type hint" on the
    // channel. This implementation should be observationally equivalent.
    //
    // This code defaults to allowing the stylesheet for non-HTTP protocols so folks can use standards mode
    // for local HTML documents.
    // FIXME: Types that aren't valid content types (isValidContentType) are allowed too, once there's a
    // content type parser.
    auto mimeType = ResponseMIMEType();
    bool typeOK =
      mimeType.empty() || text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"text/css"))
      || text::IsASCIICaselessEqual(mimeType, utf8_stringview(u8"application/x-unknown-content-type"));
    if (hasValidMIMEType)
    {
      *hasValidMIMEType = typeOK;
    }
    return typeOK;
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

    // Contexts must be identical so we know we would get the same exact result if we parsed again.
    if (_parsedStyleSheetCache->GetParserContext() != context)
    {
      return nullptr;
    }

    DidAccessDecodedData(MonotonicTime::Now());

    return _parsedStyleSheetCache;
  }

  void CachedCSSStyleSheet::SaveParsedStyleSheet(Ref<css::StyleSheetContents> &&sheet) noexcept
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
}
