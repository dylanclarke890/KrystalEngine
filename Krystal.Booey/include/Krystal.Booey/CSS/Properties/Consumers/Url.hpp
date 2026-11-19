/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::parse_property
{
  enum class AllowedUrlModifiers
  {
    None = 0,
    CrossOrigin = 1 << 0,
    Integrity = 1 << 1,
    ReferrerPolicy = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::parse_property::AllowedUrlModifiers, 4uz)

namespace krys::boo::css
{
  class TokenRange;
  class Value;

  struct CSSUrl;
  struct PropertyParserState;

  namespace parse_property
  {
    // MARK: <url>
    // https://drafts.csswg.org/css-values/#urls

    KRYS_NODISCARD Maybe<CSSUrl> ConsumeUrlRaw(TokenRange &tokens, PropertyParserState &state,
                                               AllowedUrlModifiers allowed) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeUrl(TokenRange &tokens, PropertyParserState &state,
                                            AllowedUrlModifiers allowed) noexcept;
  }
}
