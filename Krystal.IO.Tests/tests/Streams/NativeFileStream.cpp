#include "pch.hpp"

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include <fstream>
#include <vector>

namespace Krys::Tests
{
  using namespace Krys::IO;

  TEST_CASE("NativeFileReader", "[IO][Streams]")
  {
    // Setup test file
    const Path testFile = Path("test_read_file.bin");
    const std::vector<byte> testData = {byte {0x01}, byte {0x02}, byte {0x03}, byte {0x04}, byte {0x05}};

    // Create test file with data
    {
      std::ofstream file(testFile.ToString(), std::ios::binary);
      file.write(reinterpret_cast<const char *>(testData.data()), testData.size());
    }

    SECTION("Constructor and basic operations")
    {
      NativeFileReader reader(testFile);

      REQUIRE(reader.IsOpen()); // Should open in constructor
      REQUIRE((size_t)reader.Size() == testData.size());
      REQUIRE(reader.Position() == 0);
    }

    SECTION("Read operations")
    {
      NativeFileReader reader(testFile);

      byte buffer[5] = {};
      REQUIRE(reader.Read(buffer, 3) == 3);
      REQUIRE(buffer[0] == byte {0x01});
      REQUIRE(buffer[1] == byte {0x02});
      REQUIRE(buffer[2] == byte {0x03});
      REQUIRE(reader.Position() == 3);

      // Read remaining
      REQUIRE(reader.Read(buffer + 3, 2) == 2);
      REQUIRE(buffer[3] == byte {0x04});
      REQUIRE(buffer[4] == byte {0x05});
      REQUIRE(reader.Position() == 5);

      // EOF read
      REQUIRE(reader.Read(buffer, 1) == 0);
    }

    SECTION("Seek operations")
    {
      NativeFileReader reader(testFile);

      REQUIRE(reader.Seek(2, SeekOrigin::Begin));
      REQUIRE(reader.Position() == 2);

      REQUIRE(reader.Seek(1, SeekOrigin::Current));
      REQUIRE(reader.Position() == 3);

      REQUIRE(reader.Seek(-1, SeekOrigin::End));
      REQUIRE((size_t)reader.Position() == testData.size() - 1);

      // Invalid seeks
      REQUIRE_FALSE(reader.Seek(-1, SeekOrigin::Begin));
      REQUIRE_FALSE(reader.Seek(testData.size() + 1, SeekOrigin::Begin));
    }

    SECTION("Close and reopen")
    {
      NativeFileReader reader(testFile);
      reader.Close();

      REQUIRE_FALSE(reader.IsOpen());
      REQUIRE(reader.Position() == 0); // Position should reset

      REQUIRE(reader.Open());
      REQUIRE(reader.IsOpen());
    }

    SECTION("Nonexistent file reading")
    {
      NativeFileReader reader(Path("nonexistent_file.bin"));
      REQUIRE_FALSE(reader.IsOpen());
      REQUIRE_FALSE(reader.Open());

      byte buffer[1];
      REQUIRE(reader.Read(buffer, 1) == 0);
    }

    // Cleanup
    std::filesystem::remove(testFile.ToString());
  }

  TEST_CASE("NativeFileWriter", "[IO][Streams]")
  {
    const Path testFile = Path("test_write_file.bin");

    // Ensure file doesn't exist before test
    std::filesystem::remove(testFile.ToString());

    SECTION("Constructor and basic operations")
    {
      NativeFileWriter writer(testFile);

      REQUIRE(writer.IsOpen()); // Should open in constructor
      REQUIRE(writer.Size() == 0);
      REQUIRE(writer.Position() == 0);
    }

    SECTION("Write operations")
    {
      NativeFileWriter writer(testFile);
      const std::vector<byte> testData = {byte {0xAA}, byte {0xBB}, byte {0xCC}};

      REQUIRE(writer.Write(testData.data(), testData.size()));
      REQUIRE((size_t)writer.Position() == testData.size());
      REQUIRE((size_t)writer.Size() == testData.size());

      // Verify file contents
      writer.Close();
      std::ifstream file(testFile.ToString(), std::ios::binary);
      byte buffer[3] = {byte {0}};
      file.read(reinterpret_cast<char *>(buffer), 3);
      REQUIRE(buffer[0] == byte {0xAA});
      REQUIRE(buffer[1] == byte {0xBB});
      REQUIRE(buffer[2] == byte {0xCC});
    }

    SECTION("Seek and write")
    {
      NativeFileWriter writer(testFile);
      const byte data1 = byte {0x11};
      const byte data2 = byte {0x22};

      REQUIRE(writer.Write(&data1, 1));
      REQUIRE(writer.Seek(10, SeekOrigin::Begin));
      REQUIRE(writer.Write(&data2, 1));

      // File should be 11 bytes with data at positions 0 and 10
      REQUIRE(writer.Size() == 11);

      // Verify contents
      writer.Close();
      std::ifstream file(testFile.ToString(), std::ios::binary);
      byte buffer[11] = {};
      file.read(reinterpret_cast<char *>(buffer), 11);
      REQUIRE(buffer[0] == byte {0x11});
      REQUIRE(buffer[10] == byte {0x22});
    }

    SECTION("Overwrite existing file")
    {
      // Create initial file
      {
        NativeFileWriter writer(testFile);
        const byte data = byte {0xFF};
        writer.Write(&data, 1);
      }

      // Open again and overwrite
      NativeFileWriter writer(testFile);
      const byte newData = byte {0xEE};
      REQUIRE(writer.Write(&newData, 1));
      REQUIRE(writer.Size() == 1); // Should truncate
    }

    SECTION("Invalid directory writing")
    {
      NativeFileWriter writer(Path("/invalid/path/test.bin"));
      REQUIRE_FALSE(writer.IsOpen());
      REQUIRE_FALSE(writer.Open());
    }

    // Cleanup
    std::filesystem::remove(testFile.ToString());
  }
}