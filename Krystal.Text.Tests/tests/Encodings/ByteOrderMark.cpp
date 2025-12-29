#include "Krystal.Text/Encodings/ByteOrderMark.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("ByteOrderMarkScanner(Detect)", "[Text][Encodings][ByteOrderMark]")
  {
    {
      Array<byte, 3u> data = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::UTF8);
    }
    {
      Array<byte, 2u> data = {byte {0xFE}, byte {0xFF}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::UTF16BE);
    }
    {
      Array<byte, 2u> data = {byte {0xFF}, byte {0xFE}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::UTF16LE);
    }
    {
      Array<byte, 4u> data = {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::UTF32BE);
    }
    {
      Array<byte, 4u> data = {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::UTF32LE);
    }
    {
      Array<byte, 2u> data = {byte {0x00}, byte {0x00}};
      ByteOrderMark bom = ByteOrderMarkScanner::Detect(data);
      REQUIRE(bom == ByteOrderMark::None);
    }
  }

  TEST_CASE("ByteOrderMarkScanner(GetEncodingName)", "[Text][Encodings][ByteOrderMark]")
  {
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::UTF8) == u8"UTF-8");
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::UTF16BE) == u8"UTF-16BE");
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::UTF16LE) == u8"UTF-16LE");
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::UTF32BE) == u8"UTF-32BE");
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::UTF32LE) == u8"UTF-32LE");
    REQUIRE(ByteOrderMarkScanner::GetEncodingName(ByteOrderMark::None) == u8"");
  }

  TEST_CASE("ByteOrderMarkScanner(GetBytes)", "[Text][Encodings][ByteOrderMark]")
  {
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::UTF8);
      REQUIRE(bytes.size() == 3u);
      REQUIRE(bytes[0] == byte {0xEF});
      REQUIRE(bytes[1] == byte {0xBB});
      REQUIRE(bytes[2] == byte {0xBF});
    }
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::UTF16BE);
      REQUIRE(bytes.size() == 2u);
      REQUIRE(bytes[0] == byte {0xFE});
      REQUIRE(bytes[1] == byte {0xFF});
    }
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::UTF16LE);
      REQUIRE(bytes.size() == 2u);
      REQUIRE(bytes[0] == byte {0xFF});
      REQUIRE(bytes[1] == byte {0xFE});
    }
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::UTF32BE);
      REQUIRE(bytes.size() == 4u);
      REQUIRE(bytes[0] == byte {0x00});
      REQUIRE(bytes[1] == byte {0x00});
      REQUIRE(bytes[2] == byte {0xFF});
      REQUIRE(bytes[3] == byte {0xFE});
    }
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::UTF32LE);
      REQUIRE(bytes.size() == 4u);
      REQUIRE(bytes[0] == byte {0xFE});
      REQUIRE(bytes[1] == byte {0xFF});
      REQUIRE(bytes[2] == byte {0x00});
      REQUIRE(bytes[3] == byte {0x00});
    }
    {
      Span<const byte> bytes = ByteOrderMarkScanner::GetBytes(ByteOrderMark::None);
      REQUIRE(bytes.size() == 0u);
    }
  }
}