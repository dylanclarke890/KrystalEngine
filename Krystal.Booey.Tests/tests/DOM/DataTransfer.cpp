#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/DataTransferItem.hpp"
#include "Krystal.Booey/DOM/DataTransferItemList.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"
#include "Krystal.Booey/Platform/StaticPasteboard.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using dom::DataTransfer;
    using StoreMode = DataTransfer::StoreMode;

    KRYS_NODISCARD List<utf8_string> Strings(std::initializer_list<utf8_stringview> strings) noexcept
    {
      List<utf8_string> list;
      for (auto string : strings)
      {
        list.emplace_back(string);
      }
      return list;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DataTransfer: data", "[DOM][DataTransfer]")
  {
    EditorFixture fixture(u8"<p>x</p>");
    auto &document = *fixture.Document;

    SECTION("A new data transfer can be read and written, by normalised types, in the order written")
    {
      auto dataTransfer = DataTransfer::Create();
      CHECK(dataTransfer->CanReadTypes());
      CHECK(dataTransfer->CanReadData());
      CHECK(dataTransfer->CanWriteData());

      dataTransfer->SetData(document, u8"Text", u8"plain");
      dataTransfer->SetData(document, u8" URL ", u8"https://example.com/a");
      dataTransfer->SetData(document, u8"text/html;charset=utf-8", u8"<b>bold</b>");
      dataTransfer->SetData(document, u8"Application/X-Custom", u8"custom");
      CHECK(dataTransfer->Types(document)
            == Strings({u8"text/plain", u8"text/uri-list", u8"text/html", u8"application/x-custom"}));
      CHECK(dataTransfer->GetData(document, u8"text/plain;charset=utf-8") == u8"plain");
      CHECK(dataTransfer->GetData(document, u8"url") == u8"https://example.com/a");
      CHECK(dataTransfer->GetData(document, u8"TEXT/HTML") == u8"<b>bold</b>");
      CHECK(dataTransfer->GetData(document, u8"application/x-custom") == u8"custom");
      CHECK(dataTransfer->GetData(document, u8"application/x-missing").empty());

      // Writing a type again moves it to the end.
      dataTransfer->SetData(document, u8"text", u8"again");
      CHECK(dataTransfer->Types(document).back() == u8"text/plain");
      CHECK(dataTransfer->GetData(document, u8"text") == u8"again");

      dataTransfer->ClearData(u8"URL");
      CHECK(dataTransfer->Types(document)
            == Strings({u8"text/html", u8"application/x-custom", u8"text/plain"}));
      dataTransfer->ClearData();
      CHECK(dataTransfer->Types(document).empty());
    }

    SECTION("Only the safe types are written for other applications, sanitised")
    {
      auto dataTransfer = DataTransfer::Create();
      dataTransfer->SetData(document, u8"text/html", u8"<b>bold</b><script>alert(1)</script>");
      dataTransfer->SetData(document, u8"text/uri-list", u8"not a url");
      dataTransfer->SetData(document, u8"application/x-custom", u8"custom");

      // The page reads back what it wrote.
      CHECK(dataTransfer->GetData(document, u8"text/html") == u8"<b>bold</b><script>alert(1)</script>");
      CHECK(dataTransfer->GetData(document, u8"text/uri-list") == u8"not a url");

      auto &pasteboard = Downcast<StaticPasteboard>(dataTransfer->GetPasteboard());
      auto customData = pasteboard.TakeCustomData();
      auto markup = customData.ReadString(u8"text/html");
      CHECK(Narrow(markup).contains("bold"));
      CHECK_FALSE(Narrow(markup).contains("script"));
      CHECK(customData.ReadString(u8"text/uri-list").empty());
      CHECK(customData.ReadString(u8"application/x-custom").empty());
      CHECK(customData.ReadStringInCustomData(u8"application/x-custom") == u8"custom");
      CHECK(customData.ReadStringInCustomData(u8"text/uri-list") == u8"not a url");
    }

    SECTION("The store's mode limits what can be done with it")
    {
      auto pasteboard = CreateUnique<StaticPasteboard>();
      pasteboard->WriteString(u8"text/plain", u8"text");

      auto readOnly =
        DataTransfer::CreateForCopyAndPaste(document, StoreMode::Readonly, krys::move(pasteboard));
      CHECK(readOnly->GetData(document, u8"text") == u8"text");
      readOnly->SetData(document, u8"text", u8"changed");
      readOnly->ClearData();
      CHECK(readOnly->GetData(document, u8"text") == u8"text");

      // Protected: the types, but not the data.
      auto protectedPasteboard = CreateUnique<StaticPasteboard>();
      protectedPasteboard->WriteString(u8"text/plain", u8"text");
      auto protectedTransfer =
        DataTransfer::CreateForCopyAndPaste(document, StoreMode::Protected, krys::move(protectedPasteboard));
      CHECK(protectedTransfer->Types(document) == Strings({u8"text/plain"}));
      CHECK(protectedTransfer->GetData(document, u8"text").empty());

      readOnly->MakeInvalidForSecurity();
      CHECK(readOnly->Types(document).empty());
      CHECK(readOnly->GetData(document, u8"text").empty());
    }

    SECTION("Drop effects only change during drags")
    {
      auto dataTransfer = DataTransfer::Create();
      CHECK(dataTransfer->DropEffect() == u8"none");
      CHECK(dataTransfer->EffectAllowed() == u8"none");
      dataTransfer->SetDropEffect(u8"copy");
      dataTransfer->SetEffectAllowed(u8"all");
      CHECK(dataTransfer->DropEffect() == u8"none");
      CHECK(dataTransfer->EffectAllowed() == u8"none");
    }

    SECTION("An input event's data can be read, not written")
    {
      auto dataTransfer = DataTransfer::CreateForInputEvent(u8"text", u8"<i>text</i>");
      CHECK(dataTransfer->Types(document) == Strings({u8"text/plain", u8"text/html"}));
      CHECK(dataTransfer->GetData(document, u8"text/html") == u8"<i>text</i>");
      CHECK_FALSE(dataTransfer->CanWriteData());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DataTransfer: items", "[DOM][DataTransfer]")
  {
    EditorFixture fixture(u8"<p>x</p>");
    auto &document = *fixture.Document;
    auto dataTransfer = DataTransfer::Create();
    dataTransfer->SetData(document, u8"text/plain", u8"plain");

    auto &items = dataTransfer->Items(document);
    REQUIRE(items.Length() == 1);
    auto plain = items.Item(0);
    CHECK(plain->GetKind() == u8"string");
    CHECK(plain->Type() == u8"text/plain");
    CHECK_FALSE(items.Item(1));

    SECTION("Strings are added and removed with the data")
    {
      auto added = items.Add(document, u8"<b>b</b>", u8"Text/HTML");
      REQUIRE_FALSE(added.HasException());
      CHECK(added.Value()->Type() == u8"text/html");
      CHECK(dataTransfer->GetData(document, u8"text/html") == u8"<b>b</b>");

      auto duplicate = items.Add(document, u8"again", u8"TEXT/plain");
      REQUIRE(duplicate.HasException());
      CHECK(duplicate.GetException().Code() == dom::ExceptionCode::NotSupportedError);

      dataTransfer->SetData(document, u8"application/x-custom", u8"custom");
      CHECK(items.Length() == 3);
      dataTransfer->ClearData(u8"text/plain");
      CHECK(items.Length() == 2);
      CHECK(plain->Type().empty());

      CHECK_FALSE(items.Remove(0).HasException());
      CHECK(dataTransfer->GetData(document, u8"text/html").empty());
      items.Clear();
      CHECK(items.Length() == 0);
      CHECK(dataTransfer->Types(document).empty());
    }

    SECTION("Files are added as file items, and listed with the files")
    {
      auto file =
        File::Create({BlobPartVariant(utf8_string(u8"contents"))}, u8"notes.txt", {{u8"text/plain"}});
      auto &files = dataTransfer->Files(document);
      CHECK(files.Length() == 0);

      auto item = items.Add(Ref<File>(file));
      REQUIRE(item);
      CHECK(item->GetKind() == u8"file");
      CHECK(item->Type() == u8"text/plain");
      CHECK(item->GetAsFile().get() == file.get());
      CHECK(files.Length() == 1);
      CHECK(dataTransfer->Types(document) == Strings({u8"Files", u8"text/plain"}));

      CHECK_FALSE(items.Remove(1).HasException());
      CHECK(files.Length() == 0);
    }

    SECTION("A string item's string is given in a task")
    {
      utf8_string string;
      plain->GetAsString(document, [&](const dom::DOMString &value) { string = value; });
      CHECK(string.empty());
      html::EventLoop::Current().RunUntilIdle();
      CHECK(string == u8"plain");
    }

    SECTION("Nothing can be added to data that can't be written")
    {
      auto readOnly = DataTransfer::CreateForInputEvent(u8"text", u8"");
      auto &readOnlyItems = readOnly->Items(document);
      auto added = readOnlyItems.Add(document, u8"x", u8"application/x-custom");
      REQUIRE_FALSE(added.HasException());
      CHECK_FALSE(added.Value());
      CHECK(readOnlyItems.Remove(0).HasException());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DataTransfer: files on the clipboard", "[DOM][DataTransfer]")
  {
    EditorFixture fixture(u8"<p>x</p>");
    auto &document = *fixture.Document;
    fixture.Pasteboard.Text = u8"C:\\files\\notes.txt";
    fixture.Pasteboard.FilePaths = {u8"C:\\files\\notes.txt", u8"C:\\files\\photo.png"};

    auto dataTransfer =
      DataTransfer::CreateForCopyAndPaste(document, StoreMode::Readonly, Pasteboard::CreateForCopyAndPaste());

    // The files are listed, but their paths aren't exposed as text.
    auto &files = dataTransfer->Files(document);
    REQUIRE(files.Length() == 2);
    CHECK(files.Item(0)->Name() == u8"notes.txt");
    CHECK(files.Item(1)->Type() == u8"image/png");
    CHECK(dataTransfer->Types(document) == Strings({u8"Files", u8"text/uri-list"}));
    CHECK(dataTransfer->GetData(document, u8"text/plain").empty());
    CHECK(dataTransfer->HasFileOfType(u8"image/png"));
    CHECK_FALSE(dataTransfer->HasFileOfType(u8"image/gif"));

    auto &items = dataTransfer->Items(document);
    REQUIRE(items.Length() == 3);
    CHECK(items.Item(0)->Type() == u8"text/uri-list");
    CHECK(items.Item(2)->GetKind() == u8"file");
  }

  TEST_CASE("PasteboardCustomData: serialisation", "[DOM][DataTransfer]")
  {
    PasteboardCustomData data;
    data.SetOrigin(u8"null:origin");
    data.WriteString(u8"text/plain", u8"plain");
    data.WriteStringInCustomData(u8"application/x-a", u8"a");
    data.WriteStringInCustomData(u8"application/x-b", u8"");
    CHECK(data.HasSameOriginCustomData());

    auto read = PasteboardCustomData::FromSharedBuffer(*data.CreateSharedBuffer());
    CHECK(read.Origin() == u8"null:origin");
    CHECK(read.OrderedTypes() == Strings({u8"text/plain", u8"application/x-a", u8"application/x-b"}));
    CHECK_FALSE(read.ReadStringInCustomData(u8"text/plain"));
    CHECK(read.ReadStringInCustomData(u8"application/x-a") == u8"a");
    CHECK(read.ReadStringInCustomData(u8"application/x-b") == u8"");
    // The platform strings aren't serialised; they're on the platform pasteboard.
    CHECK(read.ReadString(u8"text/plain").empty());

    auto truncated = data.CreateSharedBuffer()->GetSpan();
    auto invalid =
      PasteboardCustomData::FromSharedBuffer(*SharedBuffer::Create(truncated.first(truncated.size() - 1)));
    CHECK_FALSE(invalid.HasData());
    CHECK(invalid.Origin().empty());
  }

  TEST_CASE("SelectionData: URLs and URI lists", "[DOM][DataTransfer]")
  {
    SelectionData data;
    data.SetURIList(u8"# comment\r\nfile:///C:/files/a%20b.txt\r\nhttps://example.com/\r\n");
    CHECK(data.GetURL().String() == u8"file:///C:/files/a%20b.txt");
    REQUIRE(data.Filenames().size() == 1);
    CHECK(Narrow(data.Filenames()[0]).ends_with("a b.txt"));

    SelectionData link;
    link.SetURL(url::URL(u8"https://example.com/"), u8"<Example>");
    CHECK(link.Text() == u8"https://example.com/");
    CHECK(link.URIList() == u8"https://example.com/");
    CHECK(link.Markup() == u8"<a href=\"https://example.com/\">&lt;Example&gt;</a>");

    SelectionData text;
    text.SetText(u8"a\u00A0b");
    CHECK(text.Text() == u8"a b");
  }
}
