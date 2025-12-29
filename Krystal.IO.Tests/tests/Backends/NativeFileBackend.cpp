#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/Path.hpp"
#include <catch_all.hpp>
#include <fstream>

namespace Krys::Tests
{
  using namespace Krys::IO;

  TEST_CASE("NativeFileBackend", "[IO][Backends]")
  {
    // Setup: Create a temp directory
    const string testDir = "./native_file_backend_test";
    std::filesystem::create_directory(testDir);
    Path root(testDir);
    NativeFileBackend backend(root);

    SECTION("File Creation")
    {
      auto path = Path("test.txt");
      REQUIRE_FALSE(backend.Exists(path));
      REQUIRE(backend.CreateFile(path, true));
      REQUIRE(backend.Exists(path));
      REQUIRE(backend.IsFile(path));
      REQUIRE_FALSE(backend.IsDirectory(path));

      REQUIRE_FALSE(backend.CreateFile(path, false));
      REQUIRE(backend.CreateFile(path, true));

      REQUIRE(backend.DeleteFile(path));
      REQUIRE_FALSE(backend.Exists(path));
      REQUIRE_FALSE(backend.IsFile(path));
      REQUIRE_FALSE(backend.DeleteFile(path));
    }

    SECTION("Directory Detection")
    {
      auto path = Path("subdir");
      auto fsPath = testDir + "/subdir";

      REQUIRE_FALSE(backend.Exists(path));
      REQUIRE_FALSE(backend.IsDirectory(path));

      std::filesystem::create_directory(fsPath);

      REQUIRE(backend.Exists(path));
      REQUIRE(backend.IsDirectory(path));
      REQUIRE_FALSE(backend.IsFile(path));

      std::filesystem::remove(fsPath);
    }

    SECTION("Read/Write Operations")
    {
      auto path = Path("data.bin");
      REQUIRE(backend.CreateFile(path, true));

      auto writer = backend.GetWriter(path, WriteFlags::None);
      REQUIRE(writer != nullptr);
      REQUIRE(writer->IsOpen());

      const char data[] = {0x01, 0x02, 0x03};
      REQUIRE(writer->Write(reinterpret_cast<const Krys::byte *>(data), sizeof(data)));
      writer->Close();

      auto reader = backend.GetReader(path, ReadFlags::None);
      REQUIRE(reader != nullptr);
      REQUIRE(reader->IsOpen());

      Krys::byte buffer[3];
      REQUIRE(reader->Read(buffer, sizeof(buffer)) == 3);
      REQUIRE(memcmp(data, buffer, 3) == 0);
    }

    SECTION("Directory Traversal")
    {
      std::ofstream(testDir + "/file1.txt").close();
      std::filesystem::create_directory(testDir + "/subdir");
      std::ofstream(testDir + "/subdir/file2.txt").close();

      {
        auto entries = backend.SearchFiles(Path(""), FileSearchFlags::None);
        REQUIRE(entries.size() == 1); // file1.txt
        REQUIRE(entries[0].RelativePath.Filename().ToString() == "file1.txt");
      }

      {
        auto entries = backend.SearchFiles(Path(""), FileSearchFlags::Recursive);
        REQUIRE(entries.size() == 2); // file1.txt and file2.txt
        REQUIRE(std::any_of(entries.begin(), entries.end(), [](const auto &e)
                            { return e.RelativePath.Filename().ToString() == "file1.txt"; }));
        REQUIRE(std::any_of(entries.begin(), entries.end(), [](const auto &e)
                            { return e.RelativePath.Filename().ToString() == "file2.txt"; }));
      }
    }

    // Teardown: Remove temp files
    std::filesystem::remove_all(testDir);
  }
}
