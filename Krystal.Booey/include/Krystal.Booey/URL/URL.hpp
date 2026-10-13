#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/Tags.hpp"

namespace krys::boo::url
{
  class URLParser;

  /// @brief A decoded name/value pair from an `application/x-www-form-urlencoded` string.
  struct KeyValuePair
  {
    utf8_string Key;
    utf8_string Value;

    KRYS_NODISCARD bool operator==(const KeyValuePair &) const noexcept = default;
  };

  /// @brief A URL, parsed and serialised as specified by https://url.spec.whatwg.org.
  /// @note The serialisation is stored once, with the end offsets of each component, so the component getters
  /// return views into it. String views returned by the getters are null (`data() == nullptr`) when the
  /// component is absent, and non-null but empty when it's present and empty (e.g. `Query()` for
  /// `http://a/?`).
  class URL
  {
    friend class URLParser;

  private:
    utf8_string _string;

    uint32 _isValid : 1 {false};
    uint32 _protocolIsInHTTPFamily : 1 {false};
    uint32 _hasOpaquePath : 1 {false};
    uint32 _isNull : 1 {true};
    uint32 _isHashTableDeletedValue : 1 {false};

    // This is out of order to align the bits better. The port is after the host.
    uint32 _portLength : 3 {0};
    constexpr static uint32 MaxPortLength = (1 << 3) - 1;

    constexpr static uint32 MaxSchemeLength = (1 << 24) - 1;
    uint32 _schemeEnd : 24 {0};
    uint32 _userStart {0};
    uint32 _userEnd {0};
    uint32 _passwordEnd {0};
    uint32 _hostEnd {0};
    uint32 _pathAfterLastSlash {0};
    uint32 _pathEnd {0};
    uint32 _queryEnd {0};

  public:
    /// @brief A null URL.
    URL() noexcept = default;

    explicit URL(HashTableDeletedValueTag) noexcept;

    /// @brief Resolves `relative` against `base`. `base` can be null or invalid, in which case `relative` is
    /// parsed as an absolute URL. A null `relative` (`data() == nullptr`) gives `base` without its fragment.
    URL(const URL &base, utf8_stringview relative) noexcept;

    /// @brief Parses `absoluteURL` as an absolute URL. To parse a relative URL, use the constructor that
    /// takes a base URL.
    explicit URL(utf8_stringview absoluteURL) noexcept;

    URL(const URL &) = default;
    URL &operator=(const URL &) = default;
    URL(URL &&other) noexcept;
    URL &operator=(URL &&other) noexcept;

    KRYS_NODISCARD static URL FileURLWithFileSystemPath(utf8_stringview path) noexcept;

    struct StripResult
    {
      utf8_string String;
      bool Stripped {false};
    };

    /// @brief The URL without credentials or fragment, for use as a `Referer`.
    KRYS_NODISCARD StripResult StrippedForUseAsReferrer() const noexcept;
    /// @brief Like `StrippedForUseAsReferrer`, but with the scheme's default port made explicit.
    KRYS_NODISCARD StripResult StrippedForUseAsReferrerWithExplicitPort() const noexcept;
    /// @brief Like `StrippedForUseAsReferrer`, but without the query either.
    KRYS_NODISCARD utf8_string StrippedForUseAsReport() const noexcept;

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return _isHashTableDeletedValue;
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _isNull;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _string.empty();
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _isValid;
    }

    /// @brief The serialisation if the URL is valid, otherwise the input it was parsed from.
    KRYS_NODISCARD const utf8_string &String() const noexcept KRYS_LIFETIME_BOUND
    {
      return _string;
    }

