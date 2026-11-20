#include "Krystal.Net/MimeTypeRegistry.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::net::tests
{
  TEST_CASE("MimeTypeRegistry: types by extension", "[MimeTypeRegistry]")
  {
    // Extensions are without their dot, and ASCII case-insensitive.
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"PNG") == u8"image/png");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"jpeg") == u8"image/jpeg");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"webp") == u8"image/webp");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"avif") == u8"image/avif");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"woff2") == u8"font/woff2");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"TTF") == u8"font/ttf");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"js") == u8"text/javascript");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"json") == u8"application/json");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"zip") == u8"application/zip");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"docx")
          == u8"application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"unknown").empty());
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8"").empty());
    CHECK(MimeTypeRegistry::MimeTypeForExtension(u8".png").empty());
  }
}
