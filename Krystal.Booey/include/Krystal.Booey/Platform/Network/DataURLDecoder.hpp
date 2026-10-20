#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::data_url_decoder
{
  struct Result
  {
    utf8_string MimeType;
    utf8_string Charset;
    utf8_string ContentType;
    List<uint8> Data;
  };

  enum class ShouldValidatePadding : bool
  {
    No,
    Yes
  };

  /// @brief Decodes a `data:` URL's media type and data; null if it's malformed.
  /// FIXME: Decoding on a work queue (the asynchronous `decode`); loads decode synchronously.
  KRYS_NODISCARD Maybe<Result> Decode(const url::URL &url,
                                      ShouldValidatePadding = ShouldValidatePadding::No) noexcept;
}
