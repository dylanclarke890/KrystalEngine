#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.IO/Path.hpp"
#include <catch_all.hpp>

#include <cstring>
#include <fstream>

namespace Krys::Tests
{
  using namespace Krys::IO;

  TEST_CASE("VirtualFileSystem", "[IO][VFS]")
  {
    // Setup test directories
    Path baseDir = Path("./virtual_file_system_test");
    Path dataDir = Path("./virtual_file_system_test_override");
    Path imagesDir = dataDir / Path("images");

    // Clean up and create directory structure
    std::filesystem::remove_all(baseDir.ToString());
    std::filesystem::remove_all(dataDir.ToString());

    std::filesystem::create_directories(baseDir.ToString());
    std::filesystem::create_directories(dataDir.ToString());
    std::filesystem::create_directories(baseDir.ToString() + "/data");
    std::ofstream(baseDir.ToString() + "/data/fallback.txt");
    std::filesystem::create_directories(imagesDir.ToString());

    VirtualFileSystemBuilder builder;

    auto vfs = builder // Create the vfs
                 .Mount<NativeFileBackend>(Path("/"), baseDir)
                 .Mount<NativeFileBackend>(Path("/data"), dataDir)
                 .Build();

    REQUIRE(vfs != nullptr);
    REQUIRE(vfs->Exists(Path("/")));
    REQUIRE(vfs->Exists(Path("/data")));
    REQUIRE(vfs->Exists(Path("/data/fallback.txt"))); // should exist in baseDir
    REQUIRE(vfs->Exists(Path("/data/images")));
    REQUIRE_FALSE(vfs->Exists(Path("/test_file.txt")));

    REQUIRE(vfs->CreateFile(Path("/new_file.txt"), false));
    REQUIRE_FALSE(vfs->CreateFile(Path("/new_file.txt"), false)); // Should fail
    REQUIRE(vfs->DeleteFile(Path("/new_file.txt")));

    auto entries = vfs->SearchFiles(Path("/data"));
    REQUIRE_FALSE(entries.empty());

    vfs->CreateFile(Path("/test_file.txt"));
    REQUIRE(vfs->Exists(Path("/test_file.txt")));

    SECTION("File content")
    {
      const char *testContent = "test content";
      {
        auto writer = vfs->GetWriter(Path("/test_file.txt"), WriteFlags::None);
        REQUIRE(writer->Write(reinterpret_cast<const byte *>(testContent), strlen(testContent)));
      }

      {
        auto reader = vfs->GetReader(Path("/test_file.txt"), ReadFlags::None);
        char buffer[64] = {0};
        REQUIRE((size_t)reader->Read(reinterpret_cast<byte *>(buffer), sizeof(buffer))
                == strlen(testContent));
        REQUIRE(string(buffer) == string(testContent));
      }
    }
  }
}