#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Detection/OS.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>
#include <charconv>
#include <compare>
#include <unicode/uidna.h>
#include <utility>

namespace krys::boo::url
{
  namespace
  {
    bool ShouldTrimFromURL(char8 character) noexcept
    {
      // Ignore leading/trailing whitespace and control characters.
      return character <= ' ';
    }

    bool ContainsOnlyASCII(utf8_stringview string) noexcept
    {
      return std::ranges::all_of(string, [](char8 c) { return text::IsASCII(c); });
    }

    Maybe<uint16> ParseUInt16(utf8_stringview digits) noexcept
    {
      uint16 value = 0;
      auto begin = reinterpret_cast<const char *>(digits.data());
      auto end = begin + digits.size();
      auto [ptr, error] = std::from_chars(begin, end, value);
      if (error != std::errc {} || ptr != end || digits.empty())
      {
        return null;
      }
      return value;
    }

    void AppendNumber(utf8_string &string, uint32 number) noexcept
    {
      Array<char, 10> buffer;
      auto [end, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), number);
      krys_debug_assert(error == std::errc {});
      string.append(reinterpret_cast<const char8 *>(buffer.data()), static_cast<size_t>(end - buffer.data()));
    }

    /// @brief Concatenates `parts`.
    utf8_string MakeString(std::initializer_list<utf8_stringview> parts) noexcept
    {
      size_t length = 0;
      for (auto part : parts)
      {
        length += part.size();
      }
      utf8_string result;
      result.reserve(length);
      for (auto part : parts)
      {
        result.append(part);
      }
      return result;
    }

    Maybe<char8> DecodeEscapeSequence(utf8_stringview input, size_t index, size_t length) noexcept
    {
      if (index + 3 > length || input[index] != '%')
      {
        return null;
      }
      auto digit1 = input[index + 1];
      auto digit2 = input[index + 2];
      if (!text::IsASCIIHexDigit(digit1) || !text::IsASCIIHexDigit(digit2))
      {
        return null;
      }
      return static_cast<char8>(text::ToASCIIHexValue(digit1, digit2));
    }

    utf8_string DecodeEscapeSequencesFromParsedURL(utf8_stringview input) noexcept
    {
      krys_debug_assert(ContainsOnlyASCII(input));

      auto length = input.size();
      if (length < 3 || input.find(u8'%') == utf8_stringview::npos)
      {
        return utf8_string {input};
      }

      utf8_string percentDecoded;
      percentDecoded.reserve(length);
      for (size_t i = 0; i < length;)
      {
        if (auto decodedCharacter = DecodeEscapeSequence(input, i, length))
        {
          percentDecoded.push_back(*decodedCharacter);
          i += 3;
        }
        else
        {
          percentDecoded.push_back(input[i]);
          ++i;
        }
      }

      // FIXME: Is UTF-8 always the correct encoding?
      // FIXME: This returns an empty string when we encounter an invalid UTF-8 sequence. Is that OK?
      if (!text::IsValidUTF8(percentDecoded))
      {
        return {};
      }
      return percentDecoded;
    }

    // Appends the punycoded hostname identified by the given string and length to the output buffer.
    // Return value of false means error in encoding.
    bool AppendEncodedHostname(utf8_string &buffer, utf8_stringview string) noexcept
    {
      // hostnameBuffer needs to be big enough to hold an IDN-encoded name.
      // For host names bigger than this, we won't do IDN encoding, which is almost certainly OK.
      if (string.size() > URLParser::HostnameBufferLength || ContainsOnlyASCII(string))
      {
        buffer.append(string);
        return true;
      }

      Array<char, URLParser::HostnameBufferLength> hostnameBuffer;
      UErrorCode error = U_ZERO_ERROR;
      UIDNAInfo processingDetails = UIDNA_INFO_INITIALIZER;
      int32_t numCharactersConverted = uidna_nameToASCII_UTF8(
        &URLParser::InternationalDomainNameTranscoder(), reinterpret_cast<const char *>(string.data()),
        static_cast<int32_t>(string.size()), hostnameBuffer.data(),
        static_cast<int32_t>(hostnameBuffer.size()), &processingDetails, &error);

      if (U_SUCCESS(error) && !(processingDetails.errors & ~URLParser::AllowedNameToASCIIErrors)
          && numCharactersConverted)
      {
        buffer.append(reinterpret_cast<const char8 *>(hostnameBuffer.data()),
                      static_cast<size_t>(numCharactersConverted));
        return true;
      }
      return false;
    }

    bool ForwardSlashHashOrQuestionMark(char8 c) noexcept
    {
      return c == '/' || c == '#' || c == '?';
    }

    bool SlashHashOrQuestionMark(char8 c) noexcept
    {
      return ForwardSlashHashOrQuestionMark(c) || c == '\\';
    }

    size_t FindFirstOf(utf8_stringview string, bool (*predicate)(char8)) noexcept
    {
      auto it = std::ranges::find_if(string, predicate);
      return it == string.end() ? utf8_stringview::npos : static_cast<size_t>(it - string.begin());
    }

    uint32 CountASCIIDigits(utf8_stringview string) noexcept
    {
      auto length = static_cast<uint32>(string.size());
      for (uint32 count = 0; count < length; ++count)
      {
        if (!text::IsASCIIDigit(string[count]))
        {
          return count;
        }
      }
      return length;
    }

    utf8_string PercentEncodeCharacters(utf8_stringview input, bool (*shouldEncode)(char32)) noexcept
    {
      if (std::ranges::none_of(input, [&](char8 c) { return shouldEncode(c); })) KRYS_LIKELY
      {
        return utf8_string {input};
      }

      utf8_string result;
      result.reserve(input.size());
      for (char8 c : input)
      {
        if (shouldEncode(c))
        {
          result.push_back(u8'%');
          result.push_back(static_cast<char8>(text::UpperNibbleToASCIIHexDigit(c)));
          result.push_back(static_cast<char8>(text::LowerNibbleToASCIIHexDigit(c)));
        }
        else
        {
          result.push_back(c);
        }
      }
      return result;
    }

