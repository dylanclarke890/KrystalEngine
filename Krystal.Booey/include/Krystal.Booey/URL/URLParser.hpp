#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Expected.hpp"

// ICU's IDNA transcoder (`unicode/uidna.h`); only the sources include ICU.
struct UIDNA;

namespace krys::boo::url
{
  /// @brief The URL parser, https://url.spec.whatwg.org/#concept-basic-url-parser, plus the
  /// `application/x-www-form-urlencoded` parser and serialiser.
  class URLParser
  {
    friend class URL;
    using CodePointIterator = text::CodePointIterator;

  public:
    /// @brief The IDNA errors `uidna_nameToASCII` may report without the host being rejected.
    static const uint32 AllowedNameToASCIIErrors;

    // Needs to be big enough to hold an IDN-encoded name.
    // For host names bigger than this, we won't do IDN encoding, which is almost certainly OK.
    constexpr static size_t HostnameBufferLength = 2'048;

    using URLEncodedForm = List<KeyValuePair>;

  private:
    URL _url;
    utf8_string _asciiBuffer;
    bool _urlIsSpecial {false};
    bool _urlIsFile {false};
    bool _hostHasPercentOrNonASCII {false};
    bool _didSeeSyntaxViolation {false};
    utf8_stringview _input;
    const char8 *_inputBegin {nullptr};

  public:
    KRYS_NODISCARD static bool AllValuesEqual(const URL &a, const URL &b) noexcept;
    KRYS_NODISCARD static bool InternalValuesConsistent(const URL &url) noexcept;

    KRYS_NODISCARD static URLEncodedForm ParseURLEncodedForm(utf8_stringview input) noexcept;
    KRYS_NODISCARD static Maybe<KeyValuePair> ParseQueryNameAndValue(utf8_stringview bytes) noexcept;
    KRYS_NODISCARD static utf8_string Serialise(const URLEncodedForm &tuples) noexcept;

    KRYS_NODISCARD static bool IsSpecialScheme(utf8_stringview scheme) noexcept;
    KRYS_NODISCARD static Maybe<utf8_string> MaybeCanonicaliseScheme(utf8_stringview scheme) noexcept;

    KRYS_NODISCARD static const UIDNA &InternationalDomainNameTranscoder() noexcept;
    KRYS_NODISCARD static bool IsInUserInfoEncodeSet(char32 c) noexcept;
    KRYS_NODISCARD static bool IsSpecialCharacterForFragmentDirective(char32 c) noexcept;

    KRYS_NODISCARD static Maybe<uint16> DefaultPortForProtocol(utf8_stringview scheme) noexcept;
    KRYS_NODISCARD static Maybe<utf8_string> FormURLDecode(utf8_stringview input) noexcept;

  private:
    /// @param input Null (`data() == nullptr`) gives `base` without its fragment.
    URLParser(utf8_stringview input, const URL &base = {}, bool allowC0AtEnd = false) noexcept;

    KRYS_NODISCARD URL Result() noexcept
    {
      return krys::move(_url);
    }

    enum class HostParsingResult : uint8
    {
      InvalidHost,
      IPv6WithPort,
      IPv6WithoutPort,
      IPv4WithPort,
      IPv4WithoutPort,
      DNSNameWithPort,
      DNSNameWithoutPort,
      NonSpecialHostWithoutPort,
      NonSpecialHostWithPort
    };

    enum class ReportSyntaxViolation : bool
    {
      No,
      Yes
    };

    enum class URLPart;
    enum class IPv4ParsingError;
    enum class IPv4PieceParsingError;

    using IPv4Address = uint32;
    using IPv6Address = Array<uint16, 8>;

