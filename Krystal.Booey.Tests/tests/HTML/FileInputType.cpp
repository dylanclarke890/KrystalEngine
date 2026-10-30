#include "Krystal.Booey/HTML/FileInputType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/DOM/FormDataEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/Network/FormData.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::EventLog;
  using boo::tests::FormFixture;
  using boo::tests::Str;

  namespace
  {
    KRYS_NODISCARD RefPtr<FileList> Files(std::initializer_list<utf8_stringview> paths) noexcept
    {
      List<Ref<File>> files;
      for (auto path : paths)
      {
        files.push_back(File::Create(path));
      }
      return FileList::Create(krys::move(files));
    }

    /// @brief The entries of each formdata event, as "name=file:name:type" or "name=value".
    struct FormDataEntries : dom::EventListenerCallback
    {
      List<std::string> Entries;

      void HandleEvent(dom::Event &event) noexcept override
      {
        for (auto &item : Downcast<dom::FormDataEvent>(event).FormData().Items())
        {
          if (auto *file = std::get_if<RefPtr<File>>(&item.Data))
          {
            Entries.push_back(Str(item.Name) + "=file:" + Str((*file)->Name()) + ":" + Str((*file)->Type()));
          }
          else
          {
            Entries.push_back(Str(item.Name) + "=" + Str(std::get<dom::DOMString>(item.Data)));
          }
        }
      }
    };
  }

  TEST_CASE("FileInputType: files", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8"<input id=text><input id=f type=file required>");
    auto &input = fixture.Get<HTMLInputElement>(u8"f");
    auto &text = fixture.Get<HTMLInputElement>(u8"text");

    // Only file inputs have files.
    CHECK(text.Files() == nullptr);
    text.SetFiles(Files({u8"C:/a.txt"}));
    REQUIRE(input.Files());
    CHECK(input.Files()->IsEmpty());
    CHECK(Str(input.Value()).empty());
    CHECK_FALSE(input.CheckValidity());

    auto log = EventLog::ListenTo(input, {u8"input", u8"change", u8"cancel"});

    SECTION("Chosen files fire input and change, and set the value")
    {
      input.SetFiles(Files({u8"C:/dir/first.txt", u8"C:/dir/second.png"}));
      CHECK(log->Joined() == "input change");
      CHECK(input.Files()->Length() == 2);
      CHECK(Str(input.Value()) == "C:\\fakepath\\first.txt");
      CHECK(input.CheckValidity());

      // The same files again are a cancel.
      log->Types.clear();
      input.SetFiles(Files({u8"C:/dir/first.txt", u8"C:/dir/second.png"}));
      CHECK(log->Joined() == "cancel");

      // Set by script, no events.
      log->Types.clear();
      input.SetFiles(Files({u8"C:/other.txt"}), WasSetByJavaScript::Yes);
      CHECK(log->Types.empty());
      CHECK(Str(input.Value()) == "C:\\fakepath\\other.txt");

      // Null is ignored.
      input.SetFiles(nullptr);
      CHECK(input.Files()->Length() == 1);
    }

    SECTION("Only the empty string can be set as the value, which clears the files")
    {
      input.SetFiles(Files({u8"C:/a.txt"}));
      CHECK(input.SetValue(u8"C:\\fakepath\\b.txt").HasException());
      CHECK(input.Files()->Length() == 1);
      CHECK_FALSE(input.SetValue(u8"").HasException());
      CHECK(input.Files()->IsEmpty());
      CHECK(Str(input.Value()).empty());
      CHECK_FALSE(input.CheckValidity());
    }

    SECTION("The files are saved and restored with their names")
    {
      CHECK(input.SaveFormControlState().empty());
      List<Ref<File>> files;
      files.push_back(File::Create(u8"C:/dir/a.txt", u8"shown.txt"));
      input.SetFiles(FileList::Create(krys::move(files)));
      auto state = input.SaveFormControlState();
      CHECK(state == FormControlState {u8"C:/dir/a.txt", u8"shown.txt", u8""});

      input.SetFiles(FileList::Create());
      log->Types.clear();
      input.RestoreFormControlState(state);
      CHECK(log->Joined() == "input change");
      REQUIRE(input.Files()->Length() == 1);
      CHECK(input.Files()->GetFile(0).Path() == u8"C:/dir/a.txt");
      CHECK(input.Files()->GetFile(0).Name() == u8"shown.txt");
    }
  }

  TEST_CASE("FileInputType: form data", "[HTML][Forms][Input]")
  {
    FormFixture fixture(u8R"(<form id=form action="https://example.com/upload" method=post
      enctype="multipart/form-data"><input id=f type=file name=up multiple><input name=t value=v></form>)");
    auto &form = fixture.Get<HTMLFormElement>(u8"form");
    auto &input = fixture.Get<HTMLInputElement>(u8"f");
    auto entries = CreateRef<FormDataEntries>();
    form.AddEventListener(dom::DOMStringAtom(u8"formdata"), Ref<dom::EventListenerCallback> {entries}, false);

    SECTION("Without files, an empty file")
    {
      REQUIRE_FALSE(DOMFormData::Create(&form, nullptr).HasException());
      CHECK(entries->Entries == List<std::string> {"up=file::application/octet-stream", "t=v"});
    }

    SECTION("Each file is an entry")
    {
      input.SetFiles(Files({u8"C:/a.txt", u8"C:/b.png"}));
      REQUIRE_FALSE(DOMFormData::Create(&form, nullptr).HasException());
      CHECK(entries->Entries
            == List<std::string> {"up=file:a.txt:text/plain", "up=file:b.png:image/png", "t=v"});
    }

    SECTION("Multipart bodies carry the files; the others their names")
    {
      input.SetFiles(Files({u8"C:/a.txt"}));
      auto entryList = DOMFormData::Create(&form, nullptr);
      REQUIRE_FALSE(entryList.HasException());

      auto multipart = FormData::CreateMultiPart(*entryList.Value());
      auto &elements = multipart->Elements();
      REQUIRE(elements.size() == 3);
      auto *header = std::get_if<List<uint8>>(&elements[0].Data);
      REQUIRE(header);
      auto headerText = std::string(header->begin(), header->end());
      CHECK(headerText.ends_with("Content-Disposition: form-data; name=\"up\"; filename=\"a.txt\"\r\n"
                                 "Content-Type: text/plain\r\n\r\n"));
      auto *file = std::get_if<FormDataElement::EncodedFileData>(&elements[1].Data);
      REQUIRE(file);
      CHECK(file->Filename == u8"C:/a.txt");
      CHECK(file->FileStart == 0);
      CHECK(file->FileLength == BlobDataItem::ToEndOfFile);
      // The flattened body leaves the file out.
      CHECK(Str(multipart->FlattenToString()).find("name=\"t\"\r\n\r\nv\r\n") != std::string::npos);

      auto urlEncoded = FormData::Create(*entryList.Value());
      CHECK(Str(urlEncoded->FlattenToString()) == "up=a.txt&t=v");
    }

    SECTION("Blobs that aren't on disk are their bytes")
    {
      auto formData = DOMFormData::Create();
      List<BlobPartVariant> parts;
      parts.emplace_back(utf8_string(u8"bytes"));
      auto blob = Blob::Create(krys::move(parts), {});
      formData->Append(u8"b", *blob);
      formData->Append(u8"n", *blob, utf8_stringview(u8"named.bin"));
      auto &items = formData->Items();
      REQUIRE(items.size() == 2);
      CHECK(std::get<RefPtr<File>>(items[0].Data)->Name() == u8"blob");
      CHECK(std::get<RefPtr<File>>(items[1].Data)->Name() == u8"named.bin");

      auto multipart = FormData::CreateMultiPart(*formData);
      auto body = Str(multipart->FlattenToString());
      CHECK(body.find("filename=\"blob\"\r\nContent-Type: application/octet-stream\r\n\r\nbytes\r\n")
            != std::string::npos);

      // Setting replaces the entries of that name.
      formData->Set(u8"b", *File::Create(u8"C:/c.txt"));
      REQUIRE(formData->Items().size() == 2);
      CHECK(std::get<RefPtr<File>>(formData->Items()[0].Data)->Path() == u8"C:/c.txt");
    }
  }
}
