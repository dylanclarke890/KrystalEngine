#include "Krystal.Booey/FileAPI/Blob.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    KRYS_NODISCARD std::string Text(const Blob &blob) noexcept
    {
      auto text = blob.Text();
      REQUIRE(text.HasValue());
      return Narrow(text.Value());
    }

    KRYS_NODISCARD List<uint8> Bytes(std::string_view text) noexcept
    {
      return List<uint8>(text.begin(), text.end());
    }

    /// @brief A file in the temporary directory with the given contents, removed when it goes.
    struct TemporaryFile
    {
      std::filesystem::path Path;

      TemporaryFile(std::string_view name, std::string_view contents) noexcept
          : Path(std::filesystem::temp_directory_path() / "Krystal.Booey.Tests" / name)
      {
        std::filesystem::create_directories(Path.parent_path());
        std::ofstream file(Path, std::ios::binary);
        file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
      }

      ~TemporaryFile() noexcept
      {
        std::error_code error;
        std::filesystem::remove(Path, error);
      }

      KRYS_NODISCARD utf8_string PathString() const noexcept
      {
        return Path.u8string();
      }
    };
  }

  TEST_CASE("Blob: parts, types and slices", "[FileAPI]")
  {
    SECTION("Empty")
    {
      auto blob = Blob::Create();
      CHECK(blob->Size() == 0);
      CHECK(blob->Type().empty());
      CHECK(Text(*blob).empty());
      CHECK_FALSE(blob->IsFile());
    }

    SECTION("Parts: bytes, strings and other blobs, in order")
    {
      auto inner = Blob::Create(Bytes("inner"), u8"text/plain");
      List<BlobPartVariant> parts;
      parts.emplace_back(Bytes("ab"));
      parts.emplace_back(utf8_string(u8"cd"));
      parts.emplace_back(RefPtr<Blob>(inner));
      parts.emplace_back(utf8_string(u8"é"));
      auto blob = Blob::Create(krys::move(parts), {.Type = u8"Text/Plain;Charset=UTF-8"});
      CHECK(Text(*blob) == "abcdinner\xC3\xA9");
      CHECK(blob->Size() == 11);
      CHECK(blob->MemoryCost() == 11);
      // The neighbouring bytes and strings are one item; the blob's are its own.
      CHECK(blob->Data().Items().size() == 3);
      // The type is lowercased.
      CHECK(blob->Type() == u8"text/plain;charset=utf-8");
    }

    SECTION("Types outside printable ASCII are dropped")
    {
      CHECK(Blob::NormalizedContentType(u8"text/pl\tain").empty());
      CHECK(Blob::NormalizedContentType(u8"text/pl\u00E9in").empty());
      CHECK(Blob::NormalizedContentType(u8"IMAGE/PNG") == u8"image/png");
    }

    SECTION("Native line endings")
    {
      List<BlobPartVariant> parts;
      parts.emplace_back(utf8_string(u8"a\nb\r\nc\rd"));
      parts.emplace_back(Bytes("e\nf"));
      auto blob = Blob::Create(krys::move(parts), {.Endings = EndingType::Native});
      // On Windows, CRLF; bytes are left alone.
      CHECK(Text(*blob) == "a\r\nb\r\nc\r\nde\nf");

      List<BlobPartVariant> transparent;
      transparent.emplace_back(utf8_string(u8"a\nb"));
      CHECK(Text(*Blob::Create(krys::move(transparent), {})) == "a\nb");
    }

    SECTION("Slices")
    {
      List<BlobPartVariant> parts;
      parts.emplace_back(Bytes("0123"));
      parts.emplace_back(RefPtr<Blob>(Blob::Create(Bytes("4567"), {})));
      parts.emplace_back(utf8_string(u8"89"));
      auto blob = Blob::Create(krys::move(parts), {.Type = u8"text/plain"});

      // Across items.
      auto middle = blob->Slice(2, 7, u8"TEXT/X");
      CHECK(Text(*middle) == "23456");
      CHECK(middle->Size() == 5);
      CHECK(middle->Type() == u8"text/x");
      CHECK(middle->MemoryCost() == 5);

      // From the end.
      CHECK(Text(*blob->Slice(-3, -1, {})) == "78");
      CHECK(Text(*blob->Slice(-100, 2, {})) == "01");

      // Clamped.
      CHECK(Text(*blob->Slice(8, 100, {})) == "89");
      CHECK(blob->Slice(6, 3, {})->Size() == 0);
      CHECK(blob->Slice(10, 12, {})->Size() == 0);

      // A slice of a slice.
      CHECK(Text(*middle->Slice(1, 3, {})) == "34");
    }

    SECTION("Text decodes UTF-8")
    {
      // A byte order mark is dropped, and invalid bytes become replacement characters.
      auto blob = Blob::Create(Bytes("\xEF\xBB\xBF"
                                     "a\xFF"
                                     "b"),
                               {});
      CHECK(Text(*blob)
            == "a\xEF\xBF\xBD"
               "b");
      auto bytes = blob->Bytes();
      REQUIRE(bytes.HasValue());
      CHECK(bytes.Value().size() == 6);
    }
  }

  TEST_CASE("File: files on disk and from parts", "[FileAPI]")
  {
    SECTION("A file on disk")
    {
      TemporaryFile temporary("notes.txt", "hello, file");
      auto file = File::Create(temporary.PathString());
      CHECK(file->IsFile());
      CHECK(file->Name() == u8"notes.txt");
      CHECK(file->Path() == temporary.PathString());
      CHECK(file->Type() == u8"text/plain");
      CHECK(file->Size() == 11);
      CHECK(Text(*file) == "hello, file");
      CHECK_FALSE(file->IsDirectory());

      // Its modification time, in milliseconds since the epoch.
      using namespace std::chrono;
      auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
      CHECK(file->LastModified() <= now);
      CHECK(file->LastModified() > now - 60'000);

      // Slices read their range of the file.
      CHECK(Text(*file->Slice(7, 11, {})) == "file");

      // A name of its own.
      auto renamed = File::Create(temporary.PathString(), u8"shown.PNG");
      CHECK(renamed->Name() == u8"shown.PNG");
      CHECK(renamed->Type() == u8"image/png");
      CHECK(Text(*renamed) == "hello, file");
    }

    SECTION("A file that's gone")
    {
      utf8_string path;
      RefPtr<File> file;
      {
        TemporaryFile temporary("gone.bin", "x");
        path = temporary.PathString();
        file = File::Create(path);
      }
      CHECK(file->Type().empty());
      CHECK(file->Size() == 0);
      auto bytes = file->Bytes();
      REQUIRE(bytes.HasException());
      CHECK(bytes.GetException().Code() == dom::ExceptionCode::NotFoundError);
    }

    SECTION("A directory")
    {
      auto directory = File::Create(std::filesystem::temp_directory_path().u8string());
      CHECK(directory->IsDirectory());
    }

    SECTION("From parts")
    {
      List<BlobPartVariant> parts;
      parts.emplace_back(utf8_string(u8"made"));
      auto file = File::Create(krys::move(parts), u8"made.txt", {{.Type = u8"text/plain"}, 1'234});
      CHECK(file->Name() == u8"made.txt");
      CHECK(file->Path().empty());
      CHECK(file->Type() == u8"text/plain");
      CHECK(file->LastModified() == 1'234);
      CHECK(Text(*file) == "made");

      // Without a time, it's now.
      List<BlobPartVariant> moreParts;
      auto now = File::Create(krys::move(moreParts), u8"now", {});
      CHECK(now->LastModifiedOverride());
      CHECK(*now->LastModifiedOverride() > 0);
    }

    SECTION("From a blob or another file")
    {
      auto blob = Blob::Create(Bytes("data"), u8"application/x-test");
      auto file = File::Create(*blob, u8"named");
      CHECK(file->Name() == u8"named");
      CHECK(file->Type() == u8"application/x-test");
      CHECK(Text(*file) == "data");

      auto copy = File::Create(*file, std::nullopt);
      CHECK(copy->Name() == u8"named");
      CHECK(copy->LastModifiedOverride() == file->LastModifiedOverride());
      auto renamed = File::Create(*file, Maybe<utf8_stringview>(u8"other"));
      CHECK(renamed->Name() == u8"other");
      CHECK(Text(*renamed) == "data");
    }

    SECTION("Types by extension")
    {
      CHECK(File::ContentTypeForFile(u8"C:/a/b.HTML") == u8"text/html");
      CHECK(File::ContentTypeForFile(u8"photo.jpeg") == u8"image/jpeg");
      CHECK(File::ContentTypeForFile(u8"archive.unknown").empty());
      CHECK(File::ContentTypeForFile(u8"noextension").empty());
    }
  }

  TEST_CASE("FileList: items and paths", "[FileAPI]")
  {
    auto empty = FileList::Create();
    CHECK(empty->IsEmpty());
    CHECK(empty->Length() == 0);
    CHECK(empty->Item(0) == nullptr);

    List<Ref<File>> files;
    files.push_back(File::Create(u8"C:/one.txt"));
    files.push_back(File::Create(u8"C:/two.png"));
    auto list = FileList::Create(krys::move(files));
    CHECK(list->Length() == 2);
    REQUIRE(list->Item(1));
    CHECK(list->Item(1)->Name() == u8"two.png");
    CHECK(list->Item(2) == nullptr);
    CHECK(list->Paths() == List<utf8_string> {u8"C:/one.txt", u8"C:/two.png"});
  }
}
