#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <limits>
#include <unicode/uidna.h>
#include <utility>

namespace krys::boo::url
{
  namespace
  {
    using text::CodePointIterator;

    enum URLCharacterClass : uint8
    {
      UserInfoEncode = 0x1,
      PathEncode = 0x2,
      ForbiddenHost = 0x4,
      ForbiddenDomain = 0x8,
      QueryEncode = 0x10,
      SlashQuestionOrHash = 0x20,
      ValidScheme = 0x40,
    };

    constexpr Array<uint8, 256> CharacterClassTable = []
    {
      Array<uint8, 256> table {};

      // C0 controls.
      for (size_t c = 0; c <= 0x1F; ++c)
      {
        table[c] = UserInfoEncode | PathEncode | QueryEncode | ForbiddenDomain;
      }
      table[0x0] |= ForbiddenHost;
      table[0x9] |= ForbiddenHost;
      table[0xA] |= ForbiddenHost;
      table[0xD] |= ForbiddenHost;

      table[' '] = UserInfoEncode | PathEncode | QueryEncode | ForbiddenHost | ForbiddenDomain;
      table['"'] = UserInfoEncode | PathEncode | QueryEncode;
      table['#'] =
        UserInfoEncode | PathEncode | QueryEncode | SlashQuestionOrHash | ForbiddenHost | ForbiddenDomain;
      table['%'] = ForbiddenDomain;
      table['+'] = ValidScheme;
      table['-'] = ValidScheme;
      table['.'] = ValidScheme;
      table['/'] = UserInfoEncode | SlashQuestionOrHash | ForbiddenHost | ForbiddenDomain;
      for (size_t c = '0'; c <= '9'; ++c)
      {
        table[c] = ValidScheme;
      }
      table[':'] = UserInfoEncode | ForbiddenHost | ForbiddenDomain;
      table[';'] = UserInfoEncode;
      table['<'] = UserInfoEncode | PathEncode | QueryEncode | ForbiddenHost | ForbiddenDomain;
      table['='] = UserInfoEncode;
      table['>'] = UserInfoEncode | PathEncode | QueryEncode | ForbiddenHost | ForbiddenDomain;
      table['?'] = UserInfoEncode | PathEncode | SlashQuestionOrHash | ForbiddenHost | ForbiddenDomain;
      table['@'] = UserInfoEncode | ForbiddenHost | ForbiddenDomain;
      for (size_t c = 'A'; c <= 'Z'; ++c)
      {
        table[c] = ValidScheme;
      }
      table['['] = UserInfoEncode | ForbiddenHost | ForbiddenDomain;
      table['\\'] = UserInfoEncode | SlashQuestionOrHash | ForbiddenHost | ForbiddenDomain;
      table[']'] = UserInfoEncode | ForbiddenHost | ForbiddenDomain;
      table['^'] = UserInfoEncode | PathEncode | ForbiddenHost | ForbiddenDomain;
      table['`'] = UserInfoEncode | PathEncode;
      for (size_t c = 'a'; c <= 'z'; ++c)
      {
        table[c] = ValidScheme;
      }
      table['{'] = UserInfoEncode | PathEncode;
      table['|'] = UserInfoEncode | ForbiddenHost | ForbiddenDomain;
      table['}'] = UserInfoEncode | PathEncode;
      table[0x7F] = QueryEncode | ForbiddenDomain;
      for (size_t c = 0x80; c <= 0xFF; ++c)
      {
        table[c] = QueryEncode;
      }
      return table;
    }();

    KRYS_ALWAYS_INLINE bool IsC0Control(char32 c) noexcept
    {
      return c <= 0x1F;
    }

    KRYS_ALWAYS_INLINE bool IsC0ControlOrSpace(char32 c) noexcept
    {
      return c <= 0x20;
    }

    KRYS_ALWAYS_INLINE bool IsTabOrNewline(char32 c) noexcept
    {
      return c <= 0xD && c >= 0x9 && c != 0xB && c != 0xC;
    }

    KRYS_ALWAYS_INLINE bool IsInC0ControlEncodeSet(char32 c) noexcept
    {
      return c > 0x7E || IsC0Control(c);
    }

    KRYS_ALWAYS_INLINE bool IsInFragmentEncodeSet(char32 c) noexcept
    {
      return c > 0x7E || c == '`' || ((CharacterClassTable[c] & QueryEncode) && c != '#');
    }

    KRYS_ALWAYS_INLINE bool IsInPathEncodeSet(char32 c) noexcept
    {
      return c > 0x7E || CharacterClassTable[c] & PathEncode;
    }

    KRYS_ALWAYS_INLINE bool IsInUserInfoEncodeSet(char32 c) noexcept
    {
      return c > 0x7E || CharacterClassTable[c] & UserInfoEncode;
    }

    KRYS_ALWAYS_INLINE bool IsPercentOrNonASCII(char32 c) noexcept
    {
      return !text::IsASCII(c) || c == '%';
    }

    KRYS_ALWAYS_INLINE bool IsSlashQuestionOrHash(char32 c) noexcept
    {
      return c <= '\\' && CharacterClassTable[c] & SlashQuestionOrHash;
    }

    KRYS_ALWAYS_INLINE bool IsValidSchemeCharacter(char32 c) noexcept
    {
      return c <= 'z' && CharacterClassTable[c] & ValidScheme;
    }

    KRYS_ALWAYS_INLINE bool IsSpecialCharacterForFragmentDirective(char32 c) noexcept
    {
      return !text::IsASCII(c) || c == ',' || c == '-';
    }

    KRYS_ALWAYS_INLINE bool ShouldPercentEncodeQueryByte(uint8 byte, bool urlIsSpecial) noexcept
    {
      if (CharacterClassTable[byte] & QueryEncode)
      {
        return true;
      }
      if (byte == '\'' && urlIsSpecial)
      {
        return true;
      }
      return false;
    }

    /// @return The number of bytes written (0 for a surrogate or a value above U+10FFFF).
    uint8 EncodeUTF8(char32 codePoint, Array<uint8, 4> &buffer) noexcept
    {
      if (codePoint < 0x80)
      {
        buffer[0] = static_cast<uint8>(codePoint);
        return 1;
      }
      if (codePoint < 0x800)
      {
        buffer[0] = static_cast<uint8>(0xC0 | (codePoint >> 6));
        buffer[1] = static_cast<uint8>(0x80 | (codePoint & 0x3F));
        return 2;
      }
      if ((codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > text::MaxUnicodeChar)
      {
        return 0;
      }
      if (codePoint < 0x10000)
      {
        buffer[0] = static_cast<uint8>(0xE0 | (codePoint >> 12));
        buffer[1] = static_cast<uint8>(0x80 | ((codePoint >> 6) & 0x3F));
        buffer[2] = static_cast<uint8>(0x80 | (codePoint & 0x3F));
        return 3;
      }
      buffer[0] = static_cast<uint8>(0xF0 | (codePoint >> 18));
      buffer[1] = static_cast<uint8>(0x80 | ((codePoint >> 12) & 0x3F));
      buffer[2] = static_cast<uint8>(0x80 | ((codePoint >> 6) & 0x3F));
      buffer[3] = static_cast<uint8>(0x80 | (codePoint & 0x3F));
      return 4;
    }

    void PercentEncodeByte(uint8 byte, utf8_string &buffer) noexcept
    {
      buffer.push_back(u8'%');
      buffer.push_back(static_cast<char8>(text::UpperNibbleToASCIIHexDigit(byte)));
      buffer.push_back(static_cast<char8>(text::LowerNibbleToASCIIHexDigit(byte)));
    }

    constexpr utf8_stringview ReplacementCharacterUTF8PercentEncoded = u8"%EF%BF%BD";

    enum class Scheme
    {
      WS,
      WSS,
      File,
      FTP,
      HTTP,
      HTTPS,
      NonSpecial
    };

    KRYS_ALWAYS_INLINE Scheme SchemeOf(utf8_stringview scheme) noexcept
    {
      if (scheme == u8"ws")
      {
        return Scheme::WS;
      }
      if (scheme == u8"wss")
      {
        return Scheme::WSS;
      }
      if (scheme == u8"file")
      {
        return Scheme::File;
      }
      if (scheme == u8"ftp")
      {
        return Scheme::FTP;
      }
      if (scheme == u8"http")
      {
        return Scheme::HTTP;
      }
      if (scheme == u8"https")
      {
        return Scheme::HTTPS;
      }
      return Scheme::NonSpecial;
    }

    constexpr Array<char8, 2> DotASCIICode {u8'2', u8'e'};

    size_t ZeroSequenceLength(const Array<uint16, 8> &address, size_t begin) noexcept
    {
      size_t end = begin;
      for (; end < 8; end++)
      {
        if (address[end])
        {
          break;
        }
      }
      return end - begin;
    }

    Maybe<size_t> FindLongestZeroSequence(const Array<uint16, 8> &address) noexcept
    {
      Maybe<size_t> longest;
      size_t longestLength = 0;
      for (size_t i = 0; i < 8; i++)
      {
        size_t length = ZeroSequenceLength(address, i);
        if (length)
        {
          if (length > 1 && (!longest || longestLength < length))
          {
            longest = i;
            longestLength = length;
          }
          i += length;
        }
      }
      return longest;
    }

    KRYS_ALWAYS_INLINE uint64 Pow256(size_t exponent) noexcept
    {
      krys_assert(exponent <= 4);
      constexpr Array<uint64, 5> values {1, 256, 256 * 256, 256 * 256 * 256, 256ull * 256 * 256 * 256};
      return values[exponent];
    }

    bool DNSNameEndsInNumber(utf8_stringview name) noexcept
    {
      // https://url.spec.whatwg.org/#ends-in-a-number-checker
      auto containsOctalDecimalOrHexNumber = [](utf8_stringview segment)
      {
        const auto segmentLength = segment.size();
        if (!segmentLength) KRYS_UNLIKELY
        {
          return false;
        }
        auto firstCodeUnit = segment[0];
        if (!text::IsASCIIDigit(firstCodeUnit)) KRYS_LIKELY
        {
          return false;
        }
        if (segmentLength == 1)
        {
          return true;
        }
        auto secondCodeUnit = segment[1];
        if ((secondCodeUnit == 'x' || secondCodeUnit == 'X') && firstCodeUnit == '0')
        {
          return std::ranges::all_of(segment.substr(2), [](char8 c) { return text::IsASCIIHexDigit(c); });
        }
        return std::ranges::all_of(segment, [](char8 c) { return text::IsASCIIDigit(c); });
      };

      size_t lastDotLocation = name.rfind(u8'.');
      if (lastDotLocation == utf8_stringview::npos)
      {
        return containsOctalDecimalOrHexNumber(name);
      }
      size_t lastSegmentEnd = name.size();
      if (lastDotLocation == lastSegmentEnd - 1)
      {
        lastSegmentEnd = lastDotLocation;
        lastDotLocation = lastDotLocation ? name.rfind(u8'.', lastDotLocation - 1) : utf8_stringview::npos;
      }
      size_t lastPartStart = lastDotLocation == utf8_stringview::npos ? 0 : lastDotLocation + 1;
      utf8_stringview lastPart = name.substr(lastPartStart, lastSegmentEnd - lastPartStart);
      return containsOctalDecimalOrHexNumber(lastPart);
    }

    utf8_string ReplaceAll(utf8_stringview string, char8 from, char8 to) noexcept
    {
      utf8_string result {string};
      std::ranges::replace(result, from, to);
      return result;
    }

    /// @brief Decodes `bytes` as UTF-8, replacing each invalid byte with U+FFFD.
    utf8_string ReplaceInvalidUTF8(utf8_stringview bytes) noexcept
    {
      if (text::IsValidUTF8(bytes))
      {
        return utf8_string {bytes};
      }
      utf8_string result;
      result.reserve(bytes.size());
      for (CodePointIterator it {bytes}; !it.AtEnd(); ++it)
      {
        Array<uint8, 4> buffer;
        uint8 length = EncodeUTF8(*it, buffer);
        result.append(reinterpret_cast<const char8 *>(buffer.data()), length);
      }
      return result;
    }

    void SerialiseURLEncodedForm(utf8_stringview input, utf8_string &output) noexcept
    {
      for (char8 byte : input)
      {
        if (byte == 0x20)
        {
          output.push_back(0x2B);
        }
        else if (byte == 0x2A || byte == 0x2D || byte == 0x2E || (byte >= 0x30 && byte <= 0x39)
                 || (byte >= 0x41 && byte <= 0x5A) || byte == 0x5F || (byte >= 0x61 && byte <= 0x7A))
        {
          // FIXME: Put these in the CharacterClassTable to avoid branches.
          output.push_back(byte);
        }
        else
        {
          PercentEncodeByte(byte, output);
        }
      }
    }
  }

