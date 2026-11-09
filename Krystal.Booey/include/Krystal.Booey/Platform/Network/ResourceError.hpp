/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Why a load failed.
  class ResourceError
  {
  public:
    enum class Type : uint8
    {
      Null,
      General,
      AccessControl,
      Cancellation,
      Timeout
    };

  private:
    utf8_string _domain;
    int32 _errorCode {0};
    url::URL _failingURL;
    utf8_string _localizedDescription;
    Type _type {Type::Null};

  public:
    ResourceError() noexcept = default;
    ResourceError(utf8_string domain, int32 errorCode, url::URL failingURL, utf8_string localizedDescription,
                  Type type = Type::General) noexcept
        : _domain(krys::move(domain)), _errorCode(errorCode), _failingURL(krys::move(failingURL)),
          _localizedDescription(krys::move(localizedDescription)), _type(type)
    {
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _type == Type::Null;
    }
    KRYS_NODISCARD bool IsGeneral() const noexcept
    {
      return _type == Type::General;
    }
    KRYS_NODISCARD bool IsAccessControl() const noexcept
    {
      return _type == Type::AccessControl;
    }
    KRYS_NODISCARD bool IsCancellation() const noexcept
    {
      return _type == Type::Cancellation;
    }
    KRYS_NODISCARD bool IsTimeout() const noexcept
    {
      return _type == Type::Timeout;
    }

    KRYS_NODISCARD const utf8_string &Domain() const noexcept
    {
      return _domain;
    }
    KRYS_NODISCARD int32 ErrorCode() const noexcept
    {
      return _errorCode;
    }
    KRYS_NODISCARD const url::URL &FailingURL() const noexcept
    {
      return _failingURL;
    }
    KRYS_NODISCARD const utf8_string &LocalizedDescription() const noexcept
    {
      return _localizedDescription;
    }
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
  };

  /// @brief The domain of errors Booey raises itself, rather than the host's loader.
  inline constexpr utf8_stringview ErrorDomainBooeyInternal = u8"BooeyInternal";
}
