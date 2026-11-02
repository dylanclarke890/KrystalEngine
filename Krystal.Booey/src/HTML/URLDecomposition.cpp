#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <limits>

namespace krys::boo::html
{
  dom::USVString URLDecomposition::SerializedOrigin(const url::URL &url) noexcept
  {
    // https://url.spec.whatwg.org/#concept-url-origin
    if (url.ProtocolIsBlob())
    {
      // Let pathURL be the result of parsing the result of URL path serializing url. If pathURL's scheme is
      // "http" or "https", return pathURL's origin.
      url::URL pathURL(url.Path());
      if (pathURL.IsValid() && (pathURL.ProtocolIs(u8"http") || pathURL.ProtocolIs(u8"https")))
      {
        return SerializedOrigin(pathURL);
      }
      return dom::USVString(u8"null");
    }

    // https://html.spec.whatwg.org/#ascii-serialisation-of-an-origin
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

  // The steps are those of HTMLHyperlinkElementUtils, whose url is FullURL(): an invalid URL is a null one,
  // and reinitializing the url is computing it again.

  dom::USVString URLDecomposition::Origin() const noexcept
  {
    // 1. Reinitialize url.
    auto fullURL = FullURL();

    // 2. If this's url is null, return the empty string.
    if (!fullURL.IsValid())
    {
      return {};
    }

    // 3. Return the serialization of this's url's origin.
    return SerializedOrigin(fullURL);
  }

  dom::USVString URLDecomposition::Protocol() const noexcept
  {
    // 1. Reinitialize url.
    auto fullURL = FullURL();
    if (url::ProtocolIsJavaScript(fullURL.String()))
    {
      return dom::USVString(u8"javascript:");
    }

    // 2. If this's url is null, return ":".
    // 3. Return this's url's scheme, followed by ":".
    // (An invalid URL's scheme is empty.)
    dom::USVString result(fullURL.Protocol());
    result += u8':';
    return result;
  }

  void URLDecomposition::Protocol(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    auto copy = FullURL();

    // 2. If this's url is null, then return.
    if (!copy.IsValid())
    {
      return;
    }

    // 3. Basic URL parse the given value, followed by ":", with this's url as url and scheme start state as
    //    state override.
    copy.SetProtocol(value);

    // 4. Update href.
    SetFullURL(copy);
  }

  dom::USVString URLDecomposition::Username() const noexcept
  {
    // 1. Reinitialize url.
    // 2. If this's url is null, return the empty string.
    // 3. Return this's url's username.
    return dom::USVString(FullURL().EncodedUser());
  }

  void URLDecomposition::Username(dom::DOMStringView user) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullURL.IsValid() || fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }

    // 4. Set the username, given url and the given value.
    fullURL.SetUser(user);