    void Parse(utf8_stringview input, const URL &base, bool allowC0AtEnd) noexcept;
    void ParseAuthority(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD HostParsingResult ParseHostAndPort(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool ParsePort(CodePointIterator &iterator) noexcept;

    void Failure() noexcept;

    template <ReportSyntaxViolation reportSyntaxViolation = ReportSyntaxViolation::Yes>
    void Advance(CodePointIterator &iterator) noexcept
    {
      Advance<reportSyntaxViolation>(iterator, iterator);
    }

    template <ReportSyntaxViolation reportSyntaxViolation = ReportSyntaxViolation::Yes>
    void Advance(CodePointIterator &iterator,
                 const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept;

    KRYS_NODISCARD bool TakesTwoAdvancesUntilEnd(CodePointIterator iterator) noexcept;
    void SyntaxViolation(const CodePointIterator &iterator) noexcept;
    KRYS_NODISCARD bool IsWindowsDriveLetter(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool IsSingleDotPathSegment(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool IsDoubleDotPathSegment(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool ShouldCopyFileURL(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool CheckLocalhostCodePoint(CodePointIterator &iterator, char32 codePoint) noexcept;
    KRYS_NODISCARD bool IsAtLocalhost(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool IsLocalhost(utf8_stringview view) noexcept;
    void ConsumeSingleDotPathSegment(CodePointIterator &iterator) noexcept;
    void ConsumeDoubleDotPathSegment(CodePointIterator &iterator) noexcept;
    void AppendWindowsDriveLetter(CodePointIterator &iterator) noexcept;
    KRYS_NODISCARD size_t CurrentPosition(const CodePointIterator &iterator) const noexcept;

    template <typename TUnsigned>
    void AppendNumberToASCIIBuffer(TUnsigned number) noexcept;

    template <bool (*IsInCodeSet)(char32)>
    void UTF8PercentEncode(const CodePointIterator &iterator) noexcept;
    void UTF8QueryEncode(const CodePointIterator &iterator) noexcept;

    KRYS_NODISCARD Maybe<utf8_string>
      DomainToASCII(utf8_stringview domain,
                    const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept;
    KRYS_NODISCARD utf8_string PercentDecode(
      utf8_stringview input, const CodePointIterator &iteratorForSyntaxViolationPosition) noexcept;
    KRYS_NODISCARD static utf8_string PercentDecode(utf8_stringview input) noexcept;
    KRYS_NODISCARD bool HasForbiddenHostCodePoint(utf8_stringview asciiDomain) noexcept;
    void PercentEncodeByte(uint8 byte) noexcept;
    void AppendToASCIIBuffer(char32 codePoint) noexcept;
    void AppendToASCIIBuffer(utf8_stringview characters) noexcept;
    void CopyASCIIStringUntil(const utf8_string &string, size_t length) noexcept;
    KRYS_NODISCARD bool CopyBaseWindowsDriveLetter(const URL &base) noexcept;
    KRYS_NODISCARD utf8_stringview ParsedDataView(size_t start,
                                                  size_t length) const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD char8 ParsedDataView(size_t position) const noexcept;
    KRYS_NODISCARD bool SubdomainStartsWithXNDashDash(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD bool SubdomainStartsWithXNDashDash(utf8_stringview host) noexcept;

    KRYS_NODISCARD bool NeedsNonSpecialDotSlash() const noexcept;
    void AddNonSpecialDotSlash() noexcept;

    void SerialiseIPv4(IPv4Address address) noexcept;
    KRYS_NODISCARD Expected<IPv4Address, IPv4ParsingError>
      ParseIPv4Host(const CodePointIterator &iteratorForSyntaxViolationPosition,
                    CodePointIterator iterator) noexcept;
    KRYS_NODISCARD Expected<uint32, IPv4PieceParsingError>
      ParseIPv4Piece(CodePointIterator &iterator, bool &didSeeSyntaxViolation) noexcept;
    KRYS_NODISCARD Maybe<IPv6Address> ParseIPv6Host(CodePointIterator iterator) noexcept;
    KRYS_NODISCARD Maybe<uint32> ParseIPv4PieceInsideIPv6(CodePointIterator &iterator) noexcept;
    KRYS_NODISCARD Maybe<IPv4Address> ParseIPv4AddressInsideIPv6(CodePointIterator iterator) noexcept;
    void SerialiseIPv6Piece(uint16 piece) noexcept;
    void SerialiseIPv6(IPv6Address address) noexcept;

    void CopyURLPartsUntil(const URL &base, URLPart part, const CodePointIterator &iterator) noexcept;
    KRYS_NODISCARD bool IsForbiddenHostCodePoint(char32 c) const noexcept;
    KRYS_NODISCARD bool IsForbiddenDomainCodePoint(char32 c) const noexcept;
    KRYS_NODISCARD static size_t URLLengthUntilPart(const URL &url, URLPart part) noexcept;
    void PopPath() noexcept;
    KRYS_NODISCARD bool ShouldPopPath(uint32 newPathAfterLastSlash) noexcept;
  };

  /// @see https://url.spec.whatwg.org/#forbidden-host-code-point
  KRYS_NODISCARD bool IsForbiddenHostCodePoint(char32 c) noexcept;
}
