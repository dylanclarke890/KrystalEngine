/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
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
  // FIXME: Encodings other than UTF-8 (the request's charset, the response's and `@charset`).
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
