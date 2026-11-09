#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("MIMETypeRegistry: types by extension", "[Platform][MIMETypeRegistry]")
  {
    // Extensions are without their dot, and ASCII case-insensitive.
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"PNG") == u8"image/png");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"jpeg") == u8"image/jpeg");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"webp") == u8"image/webp");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"avif") == u8"image/avif");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"woff2") == u8"font/woff2");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"TTF") == u8"font/ttf");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"js") == u8"text/javascript");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"json") == u8"application/json");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"zip") == u8"application/zip");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"docx")
          == u8"application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"unknown").empty());
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8"").empty());
    CHECK(MIMETypeRegistry::MIMETypeForExtension(u8".png").empty());
  }
}
