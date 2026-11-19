#include "Krystal.Net/DataUrlDecoder.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::net::tests
{
  namespace
  {
    KRYS_NODISCARD Maybe<data_url_decoder::Result> Decode(utf8_stringview url) noexcept
    {
      return data_url_decoder::Decode(net::Url(url));
    }

    KRYS_NODISCARD utf8_string AsText(const List<uint8> &data) noexcept
    {
      return utf8_string(data.begin(), data.end());
    }
  }

  TEST_CASE("DataURLDecoder: decodes percent-encoded and base64 data", "[DataUrlDecoder]")
  {
    SECTION("Text")
    {
      auto result = Decode(u8"data:,Hello%2C%20World!");
      REQUIRE(result);
      CHECK(result->MimeType == u8"text/plain");
      CHECK(result->Charset == u8"US-ASCII");
      CHECK(AsText(result->Data) == u8"Hello, World!");
    }

    SECTION("A media type and charset")
    {
      auto result = Decode(u8"data:Text/HTML;charset=utf-8,%3Cp%3E");
      REQUIRE(result);
      CHECK(result->MimeType == u8"text/html");
      CHECK(result->Charset == u8"utf-8");
      CHECK(result->ContentType == u8"text/html;charset=utf-8");
      CHECK(AsText(result->Data) == u8"<p>");
    }

    SECTION("Base64")
    {
      auto result = Decode(u8"data:text/plain;base64,SGVsbG8=");
      REQUIRE(result);
      CHECK(AsText(result->Data) == u8"Hello");

      // Binary data: PNG's signature.
      auto image = Decode(u8"data:image/png;base64,iVBORw0KGgo=");
      REQUIRE(image);
      CHECK(image->MimeType == u8"image/png");
      CHECK(image->Data == List<uint8> {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'});
    }

    SECTION("The fragment isn't data")
    {
      auto result = Decode(u8"data:,abc#def");
      REQUIRE(result);
      CHECK(AsText(result->Data) == u8"abc");
    }
  }

  TEST_CASE("DataURLDecoder: rejects malformed URLs", "[DataUrlDecoder]")
  {
    CHECK_FALSE(Decode(u8"data:text/plain"));
    CHECK_FALSE(Decode(u8"data:;base64,!!!!"));
  }
}
