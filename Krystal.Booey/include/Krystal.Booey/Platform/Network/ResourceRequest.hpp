/*
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief What's asked of the loader: the URL to load, and the HTTP method.
  class ResourceRequest
  {
  private:
    url::URL _url;
    utf8_string _httpMethod {u8"GET"};

  public:
    ResourceRequest() noexcept = default;
    explicit ResourceRequest(url::URL url) noexcept : _url(krys::move(url))
    {
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }
    void SetURL(url::URL url) noexcept
    {
      _url = krys::move(url);
    }
    void RemoveFragmentIdentifier() noexcept
    {
      _url.RemoveFragmentIdentifier();
    }

    KRYS_NODISCARD const utf8_string &HTTPMethod() const noexcept
    {
      return _httpMethod;
    }
    void SetHTTPMethod(utf8_string method) noexcept
    {
      _httpMethod = krys::move(method);
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _url.IsNull();
    }
  };
}
