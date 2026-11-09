/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // MARK: - Serialisation

  void Serialiser<CSSURL>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                      const CSSURL &value) noexcept
  {
    // https://drafts.csswg.org/cssom/#serialize-a-url

    builder.Append(ToLower(ValueId::Url));
    builder.Append(u8'(');

    // FIXME: Upstream can serialise the resolved URL instead (SerialisationContext's replacement URLs and
    // shouldUseResolvedURLInCSSText, used for web archives); the specified URL is upstream's default.
    SerialiseString(builder, context, value.Specified);

    if (value.Modifiers.CrossOrigin)
    {
      builder.Append(u8' ');
      Serialise(builder, context, *value.Modifiers.CrossOrigin);
    }
    if (value.Modifiers.Integrity)
    {
      builder.Append(u8' ');
      Serialise(builder, context, *value.Modifiers.Integrity);
    }
    if (value.Modifiers.ReferrerPolicy)
    {
      builder.Append(u8' ');
      Serialise(builder, context, *value.Modifiers.ReferrerPolicy);
    }

    builder.Append(u8')');
  }

  // MARK: Operations

  namespace
  {
    CSSURL CompleteURL(const CSSOMString &string, const url::URL &baseURL) noexcept
    {
      if (string.empty() || string.starts_with(u8'#'))
      {
        return CSSURL {.Specified = string, .Resolved = url::URL {string}, .Modifiers = {}};
      }
      if (baseURL.IsNull() || baseURL.IsAboutBlank())
      {
        return CSSURL {.Specified = string, .Resolved = url::URL {}, .Modifiers = {}};
      }
      return CSSURL {.Specified = string, .Resolved = url::URL {baseURL, string}, .Modifiers = {}};
    }
  }

  Maybe<CSSURL> CompleteURL(const CSSOMString &string, const ParserContext &context) noexcept
  {
    // Upstream also rejects non-`data:` URLs in WebVTT mode, which isn't supported.
    return CompleteURL(string, context.BaseURL);
  }

  CSSURL Resolve(CSSURL &&url) noexcept
  {
    if (url.Resolved.IsNull())
    {
      url.Resolved = url::URL {url.Specified};
    }
    return krys::move(url);
  }

  bool MayDependOnBaseURL(const CSSURL &url) noexcept
  {
    return !url.Specified.empty() && !url.Specified.starts_with(u8'#') && !url.Resolved.ProtocolIsData();
  }
}