    KRYS_NODISCARD utf8_string StringCenterEllipsisedToLength(uint32 length = 1'024) const noexcept;

    // Unlike User() and Password(), EncodedUser() and EncodedPassword() don't decode escape sequences.
    // This is necessary for accurate round-tripping, because encoding doesn't encode '%' characters.

    KRYS_NODISCARD utf8_stringview Protocol() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview EncodedUser() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview EncodedPassword() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview Host() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD Maybe<uint16> Port() const noexcept;
    KRYS_NODISCARD utf8_stringview Path() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview LastPathComponent() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview Query() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview FragmentIdentifier() const noexcept KRYS_LIFETIME_BOUND;

    KRYS_NODISCARD utf8_stringview QueryWithLeadingQuestionMark() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview
      FragmentIdentifierWithLeadingNumberSign() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview ViewWithoutQueryOrFragmentIdentifier() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_stringview ViewWithoutFragmentIdentifier() const noexcept KRYS_LIFETIME_BOUND;
    KRYS_NODISCARD utf8_string StringWithoutFragmentIdentifier() const noexcept;

    KRYS_NODISCARD utf8_string ProtocolHostAndPort() const noexcept;
    KRYS_NODISCARD utf8_string HostAndPort() const noexcept;
    KRYS_NODISCARD utf8_string User() const noexcept;
    KRYS_NODISCARD utf8_string Password() const noexcept;
    KRYS_NODISCARD utf8_string FileSystemPath() const noexcept;

    /// @brief The URL up to and including the last slash of its path.
    KRYS_NODISCARD URL TruncatedForUseAsBase() const noexcept;

    KRYS_NODISCARD bool HasQuery() const noexcept
    {
      return _queryEnd > _pathEnd;
    }

    KRYS_NODISCARD bool HasFragmentIdentifier() const noexcept
    {
      return _isValid && _string.size() > _queryEnd;
    }

    KRYS_NODISCARD bool HasPath() const noexcept
    {
      return _pathEnd > PathStart();
    }

    KRYS_NODISCARD bool HasCredentials() const noexcept
    {
      return _passwordEnd > _userStart;
    }

    /// @brief Whether the URL's scheme is `protocol`, which must be lowercase ASCII.
    KRYS_NODISCARD bool ProtocolIs(utf8_stringview protocol) const noexcept;

    KRYS_NODISCARD bool ProtocolIsAbout() const noexcept
    {
      return ProtocolIs(u8"about");
    }

    KRYS_NODISCARD bool ProtocolIsBlob() const noexcept
    {
      return ProtocolIs(u8"blob");
    }

    KRYS_NODISCARD bool ProtocolIsData() const noexcept
    {
      return ProtocolIs(u8"data");
    }

    KRYS_NODISCARD bool ProtocolIsFile() const noexcept
    {
      return ProtocolIs(u8"file");
    }

    KRYS_NODISCARD bool ProtocolIsJavaScript() const noexcept;

    KRYS_NODISCARD bool ProtocolIsInFTPFamily() const noexcept
    {
      return ProtocolIs(u8"ftp") || ProtocolIs(u8"ftps");
    }

    KRYS_NODISCARD bool ProtocolIsInHTTPFamily() const noexcept
    {
      return _protocolIsInHTTPFamily;
    }

    KRYS_NODISCARD bool ProtocolIsSecure() const noexcept;

    KRYS_NODISCARD bool HasOpaquePath() const noexcept
    {
      return _hasOpaquePath;
    }

    KRYS_NODISCARD bool IsAboutBlank() const noexcept;
    KRYS_NODISCARD bool IsAboutSrcDoc() const noexcept;

    KRYS_NODISCARD bool IsMatchingDomain(utf8_stringview domain) const noexcept;

    /// @return False if `newProtocol` isn't a valid scheme; true otherwise, even if the scheme wasn't
    /// changed.
    bool SetProtocol(utf8_stringview newProtocol) noexcept;
    /// @return Whether the URL is valid after the change.
    bool SetHost(utf8_stringview newHost) noexcept;

    void SetPort(Maybe<uint16> port) noexcept;

    /// @param hostAndPort E.g. `foo.com` or `foo.com:8000`.
    void SetHostAndPort(utf8_stringview hostAndPort) noexcept;
    void RemoveHostAndPort() noexcept;

    void SetUser(utf8_stringview newUser) noexcept;
    void SetPassword(utf8_stringview newPassword) noexcept;
    void RemoveCredentials() noexcept;

    /// @brief If you pass an empty path for HTTP or HTTPS URLs, the resulting path will be "/".
    void SetPath(utf8_stringview path) noexcept;

    /// @brief The query may begin with a question mark, or, if not, one will be added for you. Setting the
    /// query to the empty string will leave a "?" in the URL (with nothing after it). To clear the query,
    /// pass a null view (`data() == nullptr`).
    void SetQuery(utf8_stringview newQuery) noexcept;

    void SetFragmentIdentifier(utf8_stringview identifier) noexcept;
    void RemoveFragmentIdentifier() noexcept;
    /// @brief Removes the fragment directive (`:~:` and what follows) from the fragment and returns it.
    /// @see https://wicg.github.io/scroll-to-text-fragment/#the-fragment-directive
    utf8_string ConsumeFragmentDirective() noexcept;
    void RemoveQueryAndFragmentIdentifier() noexcept;

    KRYS_NODISCARD static bool HostIsIPAddress(utf8_stringview host) noexcept;
    KRYS_NODISCARD static bool IsIPv6Address(utf8_stringview address) noexcept;

    KRYS_NODISCARD uint32 PathStart() const noexcept;

    KRYS_NODISCARD uint32 PathEnd() const noexcept
    {
      return _pathEnd;
    }

    KRYS_NODISCARD uint32 PathAfterLastSlash() const noexcept
    {
      return _pathAfterLastSlash;
    }

    KRYS_NODISCARD bool HasSpecialScheme() const noexcept;
    KRYS_NODISCARD bool HasLocalScheme() const noexcept;
    KRYS_NODISCARD bool HasFetchScheme() const noexcept;

    KRYS_NODISCARD friend bool ProtocolHostAndPortAreEqual(const URL &a, const URL &b) noexcept;

  private:
    void Invalidate() noexcept;
    KRYS_NODISCARD uint32 HostStart() const noexcept;
    KRYS_NODISCARD uint32 CredentialsEnd() const noexcept;
    void Remove(uint32 start, uint32 length) noexcept;
    void Parse(utf8_stringview string) noexcept;
    void ParseAllowingC0AtEnd(utf8_stringview string) noexcept;
  };