  const uint32 URLParser::AllowedNameToASCIIErrors =
    UIDNA_ERROR_EMPTY_LABEL | UIDNA_ERROR_LABEL_TOO_LONG | UIDNA_ERROR_DOMAIN_NAME_TOO_LONG
    | UIDNA_ERROR_LEADING_HYPHEN | UIDNA_ERROR_TRAILING_HYPHEN | UIDNA_ERROR_HYPHEN_3_4;

  bool IsForbiddenHostCodePoint(char32 c) noexcept
  {
    return c <= 0x7F && CharacterClassTable[c] & ForbiddenHost;
  }

  bool URLParser::IsForbiddenHostCodePoint(char32 c) const noexcept
  {
    krys_debug_assert(!_urlIsSpecial);
    return url::IsForbiddenHostCodePoint(c);
  }

  bool URLParser::IsForbiddenDomainCodePoint(char32 c) const noexcept
  {
    krys_debug_assert(_urlIsSpecial);
    return c <= 0x7F && CharacterClassTable[c] & ForbiddenDomain;
  }

  bool URLParser::IsInUserInfoEncodeSet(char32 c) noexcept
  {
    return url::IsInUserInfoEncodeSet(c);
  }

  bool URLParser::IsSpecialCharacterForFragmentDirective(char32 c) noexcept
  {
    return url::IsSpecialCharacterForFragmentDirective(c);
  }