    utf8_string EscapePathWithoutCopying(utf8_stringview path) noexcept
    {
      auto questionMarkOrNumberSignOrNonASCII = [](char32 character)
      {
        return character == '?' || character == '#' || !text::IsASCII(character);
      };
      return PercentEncodeCharacters(path, questionMarkOrNumberSignOrNonASCII);
    }

    bool ProtocolIsInternal(utf8_stringview string, utf8_stringview protocol) noexcept
    {
      size_t protocolIndex = 0;
      bool isLeading = true;
      for (auto codeUnit : string)
      {
        if (isLeading)
        {
          // Skip leading whitespace and control characters.
          if (ShouldTrimFromURL(codeUnit))
          {
            continue;
          }
          isLeading = false;
        }
        else
        {
          // Skip tabs and newlines even later in the protocol.
          if (codeUnit == '\t' || codeUnit == '\r' || codeUnit == '\n')
          {
            continue;
          }
        }

        if (protocolIndex == protocol.size())
        {
          return codeUnit == ':';
        }
        char8 expectedCharacter = protocol[protocolIndex++];
        if (!text::IsASCIIAlphaCaselessEqual(codeUnit, expectedCharacter))
        {
          return false;
        }
      }
      return false;
    }

    bool IsIPv4Address(utf8_stringview string) noexcept
    {
      auto count = 0;

      size_t start = 0;
      while (true)
      {
        size_t end = string.find(u8'.', start);
        utf8_stringview octet =
          string.substr(start, end == utf8_stringview::npos ? utf8_stringview::npos : end - start);

        if (count >= 4)
        {
          return false;
        }

        const auto length = octet.size();
        if (!length || length > 3)
        {
          return false;
        }

        auto value = 0;
        for (auto i = 0u; i < length; ++i)
        {
          const auto digit = octet[i];

          // Prohibit leading zeroes.
          if (digit > '9' || digit < (!i && length > 1 ? '1' : '0'))
          {
            return false;
          }

          value = 10 * value + (digit - '0');
        }

        if (value > 255)
        {
          return false;
        }

        count++;

        if (end == utf8_stringview::npos)
        {
          break;
        }
        start = end + 1;
      }

      return count == 4;
    }