  KRYS_NODISCARD inline bool operator==(const URL &a, const URL &b) noexcept
  {
    return a.String() == b.String();
  }

  KRYS_NODISCARD inline bool operator==(const URL &a, utf8_stringview b) noexcept
  {
    return a.String() == b;
  }

  KRYS_NODISCARD bool EqualIgnoringFragmentIdentifier(const URL &a, const URL &b) noexcept;
  KRYS_NODISCARD bool ProtocolHostAndPortAreEqual(const URL &a, const URL &b) noexcept;
  KRYS_NODISCARD List<KeyValuePair> DifferingQueryParameters(const URL &a, const URL &b) noexcept;
  KRYS_NODISCARD List<KeyValuePair> QueryParameters(const URL &url) noexcept;
  KRYS_NODISCARD bool IsEqualIgnoringQueryAndFragments(const URL &a, const URL &b) noexcept;

  /// @return The parameters that were removed (including duplicates), in the order that they appear in the
  /// URL.
  List<utf8_string> RemoveQueryParameters(URL &url, const HashSet<utf8_string> &keysToRemove) noexcept;
  /// @return The parameters that were removed (including duplicates), in the order that they appear in the
  /// URL.
  List<utf8_string> RemoveQueryParameters(
    URL &url,
    KRYS_NOESCAPE const Func<bool(const utf8_string &, const utf8_string &)> &shouldRemove) noexcept;

  KRYS_NODISCARD const URL &AboutBlankURL() noexcept;
  KRYS_NODISCARD const URL &AboutSrcDocURL() noexcept;

  // Functions to do URL operations on strings.
  // These are operations that aren't faster on a parsed URL.
  // These are also different from the URL functions in that they don't require the string to be a valid and
  // parsable URL. This is especially important because valid javascript URLs are not necessarily considered
  // valid by URL.

  /// @param protocol Lowercase ASCII.
  KRYS_NODISCARD bool ProtocolIs(utf8_stringview url, utf8_stringview protocol) noexcept;
  KRYS_NODISCARD bool ProtocolIsJavaScript(utf8_stringview url) noexcept;
  KRYS_NODISCARD bool ProtocolIsInHTTPFamily(utf8_stringview url) noexcept;

  KRYS_NODISCARD Maybe<uint16> DefaultPortForProtocol(utf8_stringview protocol) noexcept;
  KRYS_NODISCARD bool IsDefaultPortForProtocol(uint16 port, utf8_stringview protocol) noexcept;
  /// @brief Disallows ports that should never be used for web resources.
  /// @see https://fetch.spec.whatwg.org/#port-blocking
  KRYS_NODISCARD bool PortAllowed(const URL &url) noexcept;

  /// @param dataURL Must start with `data:`.
  KRYS_NODISCARD utf8_string MimeTypeFromDataURL(utf8_stringview dataURL) noexcept;

  // FIXME: This needs a new, more specific name. The general thing named here can't be implemented correctly,
  // since different parts of a URL need different escaping.
  KRYS_NODISCARD utf8_string EncodeWithURLEscapeSequences(utf8_stringview input) noexcept;
  KRYS_NODISCARD utf8_string PercentEncodeFragmentDirectiveSpecialCharacters(utf8_stringview input) noexcept;
}