  template <URLParser::ReportSyntaxViolation reportSyntaxViolation>
  void URLParser::Advance(CodePointIterator &iterator,
                          const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept
  {
    ++iterator;
    while (!iterator.AtEnd() && IsTabOrNewline(*iterator)) KRYS_UNLIKELY
    {
      if constexpr (reportSyntaxViolation == ReportSyntaxViolation::Yes)
      {
        SyntaxViolation(iteratorForSyntaxViolationPosition);
      }
      ++iterator;
    }
  }

  bool URLParser::TakesTwoAdvancesUntilEnd(CodePointIterator iterator) noexcept
  {
    if (iterator.AtEnd())
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(iterator);
    if (iterator.AtEnd())
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(iterator);
    return iterator.AtEnd();
  }

  bool URLParser::IsWindowsDriveLetter(CodePointIterator iterator) noexcept
  {
    // https://url.spec.whatwg.org/#start-with-a-windows-drive-letter
    if (iterator.AtEnd() || !text::IsASCIIAlpha(*iterator))
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(iterator);
    if (iterator.AtEnd())
    {
      return false;
    }
    if (*iterator != ':' && *iterator != '|')
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(iterator);
    return iterator.AtEnd() || *iterator == '/' || *iterator == '\\' || *iterator == '?' || *iterator == '#';
  }

  KRYS_ALWAYS_INLINE void URLParser::AppendToASCIIBuffer(char32 codePoint) noexcept
  {
    krys_debug_assert(text::IsASCII(codePoint));
    if (_didSeeSyntaxViolation) KRYS_UNLIKELY
    {
      _asciiBuffer.push_back(static_cast<char8>(codePoint));
    }
  }

  KRYS_ALWAYS_INLINE void URLParser::AppendToASCIIBuffer(utf8_stringview characters) noexcept
  {
    if (_didSeeSyntaxViolation) KRYS_UNLIKELY
    {
      _asciiBuffer.append(characters);
    }
  }

  void URLParser::AppendWindowsDriveLetter(CodePointIterator &iterator) noexcept
  {
    auto lengthWithOnlyOneSlashInPath = _url._hostEnd + _url._portLength + 1;
    if (_url._pathAfterLastSlash > lengthWithOnlyOneSlashInPath)
    {
      SyntaxViolation(iterator);
      _url._pathAfterLastSlash = lengthWithOnlyOneSlashInPath;
      _asciiBuffer.resize(lengthWithOnlyOneSlashInPath);
    }
    krys_debug_assert(IsWindowsDriveLetter(iterator));
    AppendToASCIIBuffer(*iterator);
    Advance(iterator);
    krys_debug_assert(!iterator.AtEnd());
    krys_debug_assert(*iterator == ':' || *iterator == '|');
    if (*iterator == '|')
    {
      SyntaxViolation(iterator);
    }
    AppendToASCIIBuffer(':');
    Advance(iterator);
  }

  bool URLParser::CopyBaseWindowsDriveLetter(const URL &base) noexcept
  {
    if (base.ProtocolIsFile())
    {
      krys_assert(base._hostEnd + base._portLength < base._string.size());
      CodePointIterator c {utf8_stringview {base._string}.substr(base._hostEnd + base._portLength + 1)};
      if (IsWindowsDriveLetter(c))
      {
        AppendWindowsDriveLetter(c);
        return true;
      }
    }
    return false;
  }

  bool URLParser::ShouldCopyFileURL(CodePointIterator iterator) noexcept
  {
    if (!IsWindowsDriveLetter(iterator))
    {
      return true;
    }
    if (iterator.AtEnd())
    {
      return false;
    }
    Advance(iterator);
    if (iterator.AtEnd())
    {
      return true;
    }
    Advance(iterator);
    if (iterator.AtEnd())
    {
      return true;
    }
    return !IsSlashQuestionOrHash(*iterator);
  }

  void URLParser::PercentEncodeByte(uint8 byte) noexcept
  {
    krys_debug_assert(_didSeeSyntaxViolation);
    AppendToASCIIBuffer('%');
    AppendToASCIIBuffer(static_cast<char32>(text::UpperNibbleToASCIIHexDigit(byte)));
    AppendToASCIIBuffer(static_cast<char32>(text::LowerNibbleToASCIIHexDigit(byte)));
  }

  template <bool (*IsInCodeSet)(char32)>
  KRYS_ALWAYS_INLINE void URLParser::UTF8PercentEncode(const CodePointIterator &iterator) noexcept
  {
    krys_debug_assert(!iterator.AtEnd());
    char32 codePoint = *iterator;
    if (text::IsASCII(codePoint)) KRYS_LIKELY
    {
      if (IsInCodeSet(codePoint)) KRYS_UNLIKELY
      {
        SyntaxViolation(iterator);
        PercentEncodeByte(static_cast<uint8>(codePoint));
      }
      else
      {
        AppendToASCIIBuffer(codePoint);
      }
      return;
    }
    krys_debug_assert_msg(IsInCodeSet(codePoint),
                          "IsInCodeSet should always return true for non-ASCII characters");
    SyntaxViolation(iterator);

    Array<uint8, 4> buffer;
    uint8 length = EncodeUTF8(codePoint, buffer);
    if (!length)
    {
      AppendToASCIIBuffer(ReplacementCharacterUTF8PercentEncoded);
      return;
    }
    for (uint8 i = 0; i < length; ++i)
    {
      PercentEncodeByte(buffer[i]);
    }
  }

  KRYS_ALWAYS_INLINE void URLParser::UTF8QueryEncode(const CodePointIterator &iterator) noexcept
  {
    krys_debug_assert(!iterator.AtEnd());
    char32 codePoint = *iterator;
    if (text::IsASCII(codePoint)) KRYS_LIKELY
    {
      if (ShouldPercentEncodeQueryByte(static_cast<uint8>(codePoint), _urlIsSpecial)) KRYS_UNLIKELY
      {
        SyntaxViolation(iterator);
        PercentEncodeByte(static_cast<uint8>(codePoint));
      }
      else
      {
        AppendToASCIIBuffer(codePoint);
      }
      return;
    }

    SyntaxViolation(iterator);

    Array<uint8, 4> buffer;
    uint8 length = EncodeUTF8(codePoint, buffer);
    if (!length)
    {
      AppendToASCIIBuffer(ReplacementCharacterUTF8PercentEncoded);
      return;
    }
    for (uint8 i = 0; i < length; ++i)
    {
      auto byte = buffer[i];
      if (ShouldPercentEncodeQueryByte(byte, _urlIsSpecial))
      {
        PercentEncodeByte(byte);
      }
      else
      {
        AppendToASCIIBuffer(static_cast<char32>(byte));
      }
    }
  }

  Maybe<uint16> URLParser::DefaultPortForProtocol(utf8_stringview scheme) noexcept
  {
    constexpr uint16 ftpPort = 21;
    constexpr uint16 httpPort = 80;
    constexpr uint16 httpsPort = 443;
    constexpr uint16 wsPort = 80;
    constexpr uint16 wssPort = 443;

    switch (SchemeOf(scheme))
    {
      case Scheme::WS:         return wsPort;
      case Scheme::WSS:        return wssPort;
      case Scheme::HTTP:       return httpPort;
      case Scheme::HTTPS:      return httpsPort;
      case Scheme::FTP:        return ftpPort;
      case Scheme::File:
      case Scheme::NonSpecial: return null;
    }
    return null;
  }

  Maybe<utf8_string> URLParser::MaybeCanonicaliseScheme(utf8_stringview scheme) noexcept
  {
    if (scheme.empty())
    {
      return null;
    }

    size_t i = 0;
    while (i < scheme.size() && IsTabOrNewline(scheme[i]))
    {
      ++i;
    }

    if (i >= scheme.size() || !text::IsASCIIAlpha(scheme[i++]))
    {
      return null;
    }

    for (; i < scheme.size(); ++i)
    {
      if (text::IsASCIIAlphanumeric(scheme[i]) || scheme[i] == '+' || scheme[i] == '-' || scheme[i] == '.'
          || IsTabOrNewline(scheme[i]))
      {
        continue;
      }
      return null;
    }

    utf8_string result;
    result.reserve(scheme.size());
    for (char8 c : scheme)
    {
      if (!IsTabOrNewline(c))
      {
        result.push_back(text::ToASCIILower(c));
      }
    }
    return result;
  }

  bool URLParser::IsSpecialScheme(utf8_stringview scheme) noexcept
  {
    return SchemeOf(scheme) != Scheme::NonSpecial;
  }

  enum class URLParser::URLPart
  {
    SchemeEnd,
    UserStart,
    UserEnd,
    PasswordEnd,
    HostEnd,
    PortEnd,
    PathAfterLastSlash,
    PathEnd,
    QueryEnd,
  };

  size_t URLParser::URLLengthUntilPart(const URL &url, URLPart part) noexcept
  {
    switch (part)
    {
      case URLPart::QueryEnd:           return url._queryEnd;
      case URLPart::PathEnd:            return url._pathEnd;
      case URLPart::PathAfterLastSlash: return url._pathAfterLastSlash;
      case URLPart::PortEnd:            return url._hostEnd + url._portLength;
      case URLPart::HostEnd:            return url._hostEnd;
      case URLPart::PasswordEnd:        return url._passwordEnd;
      case URLPart::UserEnd:            return url._userEnd;
      case URLPart::UserStart:          return url._userStart;
      case URLPart::SchemeEnd:          return url._schemeEnd;
    }
    krys_debug_assert_msg(false, "Unknown URL part.");
    return 0;
  }

  void URLParser::CopyASCIIStringUntil(const utf8_string &string, size_t length) noexcept
  {
    krys_assert(length <= string.size());
    krys_debug_assert(_asciiBuffer.empty());
    AppendToASCIIBuffer(utf8_stringview {string}.substr(0, length));
  }

  void URLParser::CopyURLPartsUntil(const URL &base, URLPart part, const CodePointIterator &iterator) noexcept
  {
    SyntaxViolation(iterator);

    _asciiBuffer.clear();
    CopyASCIIStringUntil(base._string, URLLengthUntilPart(base, part));
    switch (part)
    {
      case URLPart::QueryEnd:           _url._queryEnd = base._queryEnd; [[fallthrough]];
      case URLPart::PathEnd:            _url._pathEnd = base._pathEnd; [[fallthrough]];
      case URLPart::PathAfterLastSlash: _url._pathAfterLastSlash = base._pathAfterLastSlash; [[fallthrough]];
      case URLPart::PortEnd:            _url._portLength = base._portLength; [[fallthrough]];
      case URLPart::HostEnd:            _url._hostEnd = base._hostEnd; [[fallthrough]];
      case URLPart::PasswordEnd:        _url._passwordEnd = base._passwordEnd; [[fallthrough]];
      case URLPart::UserEnd:            _url._userEnd = base._userEnd; [[fallthrough]];
      case URLPart::UserStart:          _url._userStart = base._userStart; [[fallthrough]];
      case URLPart::SchemeEnd:
        _url._isValid = base._isValid;
        _url._protocolIsInHTTPFamily = base._protocolIsInHTTPFamily;
        _url._schemeEnd = base._schemeEnd;
    }

    switch (SchemeOf(utf8_stringview {_asciiBuffer}.substr(0, _url._schemeEnd)))
    {
      case Scheme::WS:
      case Scheme::WSS:   _urlIsSpecial = true; return;
      case Scheme::File:  _urlIsFile = true; [[fallthrough]];
      case Scheme::FTP:
      case Scheme::HTTP:
      case Scheme::HTTPS: _urlIsSpecial = true; return;
      case Scheme::NonSpecial:
        _urlIsSpecial = false;
        auto pathStart = _url._hostEnd + _url._portLength;
        if (pathStart + 2 < _asciiBuffer.size() && _asciiBuffer[pathStart] == '/'
            && _asciiBuffer[pathStart + 1] == '.' && _asciiBuffer[pathStart + 2] == '/')
        {
          _asciiBuffer.erase(pathStart + 1, 2);
          _url._pathAfterLastSlash = std::max(2u, _url._pathAfterLastSlash) - 2;
          _url._pathEnd = std::max(2u, _url._pathEnd) - 2;
          _url._queryEnd = std::max(2u, _url._queryEnd) - 2;
        }
        return;
    }
  }

  KRYS_ALWAYS_INLINE bool URLParser::IsSingleDotPathSegment(CodePointIterator c) noexcept
  {
    if (c.AtEnd())
    {
      return false;
    }
    if (*c == '.')
    {
      Advance<ReportSyntaxViolation::No>(c);
      return c.AtEnd() || IsSlashQuestionOrHash(*c);
    }
    if (*c != '%')
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(c);
    if (c.AtEnd() || *c != DotASCIICode[0])
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(c);
    if (c.AtEnd())
    {
      return false;
    }
    if (text::IsASCIIAlphaCaselessEqual(static_cast<char8>(*c), DotASCIICode[1]))
    {
      Advance<ReportSyntaxViolation::No>(c);
      return c.AtEnd() || IsSlashQuestionOrHash(*c);
    }
    return false;
  }

  KRYS_ALWAYS_INLINE bool URLParser::IsDoubleDotPathSegment(CodePointIterator c) noexcept
  {
    if (c.AtEnd())
    {
      return false;
    }
    if (*c == '.')
    {
      Advance<ReportSyntaxViolation::No>(c);
      return IsSingleDotPathSegment(c);
    }
    if (*c != '%')
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(c);
    if (c.AtEnd() || *c != DotASCIICode[0])
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(c);
    if (c.AtEnd())
    {
      return false;
    }
    if (text::IsASCIIAlphaCaselessEqual(static_cast<char8>(*c), DotASCIICode[1]))
    {
      Advance<ReportSyntaxViolation::No>(c);
      return IsSingleDotPathSegment(c);
    }
    return false;
  }

  void URLParser::ConsumeSingleDotPathSegment(CodePointIterator &c) noexcept
  {
    krys_debug_assert(IsSingleDotPathSegment(c));
    if (*c == '.')
    {
      Advance(c);
      if (!c.AtEnd())
      {
        if (*c == '/' || *c == '\\')
        {
          Advance(c);
        }
        else
        {
          krys_debug_assert(*c == '?' || *c == '#');
        }
      }
    }
    else
    {
      krys_debug_assert(*c == '%');
      Advance(c);
      krys_debug_assert(*c == DotASCIICode[0]);
      Advance(c);
      krys_debug_assert(text::IsASCIIAlphaCaselessEqual(static_cast<char8>(*c), DotASCIICode[1]));
      Advance(c);
      if (!c.AtEnd())
      {
        if (*c == '/' || *c == '\\')
        {
          Advance(c);
        }
        else
        {
          krys_debug_assert(*c == '?' || *c == '#');
        }
      }
    }
  }

  void URLParser::ConsumeDoubleDotPathSegment(CodePointIterator &c) noexcept
  {
    krys_debug_assert(IsDoubleDotPathSegment(c));
    if (*c == '.')
    {
      Advance(c);
    }
    else
    {
      krys_debug_assert(*c == '%');
      Advance(c);
      krys_debug_assert(*c == DotASCIICode[0]);
      Advance(c);
      krys_debug_assert(text::IsASCIIAlphaCaselessEqual(static_cast<char8>(*c), DotASCIICode[1]));
      Advance(c);
    }
    ConsumeSingleDotPathSegment(c);
  }

  bool URLParser::ShouldPopPath(uint32 newPathAfterLastSlash) noexcept
  {
    krys_debug_assert(_didSeeSyntaxViolation);
    if (!_urlIsFile)
    {
      return true;
    }

    krys_debug_assert(_url._pathAfterLastSlash <= _asciiBuffer.size());
    CodePointIterator componentToPop {utf8_stringview {_asciiBuffer}.substr(
      newPathAfterLastSlash, _url._pathAfterLastSlash - newPathAfterLastSlash)};
    if (newPathAfterLastSlash == _url._hostEnd + _url._portLength + 1 && IsWindowsDriveLetter(componentToPop))
    {
      return false;
    }
    return true;
  }

  void URLParser::PopPath() noexcept
  {
    krys_debug_assert(_didSeeSyntaxViolation);
    if (_url._pathAfterLastSlash > _url._hostEnd + _url._portLength + 1)
    {
      auto newPathAfterLastSlash = _url._pathAfterLastSlash - 1;
      if (_asciiBuffer[newPathAfterLastSlash] == '/')
      {
        newPathAfterLastSlash--;
      }
      while (newPathAfterLastSlash > _url._hostEnd + _url._portLength
             && _asciiBuffer[newPathAfterLastSlash] != '/')
      {
        newPathAfterLastSlash--;
      }
      newPathAfterLastSlash++;
      if (ShouldPopPath(newPathAfterLastSlash))
      {
        _url._pathAfterLastSlash = newPathAfterLastSlash;
      }
    }
    _asciiBuffer.resize(_url._pathAfterLastSlash);
  }

  void URLParser::SyntaxViolation(const CodePointIterator &iterator) noexcept
  {
    if (_didSeeSyntaxViolation)
    {
      return;
    }
    _didSeeSyntaxViolation = true;

    krys_debug_assert(_asciiBuffer.empty());
    size_t codeUnitsToCopy = iterator.CodeUnitsSince(_inputBegin);
    krys_assert(codeUnitsToCopy <= _input.size());
    _asciiBuffer.append(_input.substr(0, codeUnitsToCopy));
  }

  void URLParser::Failure() noexcept
  {
    _url.Invalidate();
    _url._string = utf8_string {_input};
    _url._isNull = false;
  }

  bool URLParser::CheckLocalhostCodePoint(CodePointIterator &iterator, char32 codePoint) noexcept
  {
    if (iterator.AtEnd() || text::ToASCIILower(*iterator) != codePoint)
    {
      return false;
    }
    Advance<ReportSyntaxViolation::No>(iterator);
    return true;
  }

  bool URLParser::IsAtLocalhost(CodePointIterator iterator) noexcept
  {
    for (char32 c : {U'l', U'o', U'c', U'a', U'l', U'h', U'o', U's', U't'})
    {
      if (!CheckLocalhostCodePoint(iterator, c))
      {
        return false;
      }
    }
    return iterator.AtEnd();
  }

  bool URLParser::IsLocalhost(utf8_stringview view) noexcept
  {
    return IsAtLocalhost(CodePointIterator {view});
  }

  KRYS_ALWAYS_INLINE utf8_stringview URLParser::ParsedDataView(size_t start, size_t length) const noexcept
  {
    if (_didSeeSyntaxViolation) KRYS_UNLIKELY
    {
      krys_debug_assert(start + length <= _asciiBuffer.size());
      return utf8_stringview {_asciiBuffer}.substr(start, length);
    }
    krys_debug_assert(start + length <= _input.size());
    return _input.substr(start, length);
  }

  KRYS_ALWAYS_INLINE char8 URLParser::ParsedDataView(size_t position) const noexcept
  {
    if (_didSeeSyntaxViolation) KRYS_UNLIKELY
    {
      return _asciiBuffer[position];
    }
    return _input[position];
  }

  KRYS_ALWAYS_INLINE size_t URLParser::CurrentPosition(const CodePointIterator &iterator) const noexcept
  {
    if (_didSeeSyntaxViolation) KRYS_UNLIKELY
    {
      return _asciiBuffer.size();
    }

    return iterator.CodeUnitsSince(_inputBegin);
  }

  URLParser::URLParser(utf8_stringview input, const URL &base, bool allowC0AtEnd) noexcept : _input(input)
  {
    if (!_input.data())
    {
      if (base.IsValid() && !base._hasOpaquePath)
      {
        _url = base;
        _url.RemoveFragmentIdentifier();
      }
      return;
    }

    _inputBegin = _input.data();
    Parse(_input, base, allowC0AtEnd);

    krys_debug_assert(!_url._isValid || _didSeeSyntaxViolation == (_url.String() != _input)
                      || (std::ranges::all_of(_input, [](char8 c) { return IsC0ControlOrSpace(c); })
                          && _url._string == utf8_stringview {base._string}.substr(0, base._queryEnd))
                      || (base.IsValid() && base.ProtocolIsFile()));
    krys_debug_assert(InternalValuesConsistent(_url));
#if KRYS_DEBUG
    if (!_didSeeSyntaxViolation)
    {
      // Force a syntax violation at the beginning to make sure we get the same result.
      utf8_string withLeadingSpace {u8" "};
      withLeadingSpace.append(_input);
      URLParser parser(withLeadingSpace, base, allowC0AtEnd);
      URL parsed = parser.Result();
      if (parsed.IsValid())
      {
        krys_debug_assert(AllValuesEqual(parsed, _url));
      }
    }
#endif

    if (NeedsNonSpecialDotSlash()) KRYS_UNLIKELY
    {
      AddNonSpecialDotSlash();
    }
  }

  void URLParser::Parse(utf8_stringview input, const URL &base, bool allowC0AtEnd) noexcept
  {
    _url = {};
    krys_debug_assert(_asciiBuffer.empty());

    auto endIndex = input.size();
    if (!allowC0AtEnd) KRYS_LIKELY
    {
      while (endIndex && IsC0ControlOrSpace(input[endIndex - 1])) KRYS_UNLIKELY
      {
        SyntaxViolation(CodePointIterator {input});
        endIndex--;
      }
    }
    CodePointIterator c {input.substr(0, endIndex)};
    CodePointIterator authorityOrHostBegin;
    CodePointIterator queryBegin;
    while (!c.AtEnd() && IsC0ControlOrSpace(*c)) KRYS_UNLIKELY
    {
      SyntaxViolation(c);
      ++c;
    }
    auto beginAfterControlAndSpace = c;

    enum class State : uint8
    {
      SchemeStart,
      Scheme,
      NoScheme,
      SpecialRelativeOrAuthority,
      PathOrAuthority,
      Relative,
      RelativeSlash,
      SpecialAuthoritySlashes,
      SpecialAuthorityIgnoreSlashes,
      AuthorityOrHost,
      Host,
      File,
      FileSlash,
      FileHost,
      FilePathStart,
      PathStart,
      Path,
      OpaquePath,
      UTF8Query,
      Fragment,
    };

    State state = State::SchemeStart;
    while (!c.AtEnd())
    {
      if (IsTabOrNewline(*c)) KRYS_UNLIKELY
      {
        SyntaxViolation(c);
        ++c;
        continue;
      }

      switch (state)
      {
        case State::SchemeStart:
          if (text::IsASCIIAlpha(*c))
          {
            if (text::IsASCIIUpper(*c)) KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            AppendToASCIIBuffer(text::ToASCIILower(*c));
            Advance(c);
            if (c.AtEnd())
            {
              _asciiBuffer.clear();
              state = State::NoScheme;
              c = beginAfterControlAndSpace;
              break;
            }
            state = State::Scheme;
          }
          else
          {
            state = State::NoScheme;
          }
          break;
        case State::Scheme:
          if (IsValidSchemeCharacter(*c))
          {
            if (text::IsASCIIUpper(*c)) KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            AppendToASCIIBuffer(text::ToASCIILower(*c));
          }
          else if (*c == ':')
          {
            size_t schemeEnd = CurrentPosition(c);
            if (schemeEnd > URL::MaxSchemeLength)
            {
              Failure();
              return;
            }
            _url._schemeEnd = static_cast<uint32>(schemeEnd);
            AppendToASCIIBuffer(':');
            utf8_stringview urlScheme = ParsedDataView(0, _url._schemeEnd);
            switch (SchemeOf(urlScheme))
            {
              case Scheme::File:
                _urlIsSpecial = true;
                _urlIsFile = true;
                state = State::File;
                ++c;
                break;
              case Scheme::WS:
              case Scheme::WSS:
                _urlIsSpecial = true;
                if (base.ProtocolIs(urlScheme))
                {
                  state = State::SpecialRelativeOrAuthority;
                }
                else
                {
                  state = State::SpecialAuthoritySlashes;
                }
                ++c;
                break;
              case Scheme::HTTP:
              case Scheme::HTTPS: _url._protocolIsInHTTPFamily = true; [[fallthrough]];
              case Scheme::FTP:
                _urlIsSpecial = true;
                if (base.ProtocolIs(urlScheme))
                {
                  state = State::SpecialRelativeOrAuthority;
                }
                else
                {
                  state = State::SpecialAuthoritySlashes;
                }
                ++c;
                break;
              case Scheme::NonSpecial:
                auto maybeSlash = c;
                Advance(maybeSlash);
                if (!maybeSlash.AtEnd() && *maybeSlash == '/')
                {
                  AppendToASCIIBuffer('/');
                  c = maybeSlash;
                  state = State::PathOrAuthority;
                  krys_debug_assert(*c == '/');
                  ++c;
                  _url._userStart = static_cast<uint32>(CurrentPosition(c));
                }
                else
                {
                  ++c;
                  _url._userStart = static_cast<uint32>(CurrentPosition(c));
                  _url._userEnd = _url._userStart;
                  _url._passwordEnd = _url._userStart;
                  _url._hostEnd = _url._userStart;
                  _url._portLength = 0;
                  _url._pathAfterLastSlash = _url._userStart;
                  _url._hasOpaquePath = true;
                  state = State::OpaquePath;
                }
                break;
            }
            break;
          }
          else
          {
            _asciiBuffer.clear();
            state = State::NoScheme;
            c = beginAfterControlAndSpace;
            break;
          }
          Advance(c);
          if (c.AtEnd())
          {
            _asciiBuffer.clear();
            state = State::NoScheme;
            c = beginAfterControlAndSpace;
          }
          break;
        case State::NoScheme:
          if (!base.IsValid() || (base._hasOpaquePath && *c != '#'))
          {
            Failure();
            return;
          }
          if (base._hasOpaquePath && *c == '#')
          {
            CopyURLPartsUntil(base, URLPart::QueryEnd, c);
            state = State::Fragment;
            AppendToASCIIBuffer('#');
            ++c;
            break;
          }
          if (!base.ProtocolIsFile())
          {
            state = State::Relative;
            break;
          }
          state = State::File;
          break;
        case State::SpecialRelativeOrAuthority:
          if (*c == '/')
          {
            AppendToASCIIBuffer('/');
            Advance(c);
            if (c.AtEnd())
            {
              Failure();
              return;
            }
            if (*c == '/')
            {
              AppendToASCIIBuffer('/');
              state = State::SpecialAuthorityIgnoreSlashes;
              ++c;
            }
            else
            {
              state = State::RelativeSlash;
            }
          }
          else
          {
            state = State::Relative;
          }
          break;
        case State::PathOrAuthority:
          if (*c == '/')
          {
            AppendToASCIIBuffer('/');
            state = State::AuthorityOrHost;
            Advance(c);
            _url._userStart = static_cast<uint32>(CurrentPosition(c));
            authorityOrHostBegin = c;
          }
          else
          {
            krys_debug_assert(ParsedDataView(CurrentPosition(c) - 1) == '/');
            _url._userStart = static_cast<uint32>(CurrentPosition(c) - 1);
            _url._userEnd = _url._userStart;
            _url._passwordEnd = _url._userStart;
            _url._hostEnd = _url._userStart;
            _url._portLength = 0;
            _url._pathAfterLastSlash = _url._userStart + 1;
            state = State::Path;
          }
          break;
        case State::Relative:
          switch (*c)
          {
            case '/':
            case '\\':
              state = State::RelativeSlash;
              ++c;
              break;
            case '?':
              CopyURLPartsUntil(base, URLPart::PathEnd, c);
              AppendToASCIIBuffer('?');
              ++c;
              state = State::UTF8Query;
              break;
            case '#':
              CopyURLPartsUntil(base, URLPart::QueryEnd, c);
              AppendToASCIIBuffer('#');
              state = State::Fragment;
              ++c;
              break;
            default:
              CopyURLPartsUntil(base, URLPart::PathAfterLastSlash, c);
              if ((CurrentPosition(c) && ParsedDataView(CurrentPosition(c) - 1) != '/')
                  || (base.Host().empty() && base.Path().empty()))
              {
                AppendToASCIIBuffer('/');
                _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
              }
              state = State::Path;
              break;
          }
          break;
        case State::RelativeSlash:
          if (*c == '/' || *c == '\\')
          {
            ++c;
            CopyURLPartsUntil(base, URLPart::SchemeEnd, c);
            AppendToASCIIBuffer(u8"://");
            if (_urlIsSpecial)
            {
              state = State::SpecialAuthorityIgnoreSlashes;
            }
            else
            {
              _url._userStart = static_cast<uint32>(CurrentPosition(c));
              state = State::AuthorityOrHost;
              authorityOrHostBegin = c;
            }
          }
          else
          {
            CopyURLPartsUntil(base, URLPart::PortEnd, c);
            AppendToASCIIBuffer('/');
            _url._pathAfterLastSlash = base._hostEnd + base._portLength + 1;
            state = State::Path;
          }
          break;
        case State::SpecialAuthoritySlashes:
          if (*c == '/' || *c == '\\') KRYS_LIKELY
          {
            if (*c == '\\') KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            AppendToASCIIBuffer('/');
            Advance(c);
            if (!c.AtEnd() && (*c == '/' || *c == '\\')) KRYS_LIKELY
            {
              if (*c == '\\') KRYS_UNLIKELY
              {
                SyntaxViolation(c);
              }
              ++c;
              AppendToASCIIBuffer('/');
            }
            else
            {
              SyntaxViolation(c);
              AppendToASCIIBuffer('/');
            }
          }
          else
          {
            SyntaxViolation(c);
            AppendToASCIIBuffer(u8"//");
          }
          state = State::SpecialAuthorityIgnoreSlashes;
          break;
        case State::SpecialAuthorityIgnoreSlashes:
          if (*c == '/' || *c == '\\')
          {
            SyntaxViolation(c);
            ++c;
          }
          else
          {
            _url._userStart = static_cast<uint32>(CurrentPosition(c));
            state = State::AuthorityOrHost;
            authorityOrHostBegin = c;
          }
          break;
        case State::AuthorityOrHost:
          do
          {
            if (*c == '@')
            {
              auto lastAt = c;
              auto findLastAt = c;
              while (!findLastAt.AtEnd())
              {
                if (*findLastAt == '@')
                {
                  lastAt = findLastAt;
                }
                bool isSlash = *findLastAt == '/' || (_urlIsSpecial && *findLastAt == '\\');
                if (isSlash || *findLastAt == '?' || *findLastAt == '#')
                {
                  break;
                }
                ++findLastAt;
              }
              ParseAuthority(CodePointIterator(authorityOrHostBegin, lastAt));
              c = lastAt;
              Advance(c);
              authorityOrHostBegin = c;
              state = State::Host;
              _hostHasPercentOrNonASCII = false;
              break;
            }
            bool isSlash = *c == '/' || (_urlIsSpecial && *c == '\\');
            if (isSlash || *c == '?' || *c == '#')
            {
              auto iterator = CodePointIterator(authorityOrHostBegin, c);
              if (iterator.AtEnd())
              {
                if (_urlIsSpecial)
                {
                  return Failure();
                }
                _url._userEnd = static_cast<uint32>(CurrentPosition(c));
                _url._passwordEnd = _url._userEnd;
                _url._hostEnd = _url._userEnd;
                _url._portLength = 0;
                _url._pathAfterLastSlash = _url._userEnd;
              }
              else
              {
                _url._userEnd = static_cast<uint32>(CurrentPosition(authorityOrHostBegin));
                _url._passwordEnd = _url._userEnd;
                if (ParseHostAndPort(iterator) == HostParsingResult::InvalidHost)
                {
                  Failure();
                  return;
                }
                if (!isSlash) KRYS_UNLIKELY
                {
                  if (_urlIsSpecial)
                  {
                    SyntaxViolation(c);
                    AppendToASCIIBuffer('/');
                  }
                  _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
                }
              }
              state = State::Path;
              break;
            }
            if (IsPercentOrNonASCII(*c))
            {
              _hostHasPercentOrNonASCII = true;
            }
            ++c;
          } while (!c.AtEnd());
          break;
        case State::Host:
          do
          {
            bool isSlash = *c == '/' || (_urlIsSpecial && *c == '\\');
            if (isSlash || *c == '?' || *c == '#')
            {
              if (ParseHostAndPort(CodePointIterator(authorityOrHostBegin, c))
                  == HostParsingResult::InvalidHost)
              {
                Failure();
                return;
              }
              if (*c == '?' || *c == '#')
              {
                SyntaxViolation(c);
                AppendToASCIIBuffer('/');
                _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
              }
              state = State::Path;
              break;
            }
            if (IsPercentOrNonASCII(*c))
            {
              _hostHasPercentOrNonASCII = true;
            }
            ++c;
          } while (!c.AtEnd());
          break;
        case State::File:
          switch (*c)
          {
            case '\\': SyntaxViolation(c); [[fallthrough]];
            case '/':
              AppendToASCIIBuffer('/');
              state = State::FileSlash;
              ++c;
              break;
            case '?':
              SyntaxViolation(c);
              if (base.IsValid() && base.ProtocolIsFile())
              {
                CopyURLPartsUntil(base, URLPart::PathEnd, c);
                AppendToASCIIBuffer('?');
                ++c;
              }
              else
              {
                AppendToASCIIBuffer(u8"///?");
                ++c;
                _url._userStart = static_cast<uint32>(CurrentPosition(c) - 2);
                _url._userEnd = _url._userStart;
                _url._passwordEnd = _url._userStart;
                _url._hostEnd = _url._userStart;
                _url._portLength = 0;
                _url._pathAfterLastSlash = _url._userStart + 1;
                _url._pathEnd = _url._pathAfterLastSlash;
              }
              state = State::UTF8Query;
              break;
            case '#':
              SyntaxViolation(c);
              if (base.IsValid() && base.ProtocolIsFile())
              {
                CopyURLPartsUntil(base, URLPart::QueryEnd, c);
                AppendToASCIIBuffer('#');
              }
              else
              {
                AppendToASCIIBuffer(u8"///#");
                _url._userStart = static_cast<uint32>(CurrentPosition(c) - 2);
                _url._userEnd = _url._userStart;
                _url._passwordEnd = _url._userStart;
                _url._hostEnd = _url._userStart;
                _url._portLength = 0;
                _url._pathAfterLastSlash = _url._userStart + 1;
                _url._pathEnd = _url._pathAfterLastSlash;
                _url._queryEnd = _url._pathAfterLastSlash;
              }
              state = State::Fragment;
              ++c;
              break;
            default:
              SyntaxViolation(c);
              if (base.IsValid() && base.ProtocolIsFile() && ShouldCopyFileURL(c))
              {
                CopyURLPartsUntil(base, URLPart::PathAfterLastSlash, c);
              }
              else
              {
                bool copiedHost = false;
                if (base.IsValid() && base.ProtocolIsFile())
                {
                  if (base.Host().empty())
                  {
                    CopyURLPartsUntil(base, URLPart::SchemeEnd, c);
                    AppendToASCIIBuffer(u8":///");
                  }
                  else
                  {
                    CopyURLPartsUntil(base, URLPart::PortEnd, c);
                    AppendToASCIIBuffer('/');
                    copiedHost = true;
                  }
                }
                else
                {
                  AppendToASCIIBuffer(u8"///");
                }
                if (!copiedHost)
                {
                  _url._userStart = static_cast<uint32>(CurrentPosition(c) - 1);
                  _url._userEnd = _url._userStart;
                  _url._passwordEnd = _url._userStart;
                  _url._hostEnd = _url._userStart;
                  _url._portLength = 0;
                }
                _url._pathAfterLastSlash = _url._hostEnd + 1;
              }
              if (IsWindowsDriveLetter(c))
              {
                AppendWindowsDriveLetter(c);
              }
              state = State::Path;
              break;
          }
          break;
        case State::FileSlash:
          if (*c == '/' || *c == '\\') KRYS_LIKELY
          {
            if (*c == '\\') KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            if (base.IsValid() && base.ProtocolIsFile())
            {
              CopyURLPartsUntil(base, URLPart::SchemeEnd, c);
              AppendToASCIIBuffer(u8":/");
            }
            AppendToASCIIBuffer('/');
            Advance(c);
            _url._userStart = static_cast<uint32>(CurrentPosition(c));
            _url._userEnd = _url._userStart;
            _url._passwordEnd = _url._userStart;
            _url._hostEnd = _url._userStart;
            _url._portLength = 0;
            authorityOrHostBegin = c;
            state = State::FileHost;
            break;
          }
          {
            bool copiedHost = false;
            if (base.IsValid() && base.ProtocolIsFile())
            {
              if (base.Host().empty())
              {
                CopyURLPartsUntil(base, URLPart::SchemeEnd, c);
                AppendToASCIIBuffer(u8":///");
              }
              else
              {
                CopyURLPartsUntil(base, URLPart::PortEnd, c);
                AppendToASCIIBuffer('/');
                copiedHost = true;
              }
            }
            else
            {
              SyntaxViolation(c);
              AppendToASCIIBuffer(u8"//");
            }
            if (!copiedHost)
            {
              _url._userStart = static_cast<uint32>(CurrentPosition(c) - 1);
              _url._userEnd = _url._userStart;
              _url._passwordEnd = _url._userStart;
              _url._hostEnd = _url._userStart;
              _url._portLength = 0;
            }
          }
          if (IsWindowsDriveLetter(c))
          {
            AppendWindowsDriveLetter(c);
            _url._pathAfterLastSlash = _url._hostEnd + 1;
          }
          else if (CopyBaseWindowsDriveLetter(base))
          {
            AppendToASCIIBuffer('/');
            _url._pathAfterLastSlash = _url._hostEnd + 4;
          }
          else
          {
            _url._pathAfterLastSlash = _url._hostEnd + 1;
          }
          state = State::Path;
          break;
        case State::FileHost:
          do
          {
            if (IsSlashQuestionOrHash(*c))
            {
              bool windowsQuirk = TakesTwoAdvancesUntilEnd(CodePointIterator(authorityOrHostBegin, c))
                                  && IsWindowsDriveLetter(authorityOrHostBegin);
              if (windowsQuirk)
              {
                SyntaxViolation(authorityOrHostBegin);
                AppendToASCIIBuffer('/');
                AppendWindowsDriveLetter(authorityOrHostBegin);
              }
              if (windowsQuirk || authorityOrHostBegin == c)
              {
                krys_debug_assert(windowsQuirk || ParsedDataView(CurrentPosition(c) - 1) == '/');
                if (*c == '?') KRYS_UNLIKELY
                {
                  SyntaxViolation(c);
                  AppendToASCIIBuffer(u8"/?");
                  ++c;
                  state = State::UTF8Query;
                  _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c) - 1);
                  _url._pathEnd = _url._pathAfterLastSlash;
                  break;
                }
                if (*c == '#') KRYS_UNLIKELY
                {
                  SyntaxViolation(c);
                  AppendToASCIIBuffer(u8"/#");
                  ++c;
                  _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c) - 1);
                  _url._pathEnd = _url._pathAfterLastSlash;
                  _url._queryEnd = _url._pathAfterLastSlash;
                  state = State::Fragment;
                  break;
                }
                state = authorityOrHostBegin == c ? State::FilePathStart : State::Path;
                break;
              }
              if (ParseHostAndPort(CodePointIterator(authorityOrHostBegin, c))
                  == HostParsingResult::InvalidHost)
              {
                Failure();
                return;
              }
              if (IsLocalhost(ParsedDataView(_url._passwordEnd, CurrentPosition(c) - _url._passwordEnd)))
                KRYS_UNLIKELY
              {
                SyntaxViolation(c);
                _asciiBuffer.resize(_url._passwordEnd);
                _url._hostEnd = static_cast<uint32>(CurrentPosition(c));
                _url._portLength = 0;
              }

              state = State::PathStart;
              break;
            }
            if (IsPercentOrNonASCII(*c))
            {
              _hostHasPercentOrNonASCII = true;
            }
            ++c;
          } while (!c.AtEnd());
          break;
        case State::FilePathStart:
          if (*c == '/' || *c != '\\')
          {
            if (_urlIsSpecial && *c == '\\') KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            AppendToASCIIBuffer('/');
            Advance(c);
            _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
            if (IsWindowsDriveLetter(c) && CurrentPosition(c) == _url._hostEnd + 1)
            {
              AppendWindowsDriveLetter(c);
            }
          }
          state = State::Path;
          break;
        case State::PathStart:
          if (*c != '/' && *c != '\\')
          {
            SyntaxViolation(c);
            AppendToASCIIBuffer('/');
          }
          _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
          state = State::Path;
          break;
        case State::Path:
          if (*c == '/' || (_urlIsSpecial && *c == '\\'))
          {
            if (_urlIsSpecial && *c == '\\') KRYS_UNLIKELY
            {
              SyntaxViolation(c);
            }
            AppendToASCIIBuffer('/');
            ++c;
            _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
            break;
          }
          if (CurrentPosition(c) && ParsedDataView(CurrentPosition(c) - 1) == '/') KRYS_UNLIKELY
          {
            if (IsDoubleDotPathSegment(c)) KRYS_UNLIKELY
            {
              SyntaxViolation(c);
              ConsumeDoubleDotPathSegment(c);
              PopPath();
              break;
            }
            if (IsSingleDotPathSegment(c)) KRYS_UNLIKELY
            {
              SyntaxViolation(c);
              ConsumeSingleDotPathSegment(c);
              break;
            }
          }
          if (*c == '?')
          {
            _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
            AppendToASCIIBuffer('?');
            ++c;
            state = State::UTF8Query;
            break;
          }
          if (*c == '#')
          {
            _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
            _url._queryEnd = _url._pathEnd;
            state = State::Fragment;
            break;
          }
          UTF8PercentEncode<IsInPathEncodeSet>(c);
          ++c;
          break;
        case State::OpaquePath:
          if (*c == '?')
          {
            _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
            AppendToASCIIBuffer('?');
            ++c;
            state = State::UTF8Query;
          }
          else if (*c == '#')
          {
            _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
            _url._queryEnd = _url._pathEnd;
            state = State::Fragment;
          }
          else if (*c == '/')
          {
            AppendToASCIIBuffer('/');
            ++c;
            _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
          }
          else if (*c == ' ')
          {
            auto nextC = c;
            Advance<ReportSyntaxViolation::No>(nextC);
            krys_debug_assert(!nextC.AtEnd());
            if (*nextC == '?' || *nextC == '#')
            {
              SyntaxViolation(c);
              PercentEncodeByte(' ');
            }
            else
            {
              AppendToASCIIBuffer(' ');
            }
            ++c;
          }
          else
          {
            UTF8PercentEncode<IsInC0ControlEncodeSet>(c);
            ++c;
          }
          break;
        case State::UTF8Query:
          if (*c == '#')
          {
            _url._queryEnd = static_cast<uint32>(CurrentPosition(c));
            state = State::Fragment;
            break;
          }
          UTF8QueryEncode(c);
          ++c;
          break;
        case State::Fragment:
          UTF8PercentEncode<IsInFragmentEncodeSet>(c);
          ++c;
          break;
      }
    }

    switch (state)
    {
      case State::SchemeStart:
        if (!CurrentPosition(c) && base.IsValid() && !base._hasOpaquePath)
        {
          _url = base;
          _url.RemoveFragmentIdentifier();
          return;
        }
        Failure();
        return;
      case State::Scheme:                     Failure(); return;
      case State::NoScheme:                   krys_unreachable();
      case State::SpecialRelativeOrAuthority: CopyURLPartsUntil(base, URLPart::QueryEnd, c); break;
      case State::PathOrAuthority:
        krys_debug_assert(_url._userStart);
        krys_debug_assert(_url._userStart == CurrentPosition(c));
        krys_debug_assert(ParsedDataView(CurrentPosition(c) - 1) == '/');
        _url._userStart--;
        _url._userEnd = _url._userStart;
        _url._passwordEnd = _url._userStart;
        _url._hostEnd = _url._userStart;
        _url._portLength = 0;
        _url._pathAfterLastSlash = _url._userStart + 1;
        _url._pathEnd = _url._pathAfterLastSlash;
        _url._queryEnd = _url._pathAfterLastSlash;
        break;
      case State::Relative: krys_unreachable();
      case State::RelativeSlash:
        CopyURLPartsUntil(base, URLPart::PortEnd, c);
        AppendToASCIIBuffer('/');
        _url._pathAfterLastSlash = _url._hostEnd + _url._portLength + 1;
        _url._pathEnd = _url._pathAfterLastSlash;
        _url._queryEnd = _url._pathAfterLastSlash;
        break;
      case State::SpecialAuthoritySlashes:       Failure(); return;
      case State::SpecialAuthorityIgnoreSlashes: Failure(); return;
      case State::AuthorityOrHost:
        _url._userEnd = static_cast<uint32>(CurrentPosition(authorityOrHostBegin));
        _url._passwordEnd = _url._userEnd;
        if (authorityOrHostBegin.AtEnd())
        {
          _url._userEnd = _url._userStart;
          _url._passwordEnd = _url._userStart;
          _url._hostEnd = _url._userStart;
          _url._portLength = 0;
          _url._pathEnd = _url._userStart;
        }
        else if (ParseHostAndPort(authorityOrHostBegin) == HostParsingResult::InvalidHost)
        {
          Failure();
          return;
        }
        else
        {
          if (_urlIsSpecial)
          {
            SyntaxViolation(c);
            AppendToASCIIBuffer('/');
            _url._pathEnd = _url._hostEnd + _url._portLength + 1;
          }
          else
          {
            _url._pathEnd = _url._hostEnd + _url._portLength;
          }
        }
        _url._pathAfterLastSlash = _url._pathEnd;
        _url._queryEnd = _url._pathEnd;
        break;
      case State::Host:
        if (ParseHostAndPort(authorityOrHostBegin) == HostParsingResult::InvalidHost)
        {
          Failure();
          return;
        }
        if (_urlIsSpecial)
        {
          SyntaxViolation(c);
          AppendToASCIIBuffer('/');
          _url._pathEnd = _url._hostEnd + _url._portLength + 1;
        }
        else
        {
          _url._pathEnd = _url._hostEnd + _url._portLength;
        }
        _url._pathAfterLastSlash = _url._pathEnd;
        _url._queryEnd = _url._pathEnd;
        break;
      case State::File:
        if (base.IsValid() && base.ProtocolIsFile())
        {
          CopyURLPartsUntil(base, URLPart::QueryEnd, c);
          break;
        }
        SyntaxViolation(c);
        AppendToASCIIBuffer(u8"///");
        _url._userStart = static_cast<uint32>(CurrentPosition(c) - 1);
        _url._userEnd = _url._userStart;
        _url._passwordEnd = _url._userStart;
        _url._hostEnd = _url._userStart;
        _url._portLength = 0;
        _url._pathAfterLastSlash = _url._userStart + 1;
        _url._pathEnd = _url._pathAfterLastSlash;
        _url._queryEnd = _url._pathAfterLastSlash;
        break;
      case State::FileSlash:
        SyntaxViolation(c);
        {
          bool copiedHost = false;
          if (base.IsValid() && base.ProtocolIsFile())
          {
            if (base.Host().empty())
            {
              CopyURLPartsUntil(base, URLPart::SchemeEnd, c);
              AppendToASCIIBuffer(u8":/");
            }
            else
            {
              CopyURLPartsUntil(base, URLPart::PortEnd, c);
              AppendToASCIIBuffer('/');
              copiedHost = true;
            }
          }
          if (!copiedHost)
          {
            _url._userStart = static_cast<uint32>(CurrentPosition(c) + 1);
            AppendToASCIIBuffer(u8"//");
            _url._userEnd = _url._userStart;
            _url._passwordEnd = _url._userStart;
            _url._hostEnd = _url._userStart;
            _url._portLength = 0;
          }
        }
        if (CopyBaseWindowsDriveLetter(base))
        {
          AppendToASCIIBuffer('/');
          _url._pathAfterLastSlash = _url._hostEnd + 4;
        }
        else
        {
          _url._pathAfterLastSlash = _url._hostEnd + 1;
        }
        _url._pathEnd = _url._pathAfterLastSlash;
        _url._queryEnd = _url._pathAfterLastSlash;
        break;
      case State::FileHost:
        if (TakesTwoAdvancesUntilEnd(CodePointIterator(authorityOrHostBegin, c))
            && IsWindowsDriveLetter(authorityOrHostBegin))
        {
          SyntaxViolation(authorityOrHostBegin);
          AppendToASCIIBuffer('/');
          AppendWindowsDriveLetter(authorityOrHostBegin);
          _url._pathAfterLastSlash = static_cast<uint32>(CurrentPosition(c));
          _url._pathEnd = _url._pathAfterLastSlash;
          _url._queryEnd = _url._pathAfterLastSlash;
          break;
        }

        if (authorityOrHostBegin == c)
        {
          SyntaxViolation(c);
          AppendToASCIIBuffer('/');
          _url._userStart = static_cast<uint32>(CurrentPosition(c) - 1);
          _url._userEnd = _url._userStart;
          _url._passwordEnd = _url._userStart;
          _url._hostEnd = _url._userStart;
          _url._portLength = 0;
          _url._pathAfterLastSlash = _url._userStart + 1;
          _url._pathEnd = _url._pathAfterLastSlash;
          _url._queryEnd = _url._pathAfterLastSlash;
          break;
        }

        if (ParseHostAndPort(CodePointIterator(authorityOrHostBegin, c)) == HostParsingResult::InvalidHost)
        {
          Failure();
          return;
        }

        SyntaxViolation(c);
        if (IsLocalhost(ParsedDataView(_url._passwordEnd, CurrentPosition(c) - _url._passwordEnd)))
        {
          _asciiBuffer.resize(_url._passwordEnd);
          _url._hostEnd = static_cast<uint32>(CurrentPosition(c));
          _url._portLength = 0;
        }
        AppendToASCIIBuffer('/');
        _url._pathAfterLastSlash = _url._hostEnd + _url._portLength + 1;
        _url._pathEnd = _url._pathAfterLastSlash;
        _url._queryEnd = _url._pathAfterLastSlash;
        break;
      case State::PathStart:     krys_unreachable();
      case State::FilePathStart: [[fallthrough]];
      case State::Path:
        _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
        _url._queryEnd = _url._pathEnd;
        break;
      case State::OpaquePath:
        _url._pathEnd = static_cast<uint32>(CurrentPosition(c));
        _url._queryEnd = _url._pathEnd;
        break;
      case State::UTF8Query: _url._queryEnd = static_cast<uint32>(CurrentPosition(c)); break;
      case State::Fragment:  break;
    }

    if (!_didSeeSyntaxViolation) KRYS_LIKELY
    {
      _url._string = utf8_string {_input};
      krys_debug_assert(_asciiBuffer.empty());
    }
    else
    {
      _url._string = krys::move(_asciiBuffer);
    }
    _url._isValid = true;
    _url._isNull = false;
  }

  void URLParser::ParseAuthority(CodePointIterator iterator) noexcept
  {
    if (iterator.AtEnd()) KRYS_UNLIKELY
    {
      SyntaxViolation(iterator);
      _url._userEnd = static_cast<uint32>(CurrentPosition(iterator));
      _url._passwordEnd = _url._userEnd;
      return;
    }
    for (; !iterator.AtEnd(); Advance(iterator))
    {
      if (*iterator == ':')
      {
        _url._userEnd = static_cast<uint32>(CurrentPosition(iterator));
        auto iteratorAtColon = iterator;
        ++iterator;
        bool tabOrNewlineAfterColon = false;
        while (!iterator.AtEnd() && IsTabOrNewline(*iterator)) KRYS_UNLIKELY
        {
          tabOrNewlineAfterColon = true;
          ++iterator;
        }
        if (iterator.AtEnd()) KRYS_UNLIKELY
        {
          SyntaxViolation(iteratorAtColon);
          _url._passwordEnd = _url._userEnd;
          if (_url._userEnd > _url._userStart)
          {
            AppendToASCIIBuffer('@');
          }
          return;
        }
        if (tabOrNewlineAfterColon)
        {
          SyntaxViolation(iteratorAtColon);
        }
        AppendToASCIIBuffer(':');
        break;
      }
      UTF8PercentEncode<url::IsInUserInfoEncodeSet>(iterator);
    }
    for (; !iterator.AtEnd(); Advance(iterator))
    {
      UTF8PercentEncode<url::IsInUserInfoEncodeSet>(iterator);
    }
    _url._passwordEnd = static_cast<uint32>(CurrentPosition(iterator));
    if (!_url._userEnd)
    {
      _url._userEnd = _url._passwordEnd;
    }
    AppendToASCIIBuffer('@');
  }

  template <typename TUnsigned>
  void URLParser::AppendNumberToASCIIBuffer(TUnsigned number) noexcept
  {
    constexpr size_t bufferSize = sizeof(TUnsigned) * 3 + 1;
    Array<char8, bufferSize> buffer;
    size_t index = bufferSize;
    do
    {
      buffer[--index] = static_cast<char8>((number % 10) + '0');
      number /= 10;
    } while (number);
    AppendToASCIIBuffer(utf8_stringview {buffer.data() + index, bufferSize - index});
  }

  void URLParser::SerialiseIPv4(IPv4Address address) noexcept
  {
    AppendNumberToASCIIBuffer<uint8>(static_cast<uint8>(address >> 24));
    AppendToASCIIBuffer('.');
    AppendNumberToASCIIBuffer<uint8>(static_cast<uint8>(address >> 16));
    AppendToASCIIBuffer('.');
    AppendNumberToASCIIBuffer<uint8>(static_cast<uint8>(address >> 8));
    AppendToASCIIBuffer('.');
    AppendNumberToASCIIBuffer<uint8>(static_cast<uint8>(address));
  }

  void URLParser::SerialiseIPv6Piece(uint16 piece) noexcept
  {
    bool printed = false;
    if (auto nibble0 = piece >> 12)
    {
      AppendToASCIIBuffer(
        static_cast<char32>(text::LowerNibbleToLowercaseASCIIHexDigit(static_cast<uint8>(nibble0))));
      printed = true;
    }
    auto nibble1 = piece >> 8 & 0xF;
    if (printed || nibble1)
    {
      AppendToASCIIBuffer(
        static_cast<char32>(text::LowerNibbleToLowercaseASCIIHexDigit(static_cast<uint8>(nibble1))));
      printed = true;
    }
    auto nibble2 = piece >> 4 & 0xF;
    if (printed || nibble2)
    {
      AppendToASCIIBuffer(
        static_cast<char32>(text::LowerNibbleToLowercaseASCIIHexDigit(static_cast<uint8>(nibble2))));
    }
    AppendToASCIIBuffer(
      static_cast<char32>(text::LowerNibbleToLowercaseASCIIHexDigit(static_cast<uint8>(piece & 0xF))));
  }

  void URLParser::SerialiseIPv6(IPv6Address address) noexcept
  {
    AppendToASCIIBuffer('[');
    auto compressPointer = FindLongestZeroSequence(address);
    for (size_t piece = 0; piece < 8; piece++)
    {
      if (compressPointer && compressPointer.value() == piece)
      {
        krys_debug_assert(!address[piece]);
        if (piece)
        {
          AppendToASCIIBuffer(':');
        }
        else
        {
          AppendToASCIIBuffer(u8"::");
        }
        while (piece < 8 && !address[piece])
        {
          piece++;
        }
        if (piece == 8)
        {
          break;
        }
      }
      SerialiseIPv6Piece(address[piece]);
      if (piece < 7)
      {
        AppendToASCIIBuffer(':');
      }
    }
    AppendToASCIIBuffer(']');
  }

  enum class URLParser::IPv4PieceParsingError
  {
    Failure,
    Overflow,
  };

  Expected<uint32, URLParser::IPv4PieceParsingError>
    URLParser::ParseIPv4Piece(CodePointIterator &iterator, bool &didSeeSyntaxViolation) noexcept
  {
    enum class State : uint8
    {
      UnknownBase,
      Decimal,
      OctalOrHex,
      Octal,
      Hex,
    };
    State state = State::UnknownBase;
    uint64 value = 0;
    constexpr uint64 max = std::numeric_limits<uint32>::max();
    if (!iterator.AtEnd() && *iterator == '.')
    {
      return Unexpected(IPv4PieceParsingError::Failure);
    }
    while (!iterator.AtEnd())
    {
      if (IsTabOrNewline(*iterator))
      {
        didSeeSyntaxViolation = true;
        ++iterator;
        continue;
      }
      if (*iterator == '.')
      {
        krys_debug_assert(value <= max);
        return static_cast<uint32>(value);
      }
      switch (state)
      {
        case State::UnknownBase:
          if (*iterator == '0') KRYS_UNLIKELY
          {
            ++iterator;
            state = State::OctalOrHex;
            break;
          }
          state = State::Decimal;
          break;
        case State::OctalOrHex:
          didSeeSyntaxViolation = true;
          if (*iterator == 'x' || *iterator == 'X')
          {
            ++iterator;
            state = State::Hex;
            break;
          }
          state = State::Octal;
          break;
        case State::Decimal:
          if (!text::IsASCIIDigit(*iterator))
          {
            return Unexpected(IPv4PieceParsingError::Failure);
          }
          value = value * 10 + (*iterator - '0');
          if (value > max) KRYS_UNLIKELY
          {
            return Unexpected(IPv4PieceParsingError::Overflow);
          }
          ++iterator;
          break;
        case State::Octal:
          krys_debug_assert(didSeeSyntaxViolation);
          if (*iterator < '0' || *iterator > '7')
          {
            return Unexpected(IPv4PieceParsingError::Failure);
          }
          value = value * 8 + (*iterator - '0');
          if (value > max) KRYS_UNLIKELY
          {
            return Unexpected(IPv4PieceParsingError::Overflow);
          }
          ++iterator;
          break;
        case State::Hex:
          krys_debug_assert(didSeeSyntaxViolation);
          if (!text::IsASCIIHexDigit(*iterator))
          {
            return Unexpected(IPv4PieceParsingError::Failure);
          }
          value = value * 16 + text::ToASCIIHexValue(*iterator);
          if (value > max) KRYS_UNLIKELY
          {
            return Unexpected(IPv4PieceParsingError::Overflow);
          }
          ++iterator;
          break;
      }
    }
    krys_debug_assert(value <= max);
    return static_cast<uint32>(value);
  }

  enum class URLParser::IPv4ParsingError
  {
    Failure,
    NotIPv4,
  };

  Expected<URLParser::IPv4Address, URLParser::IPv4ParsingError>
    URLParser::ParseIPv4Host(const CodePointIterator &iteratorForSyntaxViolationPosition,
                             CodePointIterator iterator) noexcept
  {
    SmallList<Expected<uint32, IPv4PieceParsingError>, 4> items;
    bool didSeeSyntaxViolation = false;
    if (!iterator.AtEnd() && *iterator == '.')
    {
      return Unexpected(IPv4ParsingError::NotIPv4);
    }
    while (!iterator.AtEnd())
    {
      if (IsTabOrNewline(*iterator))
      {
        didSeeSyntaxViolation = true;
        ++iterator;
        continue;
      }
      if (items.size() >= 4)
      {
        return Unexpected(IPv4ParsingError::NotIPv4);
      }
      items.push_back(ParseIPv4Piece(iterator, didSeeSyntaxViolation));
      if (!iterator.AtEnd() && *iterator == '.')
      {
        ++iterator;
        if (iterator.AtEnd())
        {
          didSeeSyntaxViolation = true;
        }
        else if (*iterator == '.')
        {
          return Unexpected(IPv4ParsingError::NotIPv4);
        }
      }
    }
    if (!iterator.AtEnd() || !items.size() || items.size() > 4)
    {
      return Unexpected(IPv4ParsingError::NotIPv4);
    }
    for (const auto &item : items)
    {
      if (!item.has_value() && item.error() == IPv4PieceParsingError::Failure)
      {
        return Unexpected(IPv4ParsingError::NotIPv4);
      }
    }
    for (const auto &item : items)
    {
      if (!item.has_value() && item.error() == IPv4PieceParsingError::Overflow)
      {
        return Unexpected(IPv4ParsingError::Failure);
      }
    }
    if (items.size() > 1)
    {
      for (size_t i = 0; i < items.size() - 1; i++)
      {
        if (items[i].value() > 255)
        {
          return Unexpected(IPv4ParsingError::Failure);
        }
      }
    }
    if (items[items.size() - 1].value() >= Pow256(5 - items.size()))
    {
      return Unexpected(IPv4ParsingError::Failure);
    }

    if (didSeeSyntaxViolation)
    {
      SyntaxViolation(iteratorForSyntaxViolationPosition);
    }
    for (const auto &item : items)
    {
      if (item.value() > 255)
      {
        SyntaxViolation(iteratorForSyntaxViolationPosition);
      }
    }

    if (items.size() != 4) KRYS_UNLIKELY
    {
      SyntaxViolation(iteratorForSyntaxViolationPosition);
    }

    IPv4Address ipv4 = items.back().value();
    items.pop_back();
    for (size_t counter = 0; counter < items.size(); ++counter)
    {
      ipv4 += static_cast<IPv4Address>(items[counter].value() * Pow256(3 - counter));
    }
    return ipv4;
  }

  Maybe<uint32> URLParser::ParseIPv4PieceInsideIPv6(CodePointIterator &iterator) noexcept
  {
    if (iterator.AtEnd())
    {
      return null;
    }
    uint32 piece = 0;
    bool leadingZeros = false;
    while (!iterator.AtEnd())
    {
      if (!text::IsASCIIDigit(*iterator))
      {
        return null;
      }
      if (!piece && *iterator == '0')
      {
        if (leadingZeros)
        {
          return null;
        }
        leadingZeros = true;
      }
      piece = piece * 10 + *iterator - '0';
      if (piece > 255)
      {
        return null;
      }
      Advance<ReportSyntaxViolation::No>(iterator);
      if (iterator.AtEnd())
      {
        break;
      }
      if (*iterator == '.')
      {
        break;
      }
    }
    if (piece && leadingZeros)
    {
      return null;
    }
    return piece;
  }

  Maybe<URLParser::IPv4Address> URLParser::ParseIPv4AddressInsideIPv6(CodePointIterator iterator) noexcept
  {
    IPv4Address address = 0;
    for (size_t i = 0; i < 4; ++i)
    {
      if (Maybe<uint32> piece = ParseIPv4PieceInsideIPv6(iterator))
      {
        address = (address << 8) + piece.value();
      }
      else
      {
        return null;
      }
      if (i < 3)
      {
        if (iterator.AtEnd())
        {
          return null;
        }
        if (*iterator != '.')
        {
          return null;
        }
        Advance<ReportSyntaxViolation::No>(iterator);
      }
      else if (!iterator.AtEnd())
      {
        return null;
      }
    }
    krys_debug_assert(iterator.AtEnd());
    return address;
  }

  Maybe<URLParser::IPv6Address> URLParser::ParseIPv6Host(CodePointIterator c) noexcept
  {
    krys_debug_assert(*c == '[');
    const auto hostBegin = c;
    Advance(c, hostBegin);
    if (c.AtEnd())
    {
      return null;
    }

    IPv6Address address {{0, 0, 0, 0, 0, 0, 0, 0}};
    size_t piecePointer = 0;
    Maybe<size_t> compressPointer;
    bool previousValueWasZero = false;
    bool immediatelyAfterCompress = false;

    if (*c == ':')
    {
      Advance(c, hostBegin);
      if (c.AtEnd())
      {
        return null;
      }
      if (*c != ':')
      {
        return null;
      }
      Advance(c, hostBegin);
      ++piecePointer;
      compressPointer = piecePointer;
      immediatelyAfterCompress = true;
    }

    while (!c.AtEnd())
    {
      if (piecePointer == 8)
      {
        return null;
      }
      if (*c == ':')
      {
        if (compressPointer)
        {
          return null;
        }
        Advance(c, hostBegin);
        ++piecePointer;
        compressPointer = piecePointer;
        immediatelyAfterCompress = true;
        if (previousValueWasZero)
        {
          SyntaxViolation(hostBegin);
        }
        continue;
      }
      if (piecePointer == 6 || (compressPointer && piecePointer < 6))
      {
        if (Maybe<IPv4Address> ipv4Address = ParseIPv4AddressInsideIPv6(c))
        {
          if (compressPointer && piecePointer == 5)
          {
            return null;
          }
          SyntaxViolation(hostBegin);
          address[piecePointer++] = static_cast<uint16>(ipv4Address.value() >> 16);
          address[piecePointer++] = static_cast<uint16>(ipv4Address.value() & 0xFFFF);
          c = {};
          break;
        }
      }
      uint16 value = 0;
      size_t length = 0;
      bool leadingZeros = false;
      for (; length < 4; length++)
      {
        if (c.AtEnd())
        {
          break;
        }
        if (!text::IsASCIIHexDigit(*c))
        {
          break;
        }
        if (text::IsASCIIUpper(*c))
        {
          SyntaxViolation(hostBegin);
        }
        if (*c == '0' && !length)
        {
          leadingZeros = true;
        }
        value = static_cast<uint16>(value * 0x10 + text::ToASCIIHexValue(*c));
        Advance(c, hostBegin);
      }

      previousValueWasZero = !value;
      if ((value && leadingZeros) || (previousValueWasZero && (length > 1 || immediatelyAfterCompress)))
        KRYS_UNLIKELY
      {
        SyntaxViolation(hostBegin);
      }

      address[piecePointer++] = value;
      if (c.AtEnd())
      {
        break;
      }
      if (piecePointer == 8 || *c != ':')
      {
        return null;
      }
      Advance(c, hostBegin);
      if (c.AtEnd())
      {
        SyntaxViolation(hostBegin);
      }

      immediatelyAfterCompress = false;
    }

    if (!c.AtEnd())
    {
      return null;
    }

    if (compressPointer)
    {
      size_t swaps = piecePointer - compressPointer.value();
      piecePointer = 7;
      while (swaps)
      {
        std::swap(address[piecePointer--], address[compressPointer.value() + swaps-- - 1]);
      }
    }
    else if (piecePointer != 8)
    {
      return null;
    }

    Maybe<size_t> possibleCompressPointer = FindLongestZeroSequence(address);
    if (possibleCompressPointer)
    {
      possibleCompressPointer.value()++;
    }
    if (compressPointer != possibleCompressPointer) KRYS_UNLIKELY
    {
      SyntaxViolation(hostBegin);
    }

    return address;
  }

  utf8_string URLParser::PercentDecode(utf8_stringview input,
                                       const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept
  {
    utf8_string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
      char8 byte = input[i];
      if (byte != '%')
      {
        output.push_back(byte);
      }
      else if (input.size() > 2 && i < input.size() - 2)
      {
        if (text::IsASCIIHexDigit(input[i + 1]) && text::IsASCIIHexDigit(input[i + 2]))
        {
          SyntaxViolation(iteratorForSyntaxViolationPosition);
          output.push_back(static_cast<char8>(text::ToASCIIHexValue(input[i + 1], input[i + 2])));
          i += 2;
        }
        else
        {
          output.push_back(byte);
        }
      }
      else
      {
        output.push_back(byte);
      }
    }
    return output;
  }

  utf8_string URLParser::PercentDecode(utf8_stringview input) noexcept
  {
    utf8_string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
      char8 byte = input[i];
      if (byte != '%')
      {
        output.push_back(byte);
      }
      else if (input.size() > 2 && i < input.size() - 2)
      {
        if (text::IsASCIIHexDigit(input[i + 1]) && text::IsASCIIHexDigit(input[i + 2]))
        {
          output.push_back(static_cast<char8>(text::ToASCIIHexValue(input[i + 1], input[i + 2])));
          i += 2;
        }
        else
        {
          output.push_back(byte);
        }
      }
      else
      {
        output.push_back(byte);
      }
    }
    return output;
  }

  bool URLParser::NeedsNonSpecialDotSlash() const noexcept
  {
    auto pathStart = _url._hostEnd + _url._portLength;
    return !_urlIsSpecial && pathStart == _url._schemeEnd + 1U && pathStart + 1 < _url._string.size()
           && _url._string[pathStart] == '/' && _url._string[pathStart + 1] == '/';
  }

  void URLParser::AddNonSpecialDotSlash() noexcept
  {
    auto oldPathStart = _url._hostEnd + _url._portLength;
    _url._string.insert(oldPathStart + 1, u8"./");
    _url._pathAfterLastSlash += 2;
    _url._pathEnd += 2;
    _url._queryEnd += 2;
  }

  Maybe<utf8_string>
    URLParser::DomainToASCII(utf8_stringview domain,
                             const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept
  {
    utf8_string ascii;
    if (infra::StringAlgorithms::IsASCIIString(domain) && !SubdomainStartsWithXNDashDash(domain))
    {
      ascii.reserve(domain.size());
      for (char8 c : domain)
      {
        if (text::IsASCIIUpper(c)) KRYS_UNLIKELY
        {
          SyntaxViolation(iteratorForSyntaxViolationPosition);
        }
        ascii.push_back(text::ToASCIILower(c));
      }
      return ascii;
    }

    Array<char, HostnameBufferLength> hostnameBuffer;
    UErrorCode error = U_ZERO_ERROR;
    UIDNAInfo processingDetails = UIDNA_INFO_INITIALIZER;
    int32_t numCharactersConverted = uidna_nameToASCII_UTF8(
      &InternationalDomainNameTranscoder(), reinterpret_cast<const char *>(domain.data()),
      static_cast<int32_t>(domain.size()), hostnameBuffer.data(), static_cast<int32_t>(HostnameBufferLength),
      &processingDetails, &error);

    if (U_SUCCESS(error) && !(processingDetails.errors & ~AllowedNameToASCIIErrors) && numCharactersConverted)
    {
      for (int32_t i = 0; i < numCharactersConverted; ++i)
      {
        krys_debug_assert(text::IsASCII(hostnameBuffer[i]));
        krys_debug_assert(!text::IsASCIIUpper(hostnameBuffer[i]));
      }
      ascii.assign(reinterpret_cast<const char8 *>(hostnameBuffer.data()),
                   static_cast<size_t>(numCharactersConverted));
      if (domain != utf8_stringview {ascii})
      {
        SyntaxViolation(iteratorForSyntaxViolationPosition);
      }
      return ascii;
    }
    return null;
  }

  bool URLParser::HasForbiddenHostCodePoint(utf8_stringview asciiDomain) noexcept
  {
    for (auto character : asciiDomain)
    {
      if (IsForbiddenDomainCodePoint(character))
      {
        return true;
      }
    }
    return false;
  }

  bool URLParser::ParsePort(CodePointIterator &iterator) noexcept
  {
    if (_urlIsFile) KRYS_UNLIKELY
    {
      return false;
    }

    krys_debug_assert(*iterator == ':');
    auto colonIterator = iterator;
    Advance(iterator, colonIterator);
    uint32 port = 0;
    if (iterator.AtEnd()) KRYS_UNLIKELY
    {
      size_t portLength = CurrentPosition(colonIterator) - _url._hostEnd;
      krys_assert(portLength <= URL::MaxPortLength);
      _url._portLength = static_cast<uint32>(portLength);
      SyntaxViolation(colonIterator);
      return true;
    }
    size_t digitCount = 0;
    bool leadingZeros = false;
    for (; !iterator.AtEnd(); ++iterator)
    {
      if (IsTabOrNewline(*iterator)) KRYS_UNLIKELY
      {
        SyntaxViolation(colonIterator);
        continue;
      }
      if (text::IsASCIIDigit(*iterator))
      {
        if (*iterator == '0' && !digitCount)
        {
          leadingZeros = true;
        }
        ++digitCount;
        port = port * 10 + *iterator - '0';
        if (port > std::numeric_limits<uint16>::max())
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    if (port && leadingZeros)
    {
      SyntaxViolation(colonIterator);
    }

    if (!port && digitCount > 1)
    {
      SyntaxViolation(colonIterator);
    }

    krys_debug_assert(port == static_cast<uint16>(port));
    if (DefaultPortForProtocol(ParsedDataView(0, _url._schemeEnd)) == static_cast<uint16>(port)) KRYS_UNLIKELY
    {
      SyntaxViolation(colonIterator);
    }
    else
    {
      AppendToASCIIBuffer(':');
      AppendNumberToASCIIBuffer<uint16>(static_cast<uint16>(port));
    }

    size_t portLength = CurrentPosition(iterator) - _url._hostEnd;
    krys_assert(portLength <= URL::MaxPortLength);
    _url._portLength = static_cast<uint32>(portLength);
    return true;
  }

  bool URLParser::SubdomainStartsWithXNDashDash(CodePointIterator iterator) noexcept
  {
    enum class State : uint8
    {
      NotAtSubdomainBeginOrInXNDashDash,
      AtSubdomainBegin,
      AfterX,
      AfterN,
      AfterFirstDash,
    } state {State::AtSubdomainBegin};

    for (; !iterator.AtEnd(); Advance<ReportSyntaxViolation::No>(iterator))
    {
      char32 c = *iterator;

      // These characters indicate the end of the host.
      if (c == ':' || c == '/' || c == '?' || c == '#')
      {
        return false;
      }

      switch (state)
      {
        case State::NotAtSubdomainBeginOrInXNDashDash: break;
        case State::AtSubdomainBegin:
          if (c == 'x' || c == 'X')
          {
            state = State::AfterX;
            continue;
          }
          break;
        case State::AfterX:
          if (c == 'n' || c == 'N')
          {
            state = State::AfterN;
            continue;
          }
          break;
        case State::AfterN:
          if (c == '-')
          {
            state = State::AfterFirstDash;
            continue;
          }
          break;
        case State::AfterFirstDash:
          if (c == '-')
          {
            return true;
          }
          break;
      }

      if (c == '.')
      {
        state = State::AtSubdomainBegin;
      }
      else
      {
        state = State::NotAtSubdomainBeginOrInXNDashDash;
      }
    }
    return false;
  }

  bool URLParser::SubdomainStartsWithXNDashDash(utf8_stringview host) noexcept
  {
    return SubdomainStartsWithXNDashDash(CodePointIterator {host});
  }

  URLParser::HostParsingResult URLParser::ParseHostAndPort(CodePointIterator iterator) noexcept
  {
    if (iterator.AtEnd())
    {
      return HostParsingResult::InvalidHost;
    }
    if (*iterator == ':')
    {
      return HostParsingResult::InvalidHost;
    }
    if (*iterator == '[')
    {
      auto ipv6End = iterator;
      while (!ipv6End.AtEnd() && *ipv6End != ']')
      {
        ++ipv6End;
      }
      if (ipv6End.AtEnd())
      {
        return HostParsingResult::InvalidHost;
      }
      if (auto address = ParseIPv6Host(CodePointIterator(iterator, ipv6End)))
      {
        SerialiseIPv6(address.value());
        if (!ipv6End.AtEnd())
        {
          Advance(ipv6End);
          _url._hostEnd = static_cast<uint32>(CurrentPosition(ipv6End));
          if (!ipv6End.AtEnd() && *ipv6End == ':')
          {
            return ParsePort(ipv6End) ? HostParsingResult::IPv6WithPort : HostParsingResult::InvalidHost;
          }
          _url._portLength = 0;
          return ipv6End.AtEnd() ? HostParsingResult::IPv6WithoutPort : HostParsingResult::InvalidHost;
        }
        _url._hostEnd = static_cast<uint32>(CurrentPosition(ipv6End));
        return HostParsingResult::IPv6WithoutPort;
      }
      return HostParsingResult::InvalidHost;
    }

    if (!_urlIsSpecial)
    {
      for (; !iterator.AtEnd(); ++iterator)
      {
        if (IsTabOrNewline(*iterator)) KRYS_UNLIKELY
        {
          SyntaxViolation(iterator);
          continue;
        }
        if (*iterator == ':')
        {
          break;
        }
        if (IsForbiddenHostCodePoint(*iterator) && *iterator != '%') KRYS_UNLIKELY
        {
          return HostParsingResult::InvalidHost;
        }
        UTF8PercentEncode<IsInC0ControlEncodeSet>(iterator);
      }
      _url._hostEnd = static_cast<uint32>(CurrentPosition(iterator));
      if (iterator.AtEnd())
      {
        _url._portLength = 0;
        return HostParsingResult::NonSpecialHostWithoutPort;
      }
      return ParsePort(iterator) ? HostParsingResult::NonSpecialHostWithPort : HostParsingResult::InvalidHost;
    }

    if (!_hostHasPercentOrNonASCII && !SubdomainStartsWithXNDashDash(iterator)) KRYS_LIKELY
    {
      auto hostIterator = iterator;
      for (; !iterator.AtEnd(); ++iterator)
      {
        if (IsTabOrNewline(*iterator))
        {
          continue;
        }
        if (*iterator == ':')
        {
          break;
        }
        if (IsForbiddenDomainCodePoint(*iterator))
        {
          return HostParsingResult::InvalidHost;
        }
      }
      auto address = ParseIPv4Host(hostIterator, CodePointIterator(hostIterator, iterator));
      if (address)
      {
        SerialiseIPv4(address.value());
        _url._hostEnd = static_cast<uint32>(CurrentPosition(iterator));
        if (iterator.AtEnd())
        {
          _url._portLength = 0;
          return HostParsingResult::IPv4WithoutPort;
        }
        return ParsePort(iterator) ? HostParsingResult::IPv4WithPort : HostParsingResult::InvalidHost;
      }
      if (address.error() == IPv4ParsingError::Failure)
      {
        return HostParsingResult::InvalidHost;
      }
      for (; hostIterator != iterator; ++hostIterator)
      {
        if (IsTabOrNewline(*hostIterator)) KRYS_UNLIKELY
        {
          SyntaxViolation(hostIterator);
          continue;
        }
        if (text::IsASCIIUpper(*hostIterator)) KRYS_UNLIKELY
        {
          SyntaxViolation(hostIterator);
        }
        AppendToASCIIBuffer(text::ToASCIILower(*hostIterator));
      }
      _url._hostEnd = static_cast<uint32>(CurrentPosition(iterator));
      auto hostStart = _url.HostStart();
      if (DNSNameEndsInNumber(ParsedDataView(hostStart, _url._hostEnd - hostStart))) KRYS_UNLIKELY
      {
        return HostParsingResult::InvalidHost;
      }
      if (!hostIterator.AtEnd())
      {
        return ParsePort(hostIterator) ? HostParsingResult::DNSNameWithPort : HostParsingResult::InvalidHost;
      }
      _url._portLength = 0;
      return HostParsingResult::DNSNameWithoutPort;
    }

    const auto hostBegin = iterator;

    utf8_string utf8Encoded;
    for (; !iterator.AtEnd(); ++iterator)
    {
      if (IsTabOrNewline(*iterator)) KRYS_UNLIKELY
      {
        SyntaxViolation(hostBegin);
        continue;
      }
      if (*iterator == ':')
      {
        break;
      }
      if (!text::IsASCII(*iterator)) KRYS_UNLIKELY
      {
        SyntaxViolation(hostBegin);
      }

      Array<uint8, 4> buffer;
      uint8 length = EncodeUTF8(*iterator, buffer);
      if (!length)
      {
        return HostParsingResult::InvalidHost;
      }
      utf8Encoded.append(reinterpret_cast<const char8 *>(buffer.data()), length);
    }
    utf8_string percentDecoded = PercentDecode(utf8Encoded, hostBegin);
    if (!text::IsValidUTF8(percentDecoded))
    {
      return HostParsingResult::InvalidHost;
    }
    // Upstream compares the UTF-8-decoded domain with the bytes read as Latin-1; they differ exactly when a
    // byte is non-ASCII.
    if (!infra::StringAlgorithms::IsASCIIString(percentDecoded))
    {
      SyntaxViolation(hostBegin);
    }
    auto asciiDomain = DomainToASCII(percentDecoded, hostBegin);
    if (!asciiDomain || HasForbiddenHostCodePoint(asciiDomain.value()))
    {
      return HostParsingResult::InvalidHost;
    }
    utf8_string &asciiDomainValue = asciiDomain.value();

    auto address = ParseIPv4Host(hostBegin, CodePointIterator {utf8_stringview {asciiDomainValue}});
    if (address)
    {
      SerialiseIPv4(address.value());
      _url._hostEnd = static_cast<uint32>(CurrentPosition(iterator));
      if (iterator.AtEnd())
      {
        _url._portLength = 0;
        return HostParsingResult::IPv4WithoutPort;
      }
      return ParsePort(iterator) ? HostParsingResult::IPv4WithPort : HostParsingResult::InvalidHost;
    }
    if (address.error() == IPv4ParsingError::Failure)
    {
      return HostParsingResult::InvalidHost;
    }

    AppendToASCIIBuffer(asciiDomainValue);
    _url._hostEnd = static_cast<uint32>(CurrentPosition(iterator));
    auto hostStart = _url.HostStart();
    if (DNSNameEndsInNumber(ParsedDataView(hostStart, _url._hostEnd - hostStart))) KRYS_UNLIKELY
    {
      return HostParsingResult::InvalidHost;
    }
    if (!iterator.AtEnd())
    {
      return ParsePort(iterator) ? HostParsingResult::DNSNameWithPort : HostParsingResult::InvalidHost;
    }

    _url._portLength = 0;
    return HostParsingResult::DNSNameWithoutPort;
  }

  Maybe<utf8_string> URLParser::FormURLDecode(utf8_stringview input) noexcept
  {
    if (!text::IsValidUTF8(input))
    {
      return null;
    }
    auto percentDecoded = PercentDecode(input);
    return ReplaceInvalidUTF8(percentDecoded);
  }

  // https://url.spec.whatwg.org/#concept-urlencoded-parser
  URLParser::URLEncodedForm URLParser::ParseURLEncodedForm(utf8_stringview input) noexcept
  {
    URLEncodedForm output;
    size_t start = 0;
    while (start <= input.size())
    {
      size_t end = input.find(u8'&', start);
      if (end == utf8_stringview::npos)
      {
        end = input.size();
      }
      utf8_stringview bytes = input.substr(start, end - start);
      if (!bytes.empty())
      {
        if (auto nameAndValue = ParseQueryNameAndValue(bytes))
        {
          output.push_back(krys::move(*nameAndValue));
        }
      }
      start = end + 1;
    }
    return output;
  }

  Maybe<KeyValuePair> URLParser::ParseQueryNameAndValue(utf8_stringview bytes) noexcept
  {
    auto equalIndex = bytes.find(u8'=');
    if (equalIndex == utf8_stringview::npos)
    {
      auto name = FormURLDecode(ReplaceAll(bytes, u8'+', u8' '));
      if (name)
      {
        return KeyValuePair {krys::move(*name), {}};
      }
    }
    else
    {
      auto name = FormURLDecode(ReplaceAll(bytes.substr(0, equalIndex), u8'+', u8' '));
      auto value = FormURLDecode(ReplaceAll(bytes.substr(equalIndex + 1), u8'+', u8' '));
      if (name && value)
      {
        return KeyValuePair {krys::move(*name), krys::move(*value)};
      }
    }
    return null;
  }

  utf8_string URLParser::Serialise(const URLEncodedForm &tuples) noexcept
  {
    if (tuples.empty())
    {
      return {};
    }

    utf8_string output;
    for (auto &tuple : tuples)
    {
      if (!output.empty())
      {
        output.push_back(u8'&');
      }
      SerialiseURLEncodedForm(tuple.Key, output);
      output.push_back(u8'=');
      SerialiseURLEncodedForm(tuple.Value, output);
    }
    return output;
  }

  const UIDNA &URLParser::InternationalDomainNameTranscoder() noexcept
  {
    static UIDNA *encoder = []
    {
      UErrorCode error = U_ZERO_ERROR;
      auto *transcoder =
        uidna_openUTS46(UIDNA_CHECK_BIDI | UIDNA_CHECK_CONTEXTJ | UIDNA_NONTRANSITIONAL_TO_UNICODE
                          | UIDNA_NONTRANSITIONAL_TO_ASCII,
                        &error);
      krys_assert_msg(U_SUCCESS(error) && transcoder, "Couldn't create the IDNA transcoder.");
      return transcoder;
    }();
    return *encoder;
  }

  bool URLParser::AllValuesEqual(const URL &a, const URL &b) noexcept
  {
    return a._string == b._string && a._isValid == b._isValid && a._hasOpaquePath == b._hasOpaquePath
           && a._protocolIsInHTTPFamily == b._protocolIsInHTTPFamily && a._schemeEnd == b._schemeEnd
           && a._userStart == b._userStart && a._userEnd == b._userEnd && a._passwordEnd == b._passwordEnd
           && a._hostEnd == b._hostEnd && a._portLength == b._portLength
           && a._pathAfterLastSlash == b._pathAfterLastSlash && a._pathEnd == b._pathEnd
           && a._queryEnd == b._queryEnd;
  }

  bool URLParser::InternalValuesConsistent(const URL &url) noexcept
  {
    return url._schemeEnd <= url._userStart && url._userStart <= url._userEnd
           && url._userEnd <= url._passwordEnd && url._passwordEnd <= url._hostEnd
           && url._hostEnd + url._portLength <= url._pathAfterLastSlash
           && url._pathAfterLastSlash <= url._pathEnd && url._pathEnd <= url._queryEnd
           && url._queryEnd <= url._string.size();
  }
}
