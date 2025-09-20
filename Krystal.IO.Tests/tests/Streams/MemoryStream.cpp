#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.IO/Common.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::IO;

  TEST_CASE("MemoryStreamReader", "[IO][Streams]")
  {
    List<byte> testData = {byte {0x01}, byte {0x02}, byte {0x03}, byte {0x04}, byte {0x05}};

    SECTION("Basic Operations")
    {
      MemoryStreamReader reader(testData);

      REQUIRE(reader.Open());
      REQUIRE(reader.IsOpen());
      REQUIRE(reader.Size() == 5);
      REQUIRE(reader.Position() == 0);

      reader.Close();
      REQUIRE_FALSE(reader.IsOpen());
    }

    SECTION("Read Operations")
    {
      MemoryStreamReader reader(testData);
      reader.Open();

      byte buffer[3] = {byte {0}, byte {0}, byte {0}};
      REQUIRE(reader.Read(buffer, 3) == 3);
      REQUIRE(buffer[0] == byte {0x01});
      REQUIRE(buffer[1] == byte {0x02});
      REQUIRE(buffer[2] == byte {0x03});
      REQUIRE(reader.Position() == 3);

      REQUIRE(reader.Read(buffer, 3) == 2); // Only 2 bytes left
      REQUIRE(buffer[0] == byte {0x04});
      REQUIRE(buffer[1] == byte {0x05});
      REQUIRE(reader.Position() == 5);

      REQUIRE(reader.Read(buffer, 1) == 0); // EOF
    }

    SECTION("Seek Operations")
    {
      MemoryStreamReader reader(testData);
      reader.Open();

      REQUIRE(reader.Seek(2, SeekOrigin::Begin));
      REQUIRE(reader.Position() == 2);

      REQUIRE(reader.Seek(1, SeekOrigin::Current));
      REQUIRE(reader.Position() == 3);

      REQUIRE(reader.Seek(-1, SeekOrigin::End));
      REQUIRE(reader.Position() == 4);

      // Invalid seeks
      REQUIRE_FALSE(reader.Seek(-1, SeekOrigin::Begin));
      REQUIRE_FALSE(reader.Seek(10, SeekOrigin::Begin));
      REQUIRE_FALSE(reader.Seek(-10, SeekOrigin::Current));
    }

    SECTION("Edge Cases")
    {
      List<byte> emptyData;
      List<byte> singleByte = {byte {0x99}};

      SECTION("Empty Buffer")
      {
        MemoryStreamReader reader(emptyData);
        reader.Open();

        REQUIRE(reader.Size() == 0);
        REQUIRE(reader.Position() == 0);

        byte buffer[1] = {byte {0}};
        REQUIRE(reader.Read(buffer, 1) == 0);
        REQUIRE_FALSE(reader.Seek(1, SeekOrigin::Begin));
      }

      SECTION("Single Byte Buffer")
      {
        MemoryStreamReader reader(singleByte);
        reader.Open();

        byte buffer[1] = {byte {0}};
        REQUIRE(reader.Read(buffer, 1) == 1);
        REQUIRE(buffer[0] == byte {0x99});
        REQUIRE(reader.Read(buffer, 1) == 0); // EOF

        REQUIRE(reader.Seek(0, SeekOrigin::Begin));
        REQUIRE(reader.Read(buffer, 1) == 1);
      }
    }

    SECTION("Buffer Access")
    {
      List<byte> bufferAccessData = {byte {0x0A}, byte {0x0B}, byte {0x0C}};
      MemoryStreamReader reader(bufferAccessData);
      reader.Open();

      byte buffer[2] = {byte {0}, byte {0}};
      REQUIRE(reader.Read(buffer, 2) == 2);
      REQUIRE(buffer[0] == byte {0x0A});
      REQUIRE(buffer[1] == byte {0x0B});

      // Original data modification affects reader
      bufferAccessData[2] = byte {0xFF};
      REQUIRE(reader.Read(buffer, 1) == 1);
      REQUIRE(buffer[0] == byte {0xFF});
    }
  }

  TEST_CASE("MemoryStreamWriter", "[IO][Streams]")
  {
    SECTION("Basic Operations")
    {
      List<byte> buffer;
      MemoryStreamWriter writer(buffer);

      REQUIRE(writer.IsOpen());
      REQUIRE(writer.Size() == 0);
      REQUIRE(writer.Position() == 0);

      writer.Close();
      REQUIRE_FALSE(writer.IsOpen());
    }

    SECTION("Write Operations")
    {
      List<byte> buffer;
      MemoryStreamWriter writer(buffer);

      const byte data1[] = {byte {0x01}, byte {0x02}, byte {0x03}};
      REQUIRE(writer.Write(data1, 3));
      REQUIRE(writer.Position() == 3);
      REQUIRE(writer.Size() == 3);

      const byte data2[] = {byte {0x04}};
      REQUIRE(writer.Write(data2, 1));
      REQUIRE(writer.Position() == 4);
      REQUIRE(writer.Size() == 4);
    }

    SECTION("Seek Operations")
    {
      List<byte> buffer(10);
      MemoryStreamWriter writer(buffer);

      REQUIRE(writer.Seek(2, SeekOrigin::Begin));
      REQUIRE(writer.Position() == 2);

      REQUIRE(writer.Seek(1, SeekOrigin::Current));
      REQUIRE(writer.Position() == 3);

      REQUIRE(writer.Seek(-1, SeekOrigin::End));
      REQUIRE(writer.Position() == 9);
      REQUIRE(writer.Seek(11, SeekOrigin::Begin));

      REQUIRE(writer.Seek(0, SeekOrigin::Begin));
      // Invalid seeks
      REQUIRE_FALSE(writer.Seek(-1, SeekOrigin::Begin));
    }

    SECTION("Empty Buffer")
    {
      List<byte> buffer;
      MemoryStreamWriter writer(buffer);

      REQUIRE(writer.Size() == 0);
      REQUIRE(writer.Position() == 0);

      const byte data[] = {byte {0x11}};
      REQUIRE(writer.Write(data, 1));
      REQUIRE(writer.Size() == 1);
    }

    SECTION("Resizing Behavior")
    {
      List<byte> buffer(2);
      MemoryStreamWriter writer(buffer);

      writer.Seek(0, SeekOrigin::End); // go to end
      const byte data[] = {byte {0x22}};
      REQUIRE(writer.Write(data, 1));
      REQUIRE(writer.Size() == 3); // Should have resized
    }

    SECTION("Invalid Operations")
    {
      List<byte> buffer;
      MemoryStreamWriter writer(buffer);

      REQUIRE_FALSE(writer.Write(nullptr, 1)); // Null source
    }

    SECTION("Buffer Access")
    {
      List<byte> buffer(3);
      MemoryStreamWriter writer(buffer);

      const byte data[] = {byte {0xBB}, byte {0xCC}};
      REQUIRE(writer.Write(data, 2));
      REQUIRE(buffer[0] == byte {0xBB});
      REQUIRE(buffer[1] == byte {0xCC});
    }
  }

  TEST_CASE("MemoryStreamWriteThenRead", "[IO][Streams]")
  {
    SECTION("Roundtrip Test")
    {
      List<byte> buffer(3);

      // write data
      {
        MemoryStreamWriter writer(buffer);
        const byte data[] = {byte {0x33}, byte {0x44}, byte {0x55}};
        REQUIRE(writer.Write(data, 3));
      }

      // read back
      {
        MemoryStreamReader reader(buffer);
        byte readData[3] = {};
        REQUIRE(reader.Read(readData, 3) == 3);
        REQUIRE(readData[0] == byte {0x33});
        REQUIRE(readData[1] == byte {0x44});
        REQUIRE(readData[2] == byte {0x55});
      }
    }
  }
}