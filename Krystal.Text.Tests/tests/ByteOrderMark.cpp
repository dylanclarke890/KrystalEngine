#include "Krystal.Text/ByteOrderMark.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Text;

  TEST_CASE("BOM::DetectByteOrderMark", "[Text][BOM]")
  {
    {
      Array<byte, 3u> data = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::utf8);
    }
    {
      Array<byte, 2u> data = {byte {0xFE}, byte {0xFF}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::utf16be);
    }
    {
      Array<byte, 2u> data = {byte {0xFF}, byte {0xFE}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::utf16le);
    }
    {
      Array<byte, 4u> data = {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::utf32be);
    }
    {
      Array<byte, 4u> data = {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::utf32le);
    }
    {
      Array<byte, 2u> data = {byte {0x00}, byte {0x00}};
      EncodingId bom = BOM::DetectByteOrderMark(data);
      REQUIRE(bom == EncodingId::unknown);
    }
  }

  TEST_CASE("BOM::GetBytes", "[Text][BOM]")
  {
    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::utf8);
      REQUIRE(bytes.size() == 3u);
      REQUIRE(bytes[0] == byte {0xEF});
      REQUIRE(bytes[1] == byte {0xBB});
      REQUIRE(bytes[2] == byte {0xBF});
    }

    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::utf16be);
      REQUIRE(bytes.size() == 2u);
      REQUIRE(bytes[0] == byte {0xFE});
      REQUIRE(bytes[1] == byte {0xFF});
    }

    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::utf16le);
      REQUIRE(bytes.size() == 2u);
      REQUIRE(bytes[0] == byte {0xFF});
      REQUIRE(bytes[1] == byte {0xFE});
    }

    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::utf32be);
      REQUIRE(bytes.size() == 4u);
      REQUIRE(bytes[0] == byte {0x00});
      REQUIRE(bytes[1] == byte {0x00});
      REQUIRE(bytes[2] == byte {0xFF});
      REQUIRE(bytes[3] == byte {0xFE});
    }

    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::utf32le);
      REQUIRE(bytes.size() == 4u);
      REQUIRE(bytes[0] == byte {0xFE});
      REQUIRE(bytes[1] == byte {0xFF});
      REQUIRE(bytes[2] == byte {0x00});
      REQUIRE(bytes[3] == byte {0x00});
    }

    {
      Span<const byte> bytes = BOM::GetBytes(EncodingId::unknown);
      REQUIRE(bytes.size() == 0u);
    }
  }
}