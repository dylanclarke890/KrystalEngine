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

    if (url.ProtocolIs(u8"ftp") || url.ProtocolIs(u8"http") || url.ProtocolIs(u8"https")
        || url.ProtocolIs(u8"ws") || url.ProtocolIs(u8"wss"))
    {
      dom::USVString result(url.Protocol());
      result += u8"://";
      result += url.HostAndPort();
      return result;
    }

    // The origin of a file URL is left to the implementation: like the other engines, it's `file://`.
    if (url.ProtocolIsFile())
    {
      return dom::USVString(u8"file://");
    }

    // An opaque origin.
    return dom::USVString(u8"null");
  }

  dom::USVString URLDecomposition::Origin() const noexcept
  {
    auto fullURL = FullURL();
    if (!fullURL.IsValid())
    {
      return {};
    }
    return SerializedOrigin(fullURL);
  }

  dom::USVString URLDecomposition::Protocol() const noexcept
  {
    auto fullURL = FullURL();
    if (url::ProtocolIsJavaScript(fullURL.String()))
    {
      return dom::USVString(u8"javascript:");
    }
    dom::USVString result(fullURL.Protocol());
    result += u8':';
    return result;
  }

  void URLDecomposition::Protocol(dom::DOMStringView value) noexcept
  {
    auto copy = FullURL();
    copy.SetProtocol(value);
    SetFullURL(copy);
  }

  dom::USVString URLDecomposition::Username() const noexcept
  {
    return dom::USVString(FullURL().EncodedUser());
  }

  void URLDecomposition::Username(dom::DOMStringView user) noexcept
  {
    auto fullURL = FullURL();
    if (fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }
    fullURL.SetUser(user);
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Password() const noexcept
  {
    return dom::USVString(FullURL().EncodedPassword());
  }

  void URLDecomposition::Password(dom::DOMStringView password) noexcept
  {
    auto fullURL = FullURL();
    if (fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }
    fullURL.SetPassword(password);
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Host() const noexcept
  {
    return dom::USVString(FullURL().HostAndPort());
  }

  void URLDecomposition::Host(dom::DOMStringView value) noexcept
  {
    auto fullURL = FullURL();
    if (value.empty() && !fullURL.ProtocolIsFile() && fullURL.HasSpecialScheme())
    {
      return;
    }

    fullURL.SetHostAndPort(value);

    if (fullURL.IsValid())
    {
      SetFullURL(fullURL);
    }
  }

  dom::USVString URLDecomposition::Hostname() const noexcept
  {
    return dom::USVString(FullURL().Host());
  }

  void URLDecomposition::Hostname(dom::DOMStringView host) noexcept
  {
    auto fullURL = FullURL();
    if (host.empty() && !fullURL.ProtocolIsFile() && fullURL.HasSpecialScheme())
    {
      return;
    }
    fullURL.SetHost(host);
    if (fullURL.IsValid())
    {
      SetFullURL(fullURL);
    }
  }

  dom::USVString URLDecomposition::Port() const noexcept
  {
    auto port = FullURL().Port();
    if (!port)
    {
      return {};
    }
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
    auto fullURL = FullURL();
    if (fullURL.Host().empty() || fullURL.ProtocolIsFile())
    {
      return;
    }
    auto port = ParsePort(value, fullURL.Protocol());
    if (!port)
    {
      return;
    }
    fullURL.SetPort(*port);
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Pathname() const noexcept
  {
    return dom::USVString(FullURL().Path());
  }

  void URLDecomposition::Pathname(dom::DOMStringView value) noexcept
  {
    auto fullURL = FullURL();
    if (fullURL.HasOpaquePath())
    {
      return;
    }
    fullURL.SetPath(value);
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Search() const noexcept
  {
    auto fullURL = FullURL();
    return fullURL.Query().empty() ? dom::USVString()
                                   : dom::USVString(fullURL.QueryWithLeadingQuestionMark());
  }

  void URLDecomposition::Search(dom::DOMStringView value) noexcept
  {
    auto fullURL = FullURL();
    if (value.empty())
    {
      // If the given value is the empty string, set url's query to null.
      fullURL.SetQuery(utf8_stringview());
    }
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
    SetFullURL(fullURL);
  }

  dom::USVString URLDecomposition::Hash() const noexcept
  {
    auto fullURL = FullURL();
    return fullURL.FragmentIdentifier().empty()
             ? dom::USVString()
             : dom::USVString(fullURL.FragmentIdentifierWithLeadingNumberSign());
  }

  void URLDecomposition::Hash(dom::DOMStringView value) noexcept
  {
    auto fullURL = FullURL();
    if (value.empty())
    {
      fullURL.RemoveFragmentIdentifier();
    }
    else
    {
      fullURL.SetFragmentIdentifier(value.starts_with(u8'#') ? value.substr(1) : value);
    }
    SetFullURL(fullURL);
  }
}
