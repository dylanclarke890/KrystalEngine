#include "Krystal.Booey/Platform/Pasteboard/SelectionData.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD utf8_string ReplaceNonBreakingSpaceWithSpace(utf8_stringview string) noexcept
    {
      constexpr static utf8_stringview noBreakSpace {u8" "};
      utf8_string result;
      result.reserve(string.size());
      for (size_t i = 0; i < string.size();)
      {
        if (string.substr(i).starts_with(noBreakSpace))
        {
          result.push_back(u8' ');
          i += noBreakSpace.size();
          continue;
        }
        result.push_back(string[i++]);
      }
      return result;
    }

    KRYS_NODISCARD utf8_string EscapeMarkupText(utf8_stringview text) noexcept
    {
      utf8_string escaped;
      escaped.reserve(text.size());
      for (auto c : text)
      {
        switch (c)
        {
          case u8'&':  escaped += u8"&amp;"; break;
          case u8'<':  escaped += u8"&lt;"; break;
          case u8'>':  escaped += u8"&gt;"; break;
          case u8'\'': escaped += u8"&#39;"; break;
          case u8'"':  escaped += u8"&quot;"; break;
          default:     escaped.push_back(c); break;
        }
      }
      return escaped;
    }

    KRYS_NODISCARD utf8_stringview TrimSpacesAndNewlines(utf8_stringview string) noexcept
    {
      auto isSpace = [](char8_t c)
      {
        return text::IsASCIIWhitespace(c) || c == u8'\v';
      };
      while (!string.empty() && isSpace(string.front()))
      {
        string.remove_prefix(1);
      }
      while (!string.empty() && isSpace(string.back()))
      {
        string.remove_suffix(1);
      }
      return string;
    }
  }

  void SelectionData::SetText(utf8_stringview text) noexcept
  {
    _text = ReplaceNonBreakingSpaceWithSpace(text);
  }

  void SelectionData::SetURIList(utf8_stringview uriListString) noexcept
  {
    _uriList = utf8_string(uriListString);

    // The URL is the list's first valid one, which getData("url") gives
    // (https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-getdata), and every file URL's path is a file name.
    // Lines end in CRLF (RFC 2483), but a lone LF is accepted too; lines starting with `#` are comments.
    bool setUrl = HasUrl();
    utf8_stringview rest = uriListString;
    while (!rest.empty())
    {
      auto lineEnd = rest.find(u8'\n');
      auto line = TrimSpacesAndNewlines(rest.substr(0, lineEnd));
      rest = lineEnd == utf8_stringview::npos ? utf8_stringview() : rest.substr(lineEnd + 1);
      if (line.empty())
      {
        continue;
      }
      if (line[0] == u8'#')
      {
        continue;
      }

      net::Url url {line};
      if (url.IsValid())
      {
        if (!setUrl)
        {
          _url = url;
          setUrl = true;
        }

        if (url.ProtocolIsFile())
        {
          if (auto filename = url.FileSystemPath(); !filename.empty())
          {
            _filenames.push_back(krys::move(filename));
          }
        }
      }
    }
  }

  void SelectionData::SetUrl(const net::Url &url, utf8_stringview label) noexcept
  {
    _url = url;
    if (_uriList.empty())
    {
      _uriList = url.String();
    }

    if (!HasText())
    {
      SetText(url.String());
    }

    if (HasMarkup())
    {
      return;
    }

    utf8_stringview actualLabel = label.empty() ? utf8_stringview(url.String()) : label;
    _markup = u8"<a href=\"" + url.String() + u8"\">" + EscapeMarkupText(actualLabel) + u8"</a>";
  }

  const utf8_string &SelectionData::UrlLabel() const noexcept
  {
    static const utf8_string emptyString;
    if (HasText())
    {
      return Text();
    }

    if (HasUrl())
    {
      return _url.String();
    }

    return emptyString;
  }

  void SelectionData::ClearAllExceptFilenames() noexcept
  {
    ClearText();
    ClearMarkup();
    ClearURIList();
    ClearUrl();
    ClearCustomData();
    ClearBuffers();

    _canSmartReplace = false;
  }

  void SelectionData::ClearAll() noexcept
  {
    ClearAllExceptFilenames();
    _filenames.clear();
  }
}
