/*
 * Copyright (C) 2014-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Mixins/UrlDecomposition.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Net/Url.hpp"
#include <limits>

namespace krys::boo::html
{
  dom::USVString UrlDecomposition::SerializedOrigin(const net::Url &url) noexcept
  {
    // https://url.spec.whatwg.org/#concept-url-origin
    if (url.ProtocolIsBlob())
    {
      // Let pathURL be the result of parsing the result of URL path serializing url. If pathURL's scheme is
      // "http" or "https", return pathURL's origin.
      net::Url pathUrl(url.Path());
      if (pathUrl.IsValid() && (pathUrl.ProtocolIs(u8"http") || pathUrl.ProtocolIs(u8"https")))
      {
        return SerializedOrigin(pathUrl);
      }
      return dom::USVString(u8"null");
    }

    // https://html.spec.whatwg.org/multipage/browsers.html#ascii-serialisation-of-an-origin
    if (url.ProtocolIs(u8"ftp") || url.ProtocolIs(u8"http") || url.ProtocolIs(u8"https")
        || url.ProtocolIs(u8"ws") || url.ProtocolIs(u8"wss"))
    {
      // 2. Let result be origin's scheme.
      dom::USVString result(url.Protocol());
      // 3. Append "://" to result.
      result += u8"://";
      // 4. Append origin's host, serialized, to result.
      // 5. If origin's port is non-null, append a U+003A COLON character (:), and origin's port, serialized,
      //    to result.
      result += url.HostAndPort();
      // 6. Return result.
      return result;
    }

    // The origin of a file URL is left to the implementation: like the other engines, it's `file://`.
    if (url.ProtocolIsFile())
    {
      return dom::USVString(u8"file://");
    }

    // 1. If origin is an opaque origin, then return "null".
    return dom::USVString(u8"null");
  }

  // The steps are those of HTMLHyperlinkElementUtils, whose url is FullUrl(): an invalid URL is a null one,
  // and reinitializing the url is computing it again.

  dom::USVString UrlDecomposition::Origin() const noexcept
  {
    // 1. Reinitialize url.
    auto fullUrl = FullUrl();

    // 2. If this's url is null, return the empty string.
    if (!fullUrl.IsValid())
    {
      return {};
    }

    // 3. Return the serialization of this's url's origin.
    return SerializedOrigin(fullUrl);
  }

  dom::USVString UrlDecomposition::Protocol() const noexcept
  {
    // 1. Reinitialize url.
    auto fullUrl = FullUrl();
    if (net::ProtocolIsJavaScript(fullUrl.String()))
    {
      return dom::USVString(u8"javascript:");
    }

    // 2. If this's url is null, return ":".
    // 3. Return this's url's scheme, followed by ":".
    // (An invalid URL's scheme is empty.)
    dom::USVString result(fullUrl.Protocol());
    result += u8':';
    return result;
  }

  void UrlDecomposition::Protocol(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    auto copy = FullUrl();

    // 2. If this's url is null, then return.
    if (!copy.IsValid())
    {
      return;
    }

    // 3. Basic URL parse the given value, followed by ":", with this's url as url and scheme start state as
    //    state override.
    copy.SetProtocol(value);

    // 4. Update href.
    SetFullUrl(copy);
  }

  dom::USVString UrlDecomposition::Username() const noexcept
  {
    // 1. Reinitialize url.
    // 2. If this's url is null, return the empty string.
    // 3. Return this's url's username.
    return dom::USVString(FullUrl().EncodedUser());
  }

  void UrlDecomposition::Username(DOMStringView user) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullUrl.IsValid() || fullUrl.Host().empty() || fullUrl.ProtocolIsFile())
    {
      return;
    }

    // 4. Set the username, given url and the given value.
    fullUrl.SetUser(user);

    // 5. Update href.
    SetFullUrl(fullUrl);
  }

  dom::USVString UrlDecomposition::Password() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, then return the empty string.
    // 4. Return url's password.
    return dom::USVString(FullUrl().EncodedPassword());
  }

  void UrlDecomposition::Password(DOMStringView password) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullUrl.IsValid() || fullUrl.Host().empty() || fullUrl.ProtocolIsFile())
    {
      return;
    }

    // 4. Set the password, given url and the given value.
    fullUrl.SetPassword(password);

    // 5. Update href.
    SetFullUrl(fullUrl);
  }

  dom::USVString UrlDecomposition::Host() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's host is null, return the empty string.
    // 4. If url's port is null, return url's host, serialized.
    // 5. Return url's host, serialized, followed by ":" and url's port, serialized.
    return dom::USVString(FullUrl().HostAndPort());
  }

  void UrlDecomposition::Host(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullUrl.IsValid() || fullUrl.HasOpaquePath())
    {
      return;
    }

    // 4. Basic URL parse the given value, with url as url and host state as state override.
    // (An empty host fails for a special URL other than a file URL.)
    if (value.empty() && !fullUrl.ProtocolIsFile() && fullUrl.HasSpecialScheme())
    {
      return;
    }
    fullUrl.SetHostAndPort(value);

    // 5. Update href.
    if (fullUrl.IsValid())
    {
      SetFullUrl(fullUrl);
    }
  }

  dom::USVString UrlDecomposition::Hostname() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's host is null, return the empty string.
    // 4. Return url's host, serialized.
    return dom::USVString(FullUrl().Host());
  }

  void UrlDecomposition::Hostname(DOMStringView host) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullUrl.IsValid() || fullUrl.HasOpaquePath())
    {
      return;
    }

    // 4. Basic URL parse the given value, with url as url and hostname state as state override.
    // (An empty host fails for a special URL other than a file URL.)
    if (host.empty() && !fullUrl.ProtocolIsFile() && fullUrl.HasSpecialScheme())
    {
      return;
    }
    fullUrl.SetHost(host);

    // 5. Update href.
    if (fullUrl.IsValid())
    {
      SetFullUrl(fullUrl);
    }
  }

  dom::USVString UrlDecomposition::Port() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's port is null, return the empty string.
    auto port = FullUrl().Port();
    if (!port)
    {
      return {};
    }

    // 4. Return url's port, serialized.
    return dom::USVString(utf8_string(reinterpret_cast<const char8_t *>(std::to_string(*port).c_str())));
  }

  Maybe<Maybe<uint16>> UrlDecomposition::ParsePort(DOMStringView string, DOMStringView protocol) noexcept
  {
    // https://url.spec.whatwg.org/#port-state with state override given.
    uint32 port {0};
    bool foundDigit = false;
    for (auto c : string)
    {
      // https://infra.spec.whatwg.org/#ascii-tab-or-newline
      if (c == u8'\t' || c == u8'\n' || c == u8'\r')
      {
        continue;
      }
      if (text::IsASCIIDigit(c))
      {
        port = port * 10 + static_cast<uint32>(c - u8'0');
        foundDigit = true;
        if (port > std::numeric_limits<uint16>::max())
        {
          return std::nullopt;
        }
        continue;
      }
      if (!foundDigit)
      {
        return std::nullopt;
      }
      break;
    }
    if (!foundDigit || net::IsDefaultPortForProtocol(static_cast<uint16>(port), protocol))
    {
      return Maybe<uint16>();
    }
    return Maybe<uint16>(static_cast<uint16>(port));
  }

  void UrlDecomposition::Port(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullUrl.IsValid() || fullUrl.Host().empty() || fullUrl.ProtocolIsFile())
    {
      return;
    }

    // 4. If the given value is the empty string, then set url's port to null.
    // 5. Otherwise, basic URL parse the given value, with url as url and port state as state override.
    // (ParsePort gives a null port for the empty string.)
    auto port = ParsePort(value, fullUrl.Protocol());
    if (!port)
    {
      return;
    }
    fullUrl.SetPort(*port);

    // 6. Update href.
    SetFullUrl(fullUrl);
  }

  dom::USVString UrlDecomposition::Pathname() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, then return the empty string.
    // 4. Return the result of URL path serializing url.
    return dom::USVString(FullUrl().Path());
  }

  void UrlDecomposition::Pathname(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullUrl.IsValid() || fullUrl.HasOpaquePath())
    {
      return;
    }

    // 4. Set url's path to the empty list.
    // 5. Basic URL parse the given value, with url as url and path start state as state override.
    fullUrl.SetPath(value);

    // 6. Update href.
    SetFullUrl(fullUrl);
  }

  dom::USVString UrlDecomposition::Search() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, or url's query is either null or the empty string, return the empty string.
    // 4. Return "?", followed by url's query.
    auto fullUrl = FullUrl();
    return fullUrl.Query().empty() ? dom::USVString()
                                   : dom::USVString(fullUrl.QueryWithLeadingQuestionMark());
  }

  void UrlDecomposition::Search(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null, terminate these steps.
    if (!fullUrl.IsValid())
    {
      return;
    }

    // 4. If the given value is the empty string, set url's query to null.
    if (value.empty())
    {
      fullUrl.SetQuery(utf8_stringview());
    }
    // 5. Otherwise:
    // 5a. Let input be the given value with a single leading "?" removed, if any.
    // 5b. Set url's query to the empty string.
    // 5c. Basic URL parse input, with url as url and query state as state override.
    else
    {
      // Make sure that '#' in the query does not leak to the hash.
      utf8_string query;
      query.reserve(value.size());
      for (auto c : value)
      {
        if (c == u8'#')
        {
          query += u8"%23";
        }
        else
        {
          query += c;
        }
      }
      fullUrl.SetQuery(query);
    }

    // 6. Update href.
    SetFullUrl(fullUrl);
  }

  dom::USVString UrlDecomposition::Hash() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, or url's fragment is either null or the empty string, return the empty string.
    // 4. Return "#", followed by url's fragment.
    auto fullUrl = FullUrl();
    return fullUrl.FragmentIdentifier().empty()
             ? dom::USVString()
             : dom::USVString(fullUrl.FragmentIdentifierWithLeadingNumberSign());
  }

  void UrlDecomposition::Hash(DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullUrl = FullUrl();

    // 3. If url is null, then return.
    if (!fullUrl.IsValid())
    {
      return;
    }

    // 4. If the given value is the empty string, set url's fragment to null.
    if (value.empty())
    {
      fullUrl.RemoveFragmentIdentifier();
    }
    // 5. Otherwise:
    // 5a. Let input be the given value with a single leading "#" removed, if any.
    // 5b. Set url's fragment to the empty string.
    // 5c. Basic URL parse input, with url as url and fragment state as state override.
    else
    {
      fullUrl.SetFragmentIdentifier(value.starts_with(u8'#') ? value.substr(1) : value);
    }

    // 6. Update href.
    SetFullUrl(fullUrl);
  }
}
