#include "Krystal.Booey/Graphics/WOFFFileFormat.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey/Graphics/FontCustomPlatformData.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<SharedBuffer> ReadFont(const char *name) noexcept
    {
      // Ahem.ttf is with the fonts tests register; the files made from it are kept apart.
      auto directory = std::string_view(name).ends_with(".ttf") ? "fonts" : "webfonts";
      std::ifstream stream(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / directory / name,
                           std::ios::binary);
      REQUIRE(stream);
      List<uint8> bytes {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
      return SharedBuffer::Create(krys::move(bytes));
    }
  }

  TEST_CASE("ConvertWOFFToSfntIfNecessary: WOFF and WOFF 2 files become fonts", "[Graphics][WOFFFileFormat]")
  {
    auto name = GENERATE("Ahem.woff", "Ahem.woff2");
    INFO(name);

    RefPtr<SharedBuffer> buffer = ReadFont(name);
    auto *original = buffer.get();
    CHECK(ConvertWOFFToSfntIfNecessary(buffer));
    REQUIRE(buffer);
    CHECK(buffer.get() != original);

    // The converted font is Ahem's TrueType font, with its tables.
    auto data = FontCustomPlatformData::Create(*buffer, {});
    REQUIRE(data);
    CHECK(data->Face->FamilyName() == u8"Ahem");
    CHECK(buffer->Size() > ReadFont("Ahem.ttf")->Size() / 2);
  }

  TEST_CASE("ConvertWOFFToSfntIfNecessary: other files are left alone, and broken ones dropped",
            "[Graphics][WOFFFileFormat]")
  {
    SECTION("a TrueType font isn't converted")
    {
      RefPtr<SharedBuffer> buffer = ReadFont("Ahem.ttf");
      auto *original = buffer.get();
      CHECK_FALSE(ConvertWOFFToSfntIfNecessary(buffer));
      CHECK(buffer.get() == original);
    }

    SECTION("a WOFF file cut short becomes null")
    {
      auto name = GENERATE("Ahem.woff", "Ahem.woff2");
      INFO(name);
      auto whole = ReadFont(name);
      auto span = whole->GetSpan();
      List<uint8> half(span.begin(), span.begin() + static_cast<ptrdiff_t>(span.size() / 2));
      RefPtr<SharedBuffer> buffer = SharedBuffer::Create(krys::move(half));
      CHECK(ConvertWOFFToSfntIfNecessary(buffer));
      CHECK_FALSE(buffer);
    }

    SECTION("a short file isn't converted")
    {
      RefPtr<SharedBuffer> buffer = SharedBuffer::Create(List<uint8> {'w', 'O'});
      CHECK_FALSE(ConvertWOFFToSfntIfNecessary(buffer));
      CHECK(buffer);
    }
  }
}