    utf8_stringview SubstringIgnoringQueryAndFragments(const URL &url KRYS_LIFETIME_BOUND) noexcept
    {
      if (!url.IsValid())
      {
        return url.String();
      }

      return utf8_stringview {url.String()}.substr(0, url.PathEnd());
    }
  }

  void URL::Invalidate() noexcept
  {
    _isValid = false;
    _protocolIsInHTTPFamily = false;
    _hasOpaquePath = false;
    _schemeEnd = 0;
    _userStart = 0;
    _userEnd = 0;
    _passwordEnd = 0;
    _hostEnd = 0;
    _portLength = 0;
    _pathEnd = 0;
    _pathAfterLastSlash = 0;
    _queryEnd = 0;
  }

  URL::URL(HashTableDeletedValueTag) noexcept : _isNull(false), _isHashTableDeletedValue(true)
  {
  }

  URL::URL(const URL &base, utf8_stringview relative) noexcept
  {
    *this = URLParser(relative, base).Result();
  }

  URL::URL(utf8_stringview absoluteURL) noexcept
  {
    *this = URLParser(absoluteURL).Result();
  }

  URL::URL(URL &&other) noexcept
      : _string(krys::move(other._string)), _isValid(other._isValid),
        _protocolIsInHTTPFamily(other._protocolIsInHTTPFamily), _hasOpaquePath(other._hasOpaquePath),
        _isNull(other._isNull), _isHashTableDeletedValue(other._isHashTableDeletedValue),
        _portLength(other._portLength), _schemeEnd(other._schemeEnd), _userStart(other._userStart),
        _userEnd(other._userEnd), _passwordEnd(other._passwordEnd), _hostEnd(other._hostEnd),
        _pathAfterLastSlash(other._pathAfterLastSlash), _pathEnd(other._pathEnd), _queryEnd(other._queryEnd)
  {
    other._isValid = false;
  }

  URL &URL::operator=(URL &&other) noexcept
  {
    _string = krys::move(other._string);
    _isValid = other._isValid;
    other._isValid = false;
    _protocolIsInHTTPFamily = other._protocolIsInHTTPFamily;
    _hasOpaquePath = other._hasOpaquePath;
    _isNull = other._isNull;
    _isHashTableDeletedValue = other._isHashTableDeletedValue;
    _portLength = other._portLength;
    _schemeEnd = other._schemeEnd;
    _userStart = other._userStart;
    _userEnd = other._userEnd;
    _passwordEnd = other._passwordEnd;
    _hostEnd = other._hostEnd;
    _pathAfterLastSlash = other._pathAfterLastSlash;
    _pathEnd = other._pathEnd;
    _queryEnd = other._queryEnd;

    return *this;
  }

  utf8_stringview URL::LastPathComponent() const noexcept
  {
    if (!HasPath())
    {
      return {};
    }

    uint32 end = _pathEnd - 1;
    if (_string[end] == '/')
    {
      --end;
    }

    size_t start = utf8_stringview {_string}.rfind(u8'/', end);
    if (start == utf8_stringview::npos || start < PathStart())
    {
      return {};
    }
    ++start;

    return utf8_stringview {_string}.substr(start, end - start + 1);
  }

  bool URL::HasSpecialScheme() const noexcept
  {
    // https://url.spec.whatwg.org/#special-scheme
    return ProtocolIs(u8"ftp") || ProtocolIsFile() || ProtocolIs(u8"http") || ProtocolIs(u8"https")
           || ProtocolIs(u8"ws") || ProtocolIs(u8"wss");
  }

  bool URL::HasLocalScheme() const noexcept
  {
    // https://fetch.spec.whatwg.org/#local-scheme
    return ProtocolIsAbout() || ProtocolIsBlob() || ProtocolIsData();
  }

  bool URL::HasFetchScheme() const noexcept
  {
    // https://fetch.spec.whatwg.org/#fetch-scheme
    return ProtocolIsInHTTPFamily() || ProtocolIsAbout() || ProtocolIsBlob() || ProtocolIsData()
           || ProtocolIsFile();
  }

  bool URL::ProtocolIsSecure() const noexcept
  {
    // Note: FTPS is not considered secure.
    return ProtocolIs(u8"https") || ProtocolIs(u8"wss");
  }

  uint32 URL::PathStart() const noexcept
  {
    uint32 start = _hostEnd + _portLength;
    if (start == _schemeEnd + 1U && start + 1 < _string.size() && _string[start] == '/'
        && _string[start + 1] == '.')
    {
      start += 2;
    }
    return start;
  }

  utf8_stringview URL::Protocol() const noexcept
  {
    if (!_isValid)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(0, _schemeEnd);
  }

  utf8_stringview URL::Host() const noexcept
  {
    if (!_isValid)
    {
      return {};
    }

    uint32 start = HostStart();
    return utf8_stringview {_string}.substr(start, _hostEnd - start);
  }

  Maybe<uint16> URL::Port() const noexcept
  {
    return _portLength ? ParseUInt16(utf8_stringview {_string}.substr(_hostEnd + 1, _portLength - 1)) : null;
  }

  utf8_string URL::HostAndPort() const noexcept
  {
    utf8_string result {Host()};
    if (auto port = Port())
    {
      result.push_back(u8':');
      AppendNumber(result, *port);
    }
    return result;
  }

  utf8_string URL::ProtocolHostAndPort() const noexcept
  {
    if (!HasCredentials())
    {
      return _string.substr(0, PathStart());
    }

    return MakeString({utf8_stringview {_string}.substr(0, _userStart),
                       utf8_stringview {_string}.substr(HostStart(), PathStart() - HostStart())});
  }

  utf8_string URL::User() const noexcept
  {
    return DecodeEscapeSequencesFromParsedURL(EncodedUser());
  }

  utf8_string URL::Password() const noexcept
  {
    return DecodeEscapeSequencesFromParsedURL(EncodedPassword());
  }

  utf8_stringview URL::EncodedUser() const noexcept
  {
    return utf8_stringview {_string}.substr(_userStart, _userEnd - _userStart);
  }

  utf8_stringview URL::EncodedPassword() const noexcept
  {
    if (_passwordEnd == _userEnd)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(_userEnd + 1, _passwordEnd - _userEnd - 1);
  }

  utf8_stringview URL::FragmentIdentifier() const noexcept
  {
    if (!HasFragmentIdentifier())
    {
      return {};
    }

    return utf8_stringview {_string}.substr(_queryEnd + 1);
  }

  utf8_string URL::ConsumeFragmentDirective() noexcept
  {
    constexpr utf8_stringview fragmentDirectiveDelimiter = u8":~:";
    auto fragment = FragmentIdentifier();

    auto fragmentDirectiveStart = fragment.find(fragmentDirectiveDelimiter);

    if (fragmentDirectiveStart == utf8_stringview::npos)
    {
      return {};
    }

    utf8_string fragmentDirective {
      fragment.substr(fragmentDirectiveStart + fragmentDirectiveDelimiter.size())};

    utf8_string remainingFragment {fragment.substr(0, fragmentDirectiveStart)};
    if (remainingFragment.empty())
    {
      RemoveFragmentIdentifier();
    }
    else
    {
      SetFragmentIdentifier(remainingFragment);
    }

    return fragmentDirective;
  }

  URL URL::TruncatedForUseAsBase() const noexcept
  {
    return URL(utf8_stringview {_string}.substr(0, _pathAfterLastSlash));
  }

  utf8_string URL::FileSystemPath() const noexcept
  {
    if (!ProtocolIsFile())
    {
      return {};
    }

    auto result = DecodeEscapeSequencesFromParsedURL(Path());
#if KRYS_OS(WINDOWS)
    if (!result.empty() && result.front() == '/')
    {
      result.erase(0, 1);
    }
#endif
    return result;
  }

  Maybe<uint16> DefaultPortForProtocol(utf8_stringview protocol) noexcept
  {
    return URLParser::DefaultPortForProtocol(protocol);
  }

  bool IsDefaultPortForProtocol(uint16 port, utf8_stringview protocol) noexcept
  {
    return DefaultPortForProtocol(protocol) == port;
  }

  bool URL::ProtocolIsJavaScript() const noexcept
  {
    return url::ProtocolIsJavaScript(String());
  }

  bool URL::ProtocolIs(utf8_stringview protocol) const noexcept
  {
    krys_debug_assert(
      std::ranges::none_of(protocol, [](char8 c) { return text::IsASCIIUpper(c) || c <= ' '; }));

    if (!_isValid)
    {
      return false;
    }

    if (_schemeEnd != protocol.size())
    {
      return false;
    }

    // Do the comparison without making a new string object.
    for (uint32 i = 0; i < _schemeEnd; ++i)
    {
      if (!text::IsASCIIAlphaCaselessEqual(_string[i], protocol[i]))
      {
        return false;
      }
    }
    return true;
  }

  utf8_stringview URL::Query() const noexcept
  {
    if (_queryEnd == _pathEnd)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(_pathEnd + 1, _queryEnd - (_pathEnd + 1));
  }

  utf8_stringview URL::Path() const noexcept
  {
    if (!_isValid)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(PathStart(), _pathEnd - PathStart());
  }

  bool URL::SetProtocol(utf8_stringview newProtocol) noexcept
  {
    auto newProtocolPrefix = newProtocol.substr(0, newProtocol.find(u8':'));
    auto newProtocolCanonicalised = URLParser::MaybeCanonicaliseScheme(newProtocolPrefix);
    if (!newProtocolCanonicalised)
    {
      return false;
    }

    if (!_isValid)
    {
      Parse(MakeString({*newProtocolCanonicalised, u8":", _string}));
      return true;
    }

    if (URLParser::IsSpecialScheme(Protocol()) != URLParser::IsSpecialScheme(*newProtocolCanonicalised))
    {
      return true;
    }

    if ((_passwordEnd != _userStart || Port()) && *newProtocolCanonicalised == u8"file")
    {
      return true;
    }

    if (ProtocolIsFile() && Host().empty())
    {
      return true;
    }

    Parse(MakeString({*newProtocolCanonicalised, utf8_stringview {_string}.substr(_schemeEnd)}));
    return true;
  }

  uint32 URL::HostStart() const noexcept
  {
    return (_passwordEnd == _userStart) ? _passwordEnd : _passwordEnd + 1;
  }

  uint32 URL::CredentialsEnd() const noexcept
  {
    // Include '@' too if we have it.
    uint32 end = _passwordEnd;
    if (end != _hostEnd && _string[end] == '@')
    {
      end += 1;
    }
    return end;
  }

  bool URL::SetHost(utf8_stringview newHost) noexcept
  {
    if (!_isValid || HasOpaquePath())
    {
      return false;
    }

    if (auto index =
          FindFirstOf(newHost, HasSpecialScheme() ? SlashHashOrQuestionMark : ForwardSlashHashOrQuestionMark);
        index != utf8_stringview::npos)
    {
      newHost = newHost.substr(0, index);
    }

    if (newHost.find(u8'@') != utf8_stringview::npos)
    {
      return false;
    }

    if (newHost.find(u8':') != utf8_stringview::npos && !newHost.starts_with(u8'['))
    {
      return false;
    }

    utf8_string encodedHostName;
    if (HasSpecialScheme() && !AppendEncodedHostname(encodedHostName, newHost))
    {
      return false;
    }

    bool slashSlashNeeded = _userStart == _schemeEnd + 1U;
    Parse(MakeString({utf8_stringview {_string}.substr(0, HostStart()), slashSlashNeeded ? u8"//" : u8"",
                      HasSpecialScheme() ? utf8_stringview {encodedHostName} : newHost,
                      utf8_stringview {_string}.substr(_hostEnd)}));

    return _isValid;
  }

  void URL::SetPort(Maybe<uint16> port) noexcept
  {
    if (!_isValid)
    {
      return;
    }

    if (!port)
    {
      Remove(_hostEnd, _portLength);
      return;
    }

    utf8_string portString;
    AppendNumber(portString, *port);
    Parse(MakeString({utf8_stringview {_string}.substr(0, _hostEnd), u8":", portString,
                      utf8_stringview {_string}.substr(PathStart())}));
  }

  void URL::SetHostAndPort(utf8_stringview hostAndPort) noexcept
  {
    if (!_isValid || HasOpaquePath())
    {
      return;
    }

    if (auto index = FindFirstOf(hostAndPort, HasSpecialScheme() ? SlashHashOrQuestionMark
                                                                 : ForwardSlashHashOrQuestionMark);
        index != utf8_stringview::npos)
    {
      hostAndPort = hostAndPort.substr(0, index);
    }

    auto colonIndex = hostAndPort.rfind(u8':');
    if (!colonIndex)
    {
      return;
    }

    auto ipv6Separator = hostAndPort.rfind(u8']');
    if (colonIndex == utf8_stringview::npos
        || (ipv6Separator != utf8_stringview::npos && ipv6Separator > colonIndex))
    {
      SetHost(hostAndPort);
      return;
    }

    auto portString = hostAndPort.substr(colonIndex + 1);
    auto hostName = hostAndPort.substr(0, colonIndex);
    if (hostName.find(u8'@') != utf8_stringview::npos)
    {
      return;
    }
    // Multiple colons are acceptable only in case of IPv6.
    if (hostName.find(u8':') != utf8_stringview::npos && ipv6Separator == utf8_stringview::npos)
    {
      return;
    }

    uint32 portLength = CountASCIIDigits(portString);
    if (!portLength)
    {
      SetHost(hostName);
      return;
    }
    portString = portString.substr(0, portLength);
    if (!ParseUInt16(portString))
    {
      portString = {};
    }

    utf8_string encodedHostName;
    if (HasSpecialScheme() && !AppendEncodedHostname(encodedHostName, hostName))
    {
      return;
    }

    bool slashSlashNeeded = _userStart == _schemeEnd + 1U;
    Parse(MakeString({utf8_stringview {_string}.substr(0, HostStart()), slashSlashNeeded ? u8"//" : u8"",
                      HasSpecialScheme() ? utf8_stringview {encodedHostName} : hostName,
                      portString.empty() ? u8"" : u8":", portString,
                      utf8_stringview {_string}.substr(PathStart())}));
  }

  void URL::RemoveHostAndPort() noexcept
  {
    if (_isValid)
    {
      Remove(HostStart(), PathStart() - HostStart());
    }
  }

  void URL::Parse(utf8_stringview string) noexcept
  {
    *this = URLParser(string).Result();
  }

  void URL::ParseAllowingC0AtEnd(utf8_stringview string) noexcept
  {
    *this = URLParser(string, {}, true).Result();
  }

  void URL::Remove(uint32 start, uint32 length) noexcept
  {
    if (!length)
    {
      return;
    }
    krys_debug_assert(start < _string.size());
    krys_debug_assert(length <= _string.size() - start);

    auto stringAfterRemoval = std::exchange(_string, {});
    stringAfterRemoval.erase(start, length);
    Parse(stringAfterRemoval);
  }

  void URL::SetUser(utf8_stringview newUser) noexcept
  {
    if (!_isValid)
    {
      return;
    }

    uint32 end = _userEnd;
    if (!newUser.empty())
    {
      bool slashSlashNeeded = _userStart == _schemeEnd + 1U;
      bool needSeparator = end == _hostEnd || (end == _passwordEnd && _string[end] != '@');
      Parse(MakeString({utf8_stringview {_string}.substr(0, _userStart), slashSlashNeeded ? u8"//" : u8"",
                        PercentEncodeCharacters(newUser, URLParser::IsInUserInfoEncodeSet),
                        needSeparator ? u8"@" : u8"", utf8_stringview {_string}.substr(end)}));
    }
    else
    {
      // Remove '@' if we now have neither user nor password.
      if (_userEnd == _passwordEnd && end != _hostEnd && _string[end] == '@')
      {
        end += 1;
      }
      Remove(_userStart, end - _userStart);
    }
  }

  void URL::SetPassword(utf8_stringview newPassword) noexcept
  {
    if (!_isValid)
    {
      return;
    }

    if (!newPassword.empty())
    {
      bool needLeadingSlashes = _userEnd == _schemeEnd + 1U;
      Parse(MakeString({utf8_stringview {_string}.substr(0, _userEnd), needLeadingSlashes ? u8"//:" : u8":",
                        PercentEncodeCharacters(newPassword, URLParser::IsInUserInfoEncodeSet), u8"@",
                        utf8_stringview {_string}.substr(CredentialsEnd())}));
    }
    else
    {
      uint32 end = _userStart == _userEnd ? CredentialsEnd() : _passwordEnd;
      Remove(_userEnd, end - _userEnd);
    }
  }

  void URL::RemoveCredentials() noexcept
  {
    if (!_isValid)
    {
      return;
    }

    Remove(_userStart, CredentialsEnd() - _userStart);
  }

  void URL::SetFragmentIdentifier(utf8_stringview identifier) noexcept
  {
    if (!_isValid)
    {
      return;
    }

    ParseAllowingC0AtEnd(MakeString({utf8_stringview {_string}.substr(0, _queryEnd), u8"#", identifier}));
  }

  void URL::RemoveFragmentIdentifier() noexcept
  {
    if (!_isValid)
    {
      return;
    }

    _string.resize(_queryEnd);
  }

  void URL::RemoveQueryAndFragmentIdentifier() noexcept
  {
    if (!_isValid)
    {
      return;
    }

    _string.resize(_pathEnd);
    _queryEnd = _pathEnd;
  }

  void URL::SetQuery(utf8_stringview newQuery) noexcept
  {
    // FIXME: Consider renaming this function to SetEncodedQuery and/or calling PercentEncodeCharacters the
    // way SetPath does.

    if (!_isValid)
    {
      return;
    }

    ParseAllowingC0AtEnd(MakeString({utf8_stringview {_string}.substr(0, _pathEnd),
                                     (!newQuery.starts_with(u8'?') && newQuery.data()) ? u8"?" : u8"",
                                     newQuery, utf8_stringview {_string}.substr(_queryEnd)}));
  }

  void URL::SetPath(utf8_stringview path) noexcept
  {
    if (!_isValid)
    {
      return;
    }

    bool noSlashNeeded = path.starts_with(u8'/')
                         || (path.starts_with(u8'\\') && (HasSpecialScheme() || ProtocolIsFile()))
                         || (!HasSpecialScheme() && path.empty() && _schemeEnd + 1U < PathStart());
    bool dotSlashNeeded =
      !HasSpecialScheme() && Host().empty() && path.starts_with(u8"//") && path.size() > 2;
    ParseAllowingC0AtEnd(
      MakeString({utf8_stringview {_string}.substr(0, PathStart()), noSlashNeeded ? u8"" : u8"/",
                  dotSlashNeeded ? u8"/." : u8"", EscapePathWithoutCopying(path),
                  utf8_stringview {_string}.substr(_pathEnd)}));
  }

  utf8_stringview URL::ViewWithoutQueryOrFragmentIdentifier() const noexcept
  {
    if (!_isValid)
    {
      return _string;
    }

    return utf8_stringview {_string}.substr(0, PathEnd());
  }

  utf8_stringview URL::ViewWithoutFragmentIdentifier() const noexcept
  {
    if (!_isValid)
    {
      return _string;
    }

    return utf8_stringview {_string}.substr(0, _queryEnd);
  }

  utf8_string URL::StringWithoutFragmentIdentifier() const noexcept
  {
    if (!_isValid)
    {
      return _string;
    }

    return _string.substr(0, _queryEnd);
  }

  bool EqualIgnoringFragmentIdentifier(const URL &a, const URL &b) noexcept
  {
    return a.ViewWithoutFragmentIdentifier() == b.ViewWithoutFragmentIdentifier();
  }

  bool ProtocolHostAndPortAreEqual(const URL &a, const URL &b) noexcept
  {
    if (a._schemeEnd != b._schemeEnd)
    {
      return false;
    }

    uint32 hostStartA = a.HostStart();
    uint32 hostLengthA = a._hostEnd - hostStartA;
    uint32 hostStartB = b.HostStart();
    uint32 hostLengthB = b._hostEnd - b.HostStart();
    if (hostLengthA != hostLengthB)
    {
      return false;
    }

    // Check the scheme
    for (uint32 i = 0; i < a._schemeEnd; ++i)
    {
      if (text::ToASCIILower(a.String()[i]) != text::ToASCIILower(b.String()[i]))
      {
        return false;
      }
    }

    // And the host
    for (uint32 i = 0; i < hostLengthA; ++i)
    {
      if (text::ToASCIILower(a.String()[hostStartA + i]) != text::ToASCIILower(b.String()[hostStartB + i]))
      {
        return false;
      }
    }

    if (a.Port() != b.Port())
    {
      return false;
    }

    return true;
  }

  bool URL::IsMatchingDomain(utf8_stringview domain) const noexcept
  {
    if (IsNull())
    {
      return false;
    }

    if (domain.empty())
    {
      return true;
    }

    if (!ProtocolIsInHTTPFamily())
    {
      return false;
    }

    auto host = Host();
    if (!host.ends_with(domain))
    {
      return false;
    }

    return host.size() == domain.size() || host[host.size() - domain.size() - 1] == '.';
  }

  // FIXME: Rename this so it's clear that it does the appropriate escaping for URL query field values.
  utf8_string EncodeWithURLEscapeSequences(utf8_stringview input) noexcept
  {
    return PercentEncodeCharacters(input, URLParser::IsInUserInfoEncodeSet);
  }

  utf8_string PercentEncodeFragmentDirectiveSpecialCharacters(utf8_stringview input) noexcept
  {
    return PercentEncodeCharacters(input, URLParser::IsSpecialCharacterForFragmentDirective);
  }

  bool ProtocolIs(utf8_stringview url, utf8_stringview protocol) noexcept
  {
    return ProtocolIsInternal(url, protocol);
  }

  URL::StripResult URL::StrippedForUseAsReferrer() const noexcept
  {
    if (!_isValid)
    {
      return {_string, false};
    }

    uint32 end = CredentialsEnd();

    if (_userStart == end && _queryEnd == _string.size())
    {
      return {_string, false};
    }

    return {MakeString({utf8_stringview {_string}.substr(0, _userStart),
                        utf8_stringview {_string}.substr(end, _queryEnd - end)}),
            true};
  }

  URL::StripResult URL::StrippedForUseAsReferrerWithExplicitPort() const noexcept
  {
    if (!_isValid)
    {
      return {_string, false};
    }

    // Custom ports will appear in the URL string:
    if (_portLength)
    {
      return StrippedForUseAsReferrer();
    }

    auto port = DefaultPortForProtocol(Protocol());
    if (!port)
    {
      return StrippedForUseAsReferrer();
    }

    uint32 end = CredentialsEnd();
    utf8_string portString;
    AppendNumber(portString, *port);

    if (_userStart == end && _queryEnd == _string.size())
    {
      return {MakeString({utf8_stringview {_string}.substr(0, _hostEnd), u8":", portString,
                          utf8_stringview {_string}.substr(PathStart())}),
              true};
    }

    return {MakeString({utf8_stringview {_string}.substr(0, _hostEnd), u8":", portString,
                        utf8_stringview {_string}.substr(end, _queryEnd - end)}),
            true};
  }

  utf8_string URL::StrippedForUseAsReport() const noexcept
  {
    if (!_isValid)
    {
      return _string;
    }

    uint32 end = CredentialsEnd();

    if (_userStart == end && _pathEnd == _string.size())
    {
      return _string;
    }

    return MakeString({utf8_stringview {_string}.substr(0, _userStart),
                       utf8_stringview {_string}.substr(end, _pathEnd - end)});
  }

  bool ProtocolIsJavaScript(utf8_stringview url) noexcept
  {
    return ProtocolIsInternal(url, u8"javascript");
  }

  bool ProtocolIsInHTTPFamily(utf8_stringview url) noexcept
  {
    auto length = url.size();
    // Do the comparison without making a new string object.
    return length >= 5 && text::IsASCIIAlphaCaselessEqual(url[0], u8'h')
           && text::IsASCIIAlphaCaselessEqual(url[1], u8't') && text::IsASCIIAlphaCaselessEqual(url[2], u8't')
           && text::IsASCIIAlphaCaselessEqual(url[3], u8'p')
           && (url[4] == ':'
               || (text::IsASCIIAlphaCaselessEqual(url[4], u8's') && length >= 6 && url[5] == ':'));
  }

  const URL &AboutBlankURL() noexcept
  {
    static NeverDestroyed<URL> staticBlankURL {URL {u8"about:blank"}};
    return staticBlankURL;
  }

  const URL &AboutSrcDocURL() noexcept
  {
    static NeverDestroyed<URL> staticSrcDocURL {URL {u8"about:srcdoc"}};
    return staticSrcDocURL;
  }

  bool PortAllowed(const URL &url) noexcept
  {
    Maybe<uint16> port = url.Port();

    // Since most URLs don't have a port, return early for the "no port" case.
    if (!port)
    {
      return true;
    }

    // This blocked port list is defined by the Fetch spec, with the addition of port 0.
    // See https://fetch.spec.whatwg.org/#port-blocking for more information.
    constexpr auto blockedPortList = std::to_array<uint16>({
      0,      // reserved
      1,      // tcpmux
      7,      // echo
      9,      // discard
      11,     // systat
      13,     // daytime
      15,     // netstat
      17,     // qotd
      19,     // chargen
      20,     // FTP-data
      21,     // FTP-control
      22,     // SSH
      23,     // telnet
      25,     // SMTP
      37,     // time
      42,     // name
      43,     // nicname
      53,     // domain
      69,     // TFTP
      77,     // priv-rjs
      79,     // finger
      87,     // ttylink
      95,     // supdup
      101,    // hostriame
      102,    // iso-tsap
      103,    // gppitnp
      104,    // acr-nema
      109,    // POP2
      110,    // POP3
      111,    // sunrpc
      113,    // auth
      115,    // SFTP
      117,    // uucp-path
      119,    // nntp
      123,    // NTP
      135,    // loc-srv / epmap
      137,    // NetBIOS
      139,    // netbios
      143,    // IMAP2
      161,    // SNMP
      179,    // BGP
      389,    // LDAP
      427,    // SLP (Also used by Apple Filing Protocol)
      465,    // SMTP+SSL
      512,    // print / exec
      513,    // login
      514,    // shell
      515,    // printer
      526,    // tempo
      530,    // courier
      531,    // Chat
      532,    // netnews
      540,    // UUCP
      548,    // afpovertcp [Apple addition]
      554,    // rtsp
      556,    // remotefs
      563,    // NNTP+SSL
      587,    // ESMTP
      601,    // syslog-conn
      636,    // LDAP+SSL
      989,    // ftps-data
      990,    // ftps
      993,    // IMAP+SSL
      995,    // POP3+SSL
      1'719,  // H323 (RAS)
      1'720,  // H323 (Q931)
      1'723,  // H323 (H245)
      2'049,  // NFS
      3'659,  // apple-sasl / PasswordServer [Apple addition]
      4'045,  // lockd
      4'190,  // ManageSieve [Apple addition]
      5'060,  // SIP
      5'061,  // SIPS
      6'000,  // X11
      6'566,  // SANE
      6'665,  // Alternate IRC [Apple addition]
      6'666,  // Alternate IRC [Apple addition]
      6'667,  // Standard IRC [Apple addition]
      6'668,  // Alternate IRC [Apple addition]
      6'669,  // Alternate IRC [Apple addition]
      6'679,  // Alternate IRC SSL [Apple addition]
      6'697,  // IRC+SSL [Apple addition]
      10'080, // amanda
    });

    // If the port is not in the blocked port list, allow it.
    krys_debug_assert(std::ranges::is_sorted(blockedPortList));
    if (!std::ranges::binary_search(blockedPortList, port.value()))
    {
      return true;
    }

    // Allow ports 21 and 22 for FTP URLs, as Mozilla does.
    if ((port.value() == 21 || port.value() == 22) && url.ProtocolIs(u8"ftp"))
    {
      return true;
    }

    // Allow any port number in a file URL, since the port number is ignored.
    if (url.ProtocolIsFile())
    {
      return true;
    }

    return false;
  }

  utf8_string MimeTypeFromDataURL(utf8_stringview dataURL) noexcept
  {
    krys_debug_assert(ProtocolIsInternal(dataURL, u8"data"));

    // FIXME: What's the right behavior when the URL has a comma first, but a semicolon later?
    // Currently this code will break at the semicolon in that case; should add a test.
    auto index = dataURL.find(u8';', 5);
    if (index == utf8_stringview::npos)
    {
      index = dataURL.find(u8',', 5);
    }
    if (index == utf8_stringview::npos)
    {
      // FIXME: There was an old comment here that made it sound like this should be returning text/plain.
      // But we have been returning empty string here for some time, so not changing its behavior at this
      // time.
      return {};
    }
    if (index == 5)
    {
      return u8"text/plain";
    }
    krys_debug_assert(index >= 5);
    utf8_string result {dataURL.substr(5, index - 5)};
    for (auto &c : result)
    {
      c = text::ToASCIILower(c);
    }
    return result;
  }

  utf8_string URL::StringCenterEllipsisedToLength(uint32 length) const noexcept
  {
    if (_string.size() <= length)
    {
      return _string;
    }

    return MakeString({utf8_stringview {_string}.substr(0, length / 2 - 1), u8"...",
                       utf8_stringview {_string}.substr(_string.size() - (length / 2 - 2))});
  }

  URL URL::FileURLWithFileSystemPath(utf8_stringview path) noexcept
  {
    return URL(
      MakeString({u8"file://", path.starts_with(u8'/') ? u8"" : u8"/", EscapePathWithoutCopying(path)}));
  }

  utf8_stringview URL::QueryWithLeadingQuestionMark() const noexcept
  {
    if (_queryEnd <= _pathEnd)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(_pathEnd, _queryEnd - _pathEnd);
  }

  utf8_stringview URL::FragmentIdentifierWithLeadingNumberSign() const noexcept
  {
    if (!_isValid || _string.size() <= _queryEnd)
    {
      return {};
    }

    return utf8_stringview {_string}.substr(_queryEnd);
  }

  bool URL::IsAboutBlank() const noexcept
  {
    return ProtocolIsAbout() && Path() == u8"blank";
  }

  bool URL::IsAboutSrcDoc() const noexcept
  {
    return ProtocolIsAbout() && Path() == u8"srcdoc";
  }

  bool URL::IsIPv6Address(utf8_stringview string) noexcept
  {
    enum SkipState
    {
      None,
      WillSkip,
      Skipping,
      Skipped,
      Final
    };
    auto skipState = None;
    auto count = 0;

    size_t start = 0;
    while (true)
    {
      size_t end = string.find(u8':', start);
      utf8_stringview hextet =
        string.substr(start, end == utf8_stringview::npos ? utf8_stringview::npos : end - start);
      bool isLast = end == utf8_stringview::npos;
      start = end + 1;

      if (count >= 8 || skipState == Final)
      {
        return false;
      }

      const auto length = hextet.size();
      if (!length)
      {
        // :: may be used anywhere to skip 1 to 8 hextets, but only once.
        if (skipState == Skipped)
        {
          return false;
        }

        if (skipState == None)
        {
          skipState = !count ? WillSkip : Skipping;
        }
        else if (skipState == WillSkip)
        {
          skipState = Skipping;
        }
        else
        {
          skipState = Final;
        }
        if (isLast)
        {
          break;
        }
        continue;
      }

      if (skipState == WillSkip)
      {
        return false;
      }

      if (skipState == Skipping)
      {
        skipState = Skipped;
      }

      if (length > 4)
      {
        // An IPv4 address may be used in place of the final two hextets.
        if ((skipState == None && count != 6) || (skipState == Skipped && count >= 6)
            || !IsIPv4Address(hextet))
        {
          return false;
        }

        skipState = Final;
        if (isLast)
        {
          break;
        }
        continue;
      }

      for (const auto codeUnit : hextet)
      {
        // IPv6 allows leading zeroes.
        if (!text::IsASCIIHexDigit(codeUnit))
        {
          return false;
        }
      }

      count++;
      if (isLast)
      {
        break;
      }
    }

    return (count == 8 && skipState == None) || skipState == Skipped || skipState == Final;
  }

  bool URL::HostIsIPAddress(utf8_stringview host) noexcept
  {
    return host.find(u8':') != utf8_stringview::npos ? IsIPv6Address(host) : IsIPv4Address(host);
  }

  List<KeyValuePair> QueryParameters(const URL &url) noexcept
  {
    return URLParser::ParseURLEncodedForm(url.Query());
  }

  List<KeyValuePair> DifferingQueryParameters(const URL &firstURL, const URL &secondURL) noexcept
  {
    auto firstQueryParameters = URLParser::ParseURLEncodedForm(firstURL.Query());
    auto secondQueryParameters = URLParser::ParseURLEncodedForm(secondURL.Query());

    if (firstQueryParameters.empty())
    {
      return secondQueryParameters;
    }

    if (secondQueryParameters.empty())
    {
      return firstQueryParameters;
    }

    // UTF-8 byte order is code point order.
    auto compare = [](const KeyValuePair &a, const KeyValuePair &b)
    {
      if (auto result = a.Key <=> b.Key; std::is_neq(result))
      {
        return result;
      }
      return a.Value <=> b.Value;
    };
    auto comparesLessThan = [compare](const KeyValuePair &a, const KeyValuePair &b)
    {
      return compare(a, b) < 0;
    };

    std::ranges::sort(firstQueryParameters, comparesLessThan);
    std::ranges::sort(secondQueryParameters, comparesLessThan);
    size_t totalFirstQueryParameters = firstQueryParameters.size();
    size_t totalSecondQueryParameters = secondQueryParameters.size();
    size_t indexInFirstQueryParameters = 0;
    size_t indexInSecondQueryParameters = 0;
    List<KeyValuePair> differingQueryParameters;
    while (indexInFirstQueryParameters < totalFirstQueryParameters
           && indexInSecondQueryParameters < totalSecondQueryParameters)
    {
      auto comparison = compare(firstQueryParameters[indexInFirstQueryParameters],
                                secondQueryParameters[indexInSecondQueryParameters]);
      if (std::is_lt(comparison))
      {
        differingQueryParameters.push_back(firstQueryParameters[indexInFirstQueryParameters]);
        indexInFirstQueryParameters++;
      }
      else if (std::is_gt(comparison))
      {
        differingQueryParameters.push_back(secondQueryParameters[indexInSecondQueryParameters]);
        indexInSecondQueryParameters++;
      }
      else
      {
        indexInFirstQueryParameters++;
        indexInSecondQueryParameters++;
      }
    }

    while (indexInFirstQueryParameters < totalFirstQueryParameters)
    {
      differingQueryParameters.push_back(firstQueryParameters[indexInFirstQueryParameters]);
      indexInFirstQueryParameters++;
    }

    while (indexInSecondQueryParameters < totalSecondQueryParameters)
    {
      differingQueryParameters.push_back(secondQueryParameters[indexInSecondQueryParameters]);
      indexInSecondQueryParameters++;
    }

    return differingQueryParameters;
  }

  bool IsEqualIgnoringQueryAndFragments(const URL &a, const URL &b) noexcept
  {
    return SubstringIgnoringQueryAndFragments(a) == SubstringIgnoringQueryAndFragments(b);
  }

  List<utf8_string> RemoveQueryParameters(URL &url, const HashSet<utf8_string> &keysToRemove) noexcept
  {
    if (keysToRemove.empty())
    {
      return {};
    }

    return RemoveQueryParameters(url, [&](const utf8_string &key, const utf8_string &)
                                 { return keysToRemove.contains(key); });
  }

  List<utf8_string> RemoveQueryParameters(
    URL &url, KRYS_NOESCAPE const Func<bool(const utf8_string &, const utf8_string &)> &shouldRemove) noexcept
  {
    if (!url.HasQuery())
    {
      return {};
    }

    List<utf8_string> removedParameters;
    utf8_string queryWithoutRemovalKeys;
    auto query = url.Query();
    size_t start = 0;
    while (start <= query.size())
    {
      size_t end = query.find(u8'&', start);
      if (end == utf8_stringview::npos)
      {
        end = query.size();
      }
      utf8_stringview bytes = query.substr(start, end - start);
      start = end + 1;
      if (bytes.empty())
      {
        continue;
      }

      auto nameAndValue = URLParser::ParseQueryNameAndValue(bytes);
      if (!nameAndValue)
      {
        continue;
      }

      auto &key = nameAndValue->Key;
      if (key.empty())
      {
        continue;
      }

      if (shouldRemove(key, nameAndValue->Value))
      {
        removedParameters.push_back(key);
        continue;
      }

      if (!queryWithoutRemovalKeys.empty())
      {
        queryWithoutRemovalKeys.push_back(u8'&');
      }
      queryWithoutRemovalKeys.append(bytes);
    }

    if (!removedParameters.empty())
    {
      // An empty query is passed as null, which removes the query and its '?'.
      url.SetQuery(queryWithoutRemovalKeys.empty() ? utf8_stringview {}
                                                   : utf8_stringview {queryWithoutRemovalKeys});
    }

    return removedParameters;
  }
}
