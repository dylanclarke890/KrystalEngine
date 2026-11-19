#include "Krystal.Text/IDNA.hpp"
#include "Krystal.Core/Assert.hpp"
#include <unicode/uidna.h>

namespace krys::text
{
  namespace
  {
    // The URL Standard sets CheckHyphens and VerifyDnsLength to false, so these errors don't fail a domain.
    constexpr uint32 AllowedErrors = UIDNA_ERROR_EMPTY_LABEL | UIDNA_ERROR_LABEL_TOO_LONG
                                     | UIDNA_ERROR_DOMAIN_NAME_TOO_LONG | UIDNA_ERROR_LEADING_HYPHEN
                                     | UIDNA_ERROR_TRAILING_HYPHEN | UIDNA_ERROR_HYPHEN_3_4;

    KRYS_NODISCARD const UIDNA &Transcoder() noexcept
    {
      static UIDNA *transcoder = []
      {
        UErrorCode error = U_ZERO_ERROR;
        auto *result = uidna_openUTS46(UIDNA_CHECK_BIDI | UIDNA_CHECK_CONTEXTJ
                                         | UIDNA_NONTRANSITIONAL_TO_UNICODE | UIDNA_NONTRANSITIONAL_TO_ASCII,
                                       &error);
        krys_assert_msg(U_SUCCESS(error) && result, "Couldn't create the IDNA transcoder.");
        return result;
      }();
      return *transcoder;
    }

    // Converts into @p buffer, returning the length of the result, which may be longer than the buffer.
    KRYS_NODISCARD int32 NameToASCII(utf8_stringview domain, utf8_string &buffer, UErrorCode &error,
                                     UIDNAInfo &info) noexcept
    {
      error = U_ZERO_ERROR;
      info = UIDNA_INFO_INITIALIZER;
      return uidna_nameToASCII_UTF8(
        &Transcoder(), reinterpret_cast<const char *>(domain.data()), static_cast<int32>(domain.size()),
        reinterpret_cast<char *>(buffer.data()), static_cast<int32>(buffer.size()), &info, &error);
    }
  }

  Maybe<utf8_string> DomainToASCII(utf8_stringview domain) noexcept
  {
    // Punycode is usually a little longer than the UTF-8 it encodes; retry with the exact size if not.
    utf8_string ascii(domain.size() + 64, u8'\0');
    UErrorCode error;
    UIDNAInfo info;
    int32 length = NameToASCII(domain, ascii, error, info);
    if (error == U_BUFFER_OVERFLOW_ERROR)
    {
      ascii.resize(static_cast<size_t>(length));
      length = NameToASCII(domain, ascii, error, info);
    }

    if (U_FAILURE(error) || (info.errors & ~AllowedErrors) || !length)
    {
      return null;
    }
    ascii.resize(static_cast<size_t>(length));
    return ascii;
  }
}
