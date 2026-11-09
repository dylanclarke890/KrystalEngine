/*
 * Copyright (C) 2012 Google, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The initiator types a load can report.
  struct CachedResourceRequestInitiatorTypes
  {
    static constexpr utf8_stringview CSS = u8"css";
    static constexpr utf8_stringview Icon = u8"icon";
    static constexpr utf8_stringview XMLHTTPRequest = u8"xmlhttprequest";
  };

  /// @brief A request for a cached resource: the resource request, how to load it and who asked.
  class CachedResourceRequest
  {
  private:
    ResourceRequest _resourceRequest;
    ResourceLoaderOptions _options;
    utf8_string _initiatorType;
    utf8_string _charset;
    bool _ignoreForRequestCount {false};

  public:
    explicit CachedResourceRequest(ResourceRequest &&request,
                                   const ResourceLoaderOptions &options = {}) noexcept
        : _resourceRequest(krys::move(request)), _options(options)
    {
    }

    KRYS_NODISCARD const ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD ResourceRequest &GetResourceRequest() noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD ResourceRequest &&ReleaseResourceRequest() noexcept
    {
      return krys::move(_resourceRequest);
    }

    KRYS_NODISCARD const ResourceLoaderOptions &Options() const noexcept
    {
      return _options;
    }
    KRYS_NODISCARD bool AllowsCaching() const noexcept
    {
      return _options.Caching == CachingPolicy::AllowCaching
             || _options.Caching == CachingPolicy::AllowCachingMainResourcePrefetch;
    }

    KRYS_NODISCARD const utf8_string &InitiatorType() const noexcept
    {
      return _initiatorType;
    }
    void SetInitiatorType(utf8_stringview initiatorType) noexcept
    {
      _initiatorType = utf8_string(initiatorType);
    }

    /// @brief The encoding a text resource is decoded from unless the resource says otherwise.
    KRYS_NODISCARD const utf8_string &Charset() const noexcept
    {
      return _charset;
    }
    void SetCharset(utf8_string charset) noexcept
    {
      _charset = krys::move(charset);
    }

    KRYS_NODISCARD bool IgnoreForRequestCount() const noexcept
    {
      return _ignoreForRequestCount;
    }
    void SetIgnoreForRequestCount(bool ignoreForRequestCount) noexcept
    {
      _ignoreForRequestCount = ignoreForRequestCount;
    }
  };
}
