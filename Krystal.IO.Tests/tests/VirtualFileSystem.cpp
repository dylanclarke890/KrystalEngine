#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include <catch_all.hpp>
#include <filesystem>
#include <fstream>

namespace
{
  namespace fs = std::filesystem;

  using namespace Krys;
  using namespace Krys::IO;

  void WriteTextFile(const fs::path &p, stringview s)
  {
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    REQUIRE(f.is_open());
    f.write(s.data(), static_cast<std::streamsize>(s.size()));
    f.flush();
    REQUIRE(f.good());
  }

  string ReadAllText(IStreamReader &r)
  {
    string out;
    out.resize(static_cast<size_t>(r.Size()));
    const auto n = r.Read(reinterpret_cast<byte *>(out.data()), static_cast<uint64>(out.size()));
    out.resize(static_cast<size_t>(n));
    return out;
  }
}

TEST_CASE("VirtualFileSystem", "[IO][VFS]")
{
  // --- Build an isolated sandbox
  const fs::path base = fs::path("vfs_test_sandbox/base").lexically_normal();
  const fs::path data = fs::path("vfs_test_sandbox/override_data").lexically_normal();
  const fs::path baseData = base / "data";
  const fs::path baseDB = base / "database";
  const fs::path dataUI = data / "ui";
  const fs::path dataImg = data / "images";

  // Clean slate
  fs::remove_all(base);
  fs::remove_all(data);
  fs::create_directories(baseData);
  fs::create_directories(baseDB);
  fs::create_directories(dataUI);
  fs::create_directories(dataImg);

  // Base tree (mounted at "/")
  WriteTextFile(baseData / "fallback.txt", "base-fallback");
  WriteTextFile(baseData / "ui" / "theme.txt", "base-theme");
  WriteTextFile(baseData / "shared.txt", "base-shared");
  WriteTextFile(baseDB / "file.txt", "db-file");

  // Override tree (mounted at "/data")
  WriteTextFile(dataUI / "theme.txt", "override-theme");
  WriteTextFile(dataImg / "pic.png", "PNGDATA");
  WriteTextFile(data / "shared.txt", "override-shared"); // same logical file as base/data/shared.txt

  // --- Build VFS: more specific alias (/data) should win over (/)
  VirtualFileSystemBuilder builder;
  auto vfs = builder.Mount<NativeFileBackend>(Path("/"), Path(base))
               .Mount<NativeFileBackend>(Path("/data"), Path(data))
               .Build();

  REQUIRE(vfs != nullptr);

  SECTION("Mount visibility: roots and directories exist")
  {
    REQUIRE(vfs->Exists(Path("/")));
    REQUIRE(vfs->Exists(Path("/data")));
    REQUIRE(vfs->Exists(Path("/data/ui")));
    REQUIRE(vfs->Exists(Path("/database")));
    // Trailing slash should not matter
    REQUIRE(vfs->Exists(Path("/data/")));
  }

  SECTION("Precedence: longest component prefix wins for reads")
  {
    // Both base and override have /data/ui/theme.txt, override must win.
    auto r = vfs->GetReader(Path("/data/ui/theme.txt"), ReadFlags::None);
    REQUIRE(r != nullptr);
    REQUIRE(ReadAllText(*r) == "override-theme");
  }

  SECTION("Fallback: if specific mount lacks the file, fall back to less specific")
  {
    // Only base has /data/fallback.txt (under / mount at base/data/fallback.txt).
    auto r = vfs->GetReader(Path("/data/fallback.txt"), ReadFlags::None);
    REQUIRE(r != nullptr);
    REQUIRE(ReadAllText(*r) == "base-fallback");
  }

  SECTION("Writer: Create+Truncate allows creating brand new files in the right mount")
  {
    // Create a file under /data (should land in 'data' physical root).
    {
      auto w = vfs->GetWriter(Path("/data/new.txt"), WriteFlags::Create | WriteFlags::Truncate);
      REQUIRE(w != nullptr);
      const char *text = "hello-new";
      REQUIRE(w->Write(reinterpret_cast<const byte *>(text), static_cast<uint64>(std::strlen(text))));
      w->Flush();
    }
    {
      auto r = vfs->GetReader(Path("/data/new.txt"), ReadFlags::None);
      REQUIRE(r != nullptr);
      REQUIRE(ReadAllText(*r) == "hello-new");
    }

    // Create a file at the VFS root (should land in 'base' physical root).
    {
      auto w = vfs->GetWriter(Path("/new_root.txt"), WriteFlags::Create | WriteFlags::Truncate);
      REQUIRE(w != nullptr);
      const char *text = "root-file";
      REQUIRE(w->Write(reinterpret_cast<const byte *>(text), static_cast<uint64>(std::strlen(text))));
      w->Flush();
    }
    {
      auto r = vfs->GetReader(Path("/new_root.txt"), ReadFlags::None);
      REQUIRE(r != nullptr);
      REQUIRE(ReadAllText(*r) == "root-file");
    }
  }

  SECTION("Component-aware matching: '/data' must NOT match '/database'")
  {
    // If prefix matching were string-based, '/database/file.txt' might be routed to '/data' alias.
    // We only created this under base:/database/file.txt.
    REQUIRE(vfs->Exists(Path("/database/file.txt")));
    // And '/data/base' should not suddenly appear:
    REQUIRE_FALSE(vfs->Exists(Path("/data/base"))); // sanity check; nothing created here
  }

  SECTION("SearchFiles: non-recursive, de-dup by logical (virtual) path; precedence picks override")
  {
    // We expect to see 'shared.txt' only once, and it should refer to the override, not base.
    auto list = vfs->SearchFiles(Path("/data"), FileSearchFlags::None);
    REQUIRE_FALSE(list.empty());

    // There should be exactly one 'shared.txt'.
    size_t sharedCount = 0;
    bool sharedLooksOverride = false;
    for (const auto &e : list)
    {
      if (e.RelativePath.Filename().ToString() == "shared.txt")
      {
        ++sharedCount;
        // Heuristic: absolute path should point inside 'data' physical root if override won.
        // (Your VFSFileEntry holds AbsolutePath and RelativePath in your backend; adjust if different.)
        sharedLooksOverride = e.AbsolutePath.ToString().find(data.generic_string()) != std::string::npos;
      }
    }
    REQUIRE(sharedCount == 1);
    REQUIRE(sharedLooksOverride); // precedence should pick the override copy
  }

  SECTION("SearchFiles: recursive vs non-recursive")
  {
    auto flat = vfs->SearchFiles(Path("/data"), FileSearchFlags::None);
    auto rec = vfs->SearchFiles(Path("/data"), FileSearchFlags::Recursive);

    // Recursive must include nested files like ui/theme.txt and images/pic.png
    auto has_theme = false, has_pic = false;
    for (const auto &e : rec)
    {
      const auto s = e.RelativePath.ToString();
      if (s.find("ui/theme.txt") != std::string::npos)
        has_theme = true;
      if (s.find("images/pic.png") != std::string::npos)
        has_pic = true;
    }
    REQUIRE(has_theme);
    REQUIRE(has_pic);

    // Non-recursive should not include those nested files (directory-only depth)
    auto flat_has_theme = false, flat_has_pic = false;
    for (const auto &e : flat)
    {
      const auto s = e.RelativePath.ToString();
      if (s.find("ui/theme.txt") != std::string::npos)
        flat_has_theme = true;
      if (s.find("images/pic.png") != std::string::npos)
        flat_has_pic = true;
    }
    REQUIRE_FALSE(flat_has_theme);
    REQUIRE_FALSE(flat_has_pic);
  }

  SECTION("Delete and re-create file")
  {
    // Create under root, then delete, then ensure it’s gone, then create again and read.
    const Path p("/temp_delete.txt");

    {
      auto w = vfs->GetWriter(p, WriteFlags::Create | WriteFlags::Truncate);
      REQUIRE(w != nullptr);
      const char *msg = "to-be-deleted";
      REQUIRE(w->Write(reinterpret_cast<const byte *>(msg), static_cast<uint64>(std::strlen(msg))));
    }
    REQUIRE(vfs->Exists(p));
    REQUIRE(vfs->DeleteFile(p));
    REQUIRE_FALSE(vfs->Exists(p));

    {
      auto w = vfs->GetWriter(p, WriteFlags::Create | WriteFlags::Truncate);
      REQUIRE(w != nullptr);
      const char *msg = "recreated";
      REQUIRE(w->Write(reinterpret_cast<const byte *>(msg), static_cast<uint64>(std::strlen(msg))));
    }
    auto r = vfs->GetReader(p, ReadFlags::None);
    REQUIRE(r != nullptr);
    REQUIRE(ReadAllText(*r) == "recreated");
  }
}
