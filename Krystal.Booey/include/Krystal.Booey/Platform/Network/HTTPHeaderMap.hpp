/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief A response's or request's HTTP headers: names compare ignoring ASCII case, and a header given
  /// more than once is one header, its values joined with ", ", as Fetch's header lists combine them.
  /// @see https://fetch.spec.whatwg.org/#concept-header-list
  class HTTPHeaderMap
  {
  private:
    /// @brief The headers in the order they were first given, with their names as first given.
    List<Pair<utf8_string, utf8_string>> _headers;

  public:
    /// @brief The value of the header @p name, or none.
    KRYS_NODISCARD Maybe<utf8_stringview> Get(utf8_stringview name) const noexcept;
    KRYS_NODISCARD bool Contains(utf8_stringview name) const noexcept
    {
      return Get(name).has_value();
    }

    /// @brief Makes @p value the header's only value.
    void Set(utf8_stringview name, utf8_stringview value) noexcept;
    /// @brief Adds @p value to the header's values (after a ", "), or adds the header.
    void Add(utf8_stringview name, utf8_stringview value) noexcept;
    void Remove(utf8_stringview name) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _headers.empty();
    }
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _headers.size();
    }
    KRYS_NODISCARD auto begin() const noexcept
    {
      return _headers.begin();
    }
    KRYS_NODISCARD auto end() const noexcept
    {
      return _headers.end();
    }

  private:
    KRYS_NODISCARD Pair<utf8_string, utf8_string> *Find(utf8_stringview name) noexcept;
  };
}