    // 5. Update href.
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Password() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, then return the empty string.
    // 4. Return url's password.
    return dom::USVString(FullURL().EncodedPassword());
  }

  void URLDecomposition::Password(dom::DOMStringView password) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullURL.IsValid() || fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }

    // 4. Set the password, given url and the given value.
    fullURL.SetPassword(password);

    // 5. Update href.
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Host() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's host is null, return the empty string.
    // 4. If url's port is null, return url's host, serialized.
    // 5. Return url's host, serialized, followed by ":" and url's port, serialized.
    return dom::USVString(FullURL().HostAndPort());
  }

  void URLDecomposition::Host(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullURL.IsValid() || fullURL.HasOpaquePath())
    {
      return;
    }

    // 4. Basic URL parse the given value, with url as url and host state as state override.
    // (An empty host fails for a special URL other than a file URL.)
    if (value.empty() && !fullURL.ProtocolIsFile() && fullURL.HasSpecialScheme())
    {
      return;
    }
    fullURL.SetHostAndPort(value);

    // 5. Update href.
    if (fullURL.IsValid())
    {
      SetFullURL(fullURL);
    }
  }

  dom::USVString URLDecomposition::Hostname() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's host is null, return the empty string.
    // 4. Return url's host, serialized.
    return dom::USVString(FullURL().Host());
  }

  void URLDecomposition::Hostname(dom::DOMStringView host) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullURL.IsValid() || fullURL.HasOpaquePath())
    {
      return;
    }

    // 4. Basic URL parse the given value, with url as url and hostname state as state override.
    // (An empty host fails for a special URL other than a file URL.)
    if (host.empty() && !fullURL.ProtocolIsFile() && fullURL.HasSpecialScheme())
    {
      return;
    }
    fullURL.SetHost(host);

    // 5. Update href.
    if (fullURL.IsValid())
    {
      SetFullURL(fullURL);
    }
  }

  dom::USVString URLDecomposition::Port() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url or url's port is null, return the empty string.
    auto port = FullURL().Port();
    if (!port)
    {
      return {};
    }

    // 4. Return url's port, serialized.
    return dom::USVString(utf8_string(reinterpret_cast<const char8_t *>(std::to_string(*port).c_str())));
  }

  Maybe<Maybe<uint16>> URLDecomposition::ParsePort(dom::DOMStringView string,
                                                   dom::DOMStringView protocol) noexcept
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
    if (!foundDigit || url::IsDefaultPortForProtocol(static_cast<uint16>(port), protocol))
    {
      return Maybe<uint16>();
    }
    return Maybe<uint16>(static_cast<uint16>(port));
  }

  void URLDecomposition::Port(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url cannot have a username/password/port, then return.
    if (!fullURL.IsValid() || fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }

    // 4. If the given value is the empty string, then set url's port to null.
    // 5. Otherwise, basic URL parse the given value, with url as url and port state as state override.
    // (ParsePort gives a null port for the empty string.)
    auto port = ParsePort(value, fullURL.Protocol());
    if (!port)
    {
      return;
    }
    fullURL.SetPort(*port);

    // 6. Update href.
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Pathname() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, then return the empty string.
    // 4. Return the result of URL path serializing url.
    return dom::USVString(FullURL().Path());
  }

  void URLDecomposition::Pathname(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null or url has an opaque path, then return.
    if (!fullURL.IsValid() || fullURL.HasOpaquePath())
    {
      return;
    }

    // 4. Set url's path to the empty list.
    // 5. Basic URL parse the given value, with url as url and path start state as state override.
    fullURL.SetPath(value);

    // 6. Update href.
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Search() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, or url's query is either null or the empty string, return the empty string.
    // 4. Return "?", followed by url's query.
    auto fullURL = FullURL();
    return fullURL.Query().empty() ? dom::USVString()
                                   : dom::USVString(fullURL.QueryWithLeadingQuestionMark());
  }

  void URLDecomposition::Search(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null, terminate these steps.
    if (!fullURL.IsValid())
    {
      return;
    }

    // 4. If the given value is the empty string, set url's query to null.
    if (value.empty())
    {
      fullURL.SetQuery(utf8_stringview());
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
      fullURL.SetQuery(query);
    }

    // 6. Update href.
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Hash() const noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    // 3. If url is null, or url's fragment is either null or the empty string, return the empty string.
    // 4. Return "#", followed by url's fragment.
    auto fullURL = FullURL();
    return fullURL.FragmentIdentifier().empty()
             ? dom::USVString()
             : dom::USVString(fullURL.FragmentIdentifierWithLeadingNumberSign());
  }

  void URLDecomposition::Hash(dom::DOMStringView value) noexcept
  {
    // 1. Reinitialize url.
    // 2. Let url be this's url.
    auto fullURL = FullURL();

    // 3. If url is null, then return.
    if (!fullURL.IsValid())
    {
      return;
    }

    // 4. If the given value is the empty string, set url's fragment to null.
    if (value.empty())
    {
      fullURL.RemoveFragmentIdentifier();
    }
    // 5. Otherwise:
    // 5a. Let input be the given value with a single leading "#" removed, if any.
    // 5b. Set url's fragment to the empty string.
    // 5c. Basic URL parse input, with url as url and fragment state as state override.
    else
    {
      fullURL.SetFragmentIdentifier(value.starts_with(u8'#') ? value.substr(1) : value);
    }

    // 6. Update href.
    SetFullURL(fullURL);
  }
}
