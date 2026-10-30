#include "Krystal.Booey/DOM/Document.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/Core/EncodingRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/CustomEvent.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Booey/HTML/BeforeUnloadEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HashChangeEvent.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Storage/Storage.hpp"
#include "Krystal.Booey/Storage/StorageEvent.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    Ref<HTMLDocument> Parse(utf8_string &&input,
                            utf8_stringview url = u8"https://example.com/dir/page.html") noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      document->SetURL(url::URL(url));
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    // NOTE: `Document::Head()` returns null for a parsed document, so the head is found by position.
    RefPtr<Element> HeadOf(Document &document) noexcept
    {
      return document.DocumentElement()->FirstElementChild();
    }

    Ref<Element> CreateBase(Document &document, DOMString &&href) noexcept
    {
      auto base = document.CreateElement(u8"base").ReleaseValue();
      REQUIRE_FALSE(base->SetAttribute(u8"href", krys::move(href)).HasException());
      return base;
    }
  }

  TEST_CASE("Document::URL", "[DOM][Document]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("A new document's URL is about:blank")
    {
      REQUIRE(document->URL() == u8"about:blank");
      REQUIRE(document->DocumentURI() == u8"about:blank");
      REQUIRE(document->BaseURL() == url::AboutBlankURL());
      REQUIRE(document->BaseURI() == u8"about:blank");
    }

    SECTION("Setting the URL changes the base URL")
    {
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
      REQUIRE(document->URL() == u8"https://example.com/dir/page.html");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");

      auto element = CreateRef<TestElement>(*document);
      REQUIRE(element->BaseURI() == u8"https://example.com/dir/page.html");
    }

    SECTION("An empty URL becomes about:blank")
    {
      document->SetURL(url::URL(u8"https://example.com/"));
      document->SetURL(url::URL());
      REQUIRE(document->URL() == u8"about:blank");
    }

    SECTION("CompleteURL resolves against the base URL")
    {
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
      REQUIRE(document->CompleteURL(u8"a.css") == u8"https://example.com/dir/a.css");
      REQUIRE(document->CompleteURL(u8"a.css", url::URL(u8"https://other.com/x/"))
              == u8"https://other.com/x/a.css");
    }

    SECTION("A cloned document keeps the URL")
    {
      document->SetURL(url::URL(u8"https://example.com/"));
      auto clone = document->CloneNode().ReleaseValue();
      REQUIRE(Downcast<Document>(clone.get())->URL() == u8"https://example.com/");
    }
  }

  TEST_CASE("Document base URL from base elements", "[DOM][Document]")
  {
    SECTION("The first base element with an href sets the base URL, relative to the document's URL")
    {
      auto document = Parse(u8"<base target=_top><base href=sub/><base href=https://other.com/>");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/sub/");
      REQUIRE(document->CompleteURL(u8"a.css") == u8"https://example.com/dir/sub/a.css");
    }

    SECTION("data: and javascript: base URLs are ignored")
    {
      REQUIRE(Parse(u8"<base href='data:text/plain,a'>")->BaseURL() == u8"https://example.com/dir/page.html");
      REQUIRE(Parse(u8"<base href='javascript:void(0)'>")->BaseURL()
              == u8"https://example.com/dir/page.html");
    }

    SECTION("An unparsable base URL is ignored")
    {
      REQUIRE(Parse(u8"<base href='https://[invalid'>")->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Changing or removing the href updates the base URL")
    {
      auto document = Parse(u8"<base href=a/>");
      auto base = HeadOf(*document)->FirstElementChild();
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(base->SetAttribute(u8"href", u8"b/").HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/b/");

      base->RemoveAttribute(u8"href");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Inserting and removing base elements updates the base URL")
    {
      auto document = Parse(u8"<base href=a/>");
      auto head = HeadOf(*document);
      auto first = head->FirstElementChild();

      auto earlier = CreateBase(*document, u8"b/");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(head->Prepend({Ref<Node>(earlier)}).HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/b/");

      REQUIRE_FALSE(earlier->Remove().HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(first->Remove().HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Base elements in shadow trees are ignored")
    {
      auto document = Parse(u8"<div></div>");
      auto host = document->Body()->FirstElementChild();
      auto shadow = host->AttachShadow({}).ReleaseValue();

      auto base = CreateBase(*document, u8"a/");
      REQUIRE_FALSE(shadow->Append({Ref<Node>(base)}).HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }
  }

  TEST_CASE("Document::CssParserContext", "[DOM][Document][CSS]")
  {
    SECTION("Follows the base URL and mode")
    {
      auto document = Parse(u8"<base href=sub/>");
      auto &context = document->CssParserContext();
      REQUIRE(context.BaseURL == u8"https://example.com/dir/sub/");
      REQUIRE(context.Mode == css::ParserMode::HTMLQuirks);
      REQUIRE(context.IsHTMLDocument);
      REQUIRE(context.HasDocumentSecurityOrigin);

      REQUIRE(Parse(u8"<!DOCTYPE html>")->CssParserContext().Mode == css::ParserMode::HTMLStandard);
    }

    SECTION("Is recreated when the base URL changes")
    {
      auto document = Parse(u8"<!DOCTYPE html>");
      REQUIRE(document->CssParserContext().BaseURL == u8"https://example.com/dir/page.html");

      document->SetURL(url::URL(u8"https://other.com/"));
      REQUIRE(document->CssParserContext().BaseURL == u8"https://other.com/");
    }

    SECTION("Isn't an HTML document's context for other documents")
    {
      auto document = CreateRef<Document>();
      REQUIRE_FALSE(document->CssParserContext().IsHTMLDocument);
    }

    SECTION("The Document constructors")
    {
      auto document = Parse(u8"<p>");

      REQUIRE(css::ParserContext(*document) == document->CssParserContext());
      REQUIRE(css::MediaQueryParserContext(*document).Context == document->CssParserContext());
      REQUIRE(css::SelectorParserContext(*document).Mode == css::ParserMode::HTMLQuirks);
      REQUIRE(css::SelectorParserContext(*document)
              == css::SelectorParserContext(document->CssParserContext()));

      css::ParserContext sheetContext(*document, url::URL(u8"https://cdn.com/a.css"), u8"utf-8");
      REQUIRE(sheetContext.BaseURL == u8"https://cdn.com/a.css");
      REQUIRE(sheetContext.Charset == u8"utf-8");
      REQUIRE(sheetContext.Mode == css::ParserMode::HTMLQuirks);
    }
  }

  TEST_CASE("Document::CreateEvent creates the interface its table names", "[DOM][Document][Event]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto create = [&document](DOMStringView interface)
    {
      auto event = document->CreateEvent(interface);
      REQUIRE_FALSE(event.HasException());
      return event.ReleaseValue();
    };

    CHECK(Is<html::BeforeUnloadEvent>(*create(u8"beforeunloadevent")));
    CHECK(Is<CompositionEvent>(*create(u8"compositionevent")));
    CHECK(dynamic_cast<CustomEvent<RefPtr<html::SerializedScriptValue>> *>(&*create(u8"customevent")));
    CHECK(Is<DragEvent>(*create(u8"dragevent")));
    CHECK(Is<FocusEvent>(*create(u8"focusevent")));
    CHECK(Is<html::HashChangeEvent>(*create(u8"hashchangeevent")));
    CHECK(Is<KeyboardEvent>(*create(u8"keyboardevent")));
    CHECK(Is<html::MessageEvent>(*create(u8"messageevent")));
    CHECK(Is<MouseEvent>(*create(u8"mouseevent")));
    CHECK(Is<MouseEvent>(*create(u8"mouseevents")));
    CHECK(Is<StorageEvent>(*create(u8"storageevent")));
    CHECK(Is<TextEvent>(*create(u8"textevent")));
    CHECK(Is<TouchEvent>(*create(u8"touchevent")));
    for (auto name : {u8"uievent", u8"uievents"})
    {
      auto event = create(name);
      CHECK(dynamic_cast<UIEvent *>(&*event));
      CHECK_FALSE(Is<MouseEvent>(*event));
    }
    for (auto name : {u8"event", u8"events", u8"htmlevents", u8"svgevents"})
    {
      auto event = create(name);
      CHECK_FALSE(dynamic_cast<UIEvent *>(&*event));
      CHECK_FALSE(Is<CompositionEvent>(*event));
    }

    SECTION("Case-insensitively")
    {
      CHECK(Is<MouseEvent>(*create(u8"MouseEvents")));
      CHECK(Is<KeyboardEvent>(*create(u8"KEYBOARDEVENT")));
    }
  }

  TEST_CASE("Document::CreateEvent throws for names outside its table", "[DOM][Document][Event]")
  {
    auto document = CreateRef<HTMLDocument>();
    for (auto name : {u8"", u8"foo", u8"mouseeventz", u8"devicemotionevent", u8"deviceorientationevent",
                      u8"wheelevent", u8"keyboardevents", u8"popstateevent", u8"mutationevent"})
    {
      auto event = document->CreateEvent(name);
      REQUIRE(event.HasException());
      CHECK(event.GetException().Code() == ExceptionCode::NotSupportedError);
    }
  }

  TEST_CASE("Document::CreateEvent's event is untrusted and uninitialized", "[DOM][Document][Event]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto &loop = html::EventLoop::Current();
    loop.Run(loop.Now() + std::chrono::milliseconds(3));
    auto event = document->CreateEvent(u8"beforeunloadevent").ReleaseValue();

    CHECK(event->Type().View().empty());
    CHECK_FALSE(event->Bubbles());
    CHECK_FALSE(event->Cancelable());
    CHECK_FALSE(event->IsTrusted());
    CHECK(event->TimeStamp() == loop.Now());

    auto element = document->CreateElement(u8"div").ReleaseValue();
    auto dispatched = element->DispatchEvent(*event);
    REQUIRE(dispatched.HasException());
    CHECK(dispatched.GetException().Code() == ExceptionCode::InvalidStateError);

    event->InitEvent(DOMStringAtom(u8"custom"), true, true);
    CHECK_FALSE(element->DispatchEvent(*event).HasException());
  }

  TEST_CASE("Document::CreateEvent with the legacy init methods", "[DOM][Document][Event]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("initMessageEvent")
    {
      auto event = document->CreateEvent(u8"messageevent").ReleaseValue();
      auto &message = Downcast<html::MessageEvent>(*event);
      message.InitMessageEvent(DOMStringAtom(u8"message"), true, false, nullptr, u8"https://example.com",
                               u8"7");
      CHECK(message.Type() == u8"message");
      CHECK(message.Bubbles());
      CHECK(message.Origin() == u8"https://example.com");
      CHECK(message.LastEventId() == u8"7");
      CHECK(message.Source() == nullptr);
    }

    SECTION("initStorageEvent")
    {
      auto event = document->CreateEvent(u8"storageevent").ReleaseValue();
      auto &storage = Downcast<StorageEvent>(*event);
      storage.InitStorageEvent(DOMStringAtom(u8"storage"), false, true, DOMString(u8"key"), null,
                               DOMString(u8"value"), u8"https://example.com/");
      CHECK(storage.Type() == u8"storage");
      CHECK(storage.Cancelable());
      CHECK(storage.Key() == DOMString(u8"key"));
      CHECK_FALSE(storage.OldValue());
      CHECK(storage.NewValue() == DOMString(u8"value"));
      CHECK(storage.URL() == u8"https://example.com/");
      CHECK(storage.StorageArea() == nullptr);
    }

    SECTION("Not while it's being dispatched")
    {
      auto event = document->CreateEvent(u8"storageevent").ReleaseValue();
      auto &storage = Downcast<StorageEvent>(*event);
      storage.InitStorageEvent(DOMStringAtom(u8"storage"));
      auto element = document->CreateElement(u8"div").ReleaseValue();
      auto handle =
        element->AddEventListener<StorageEvent>(DOMStringAtom(u8"storage"), [](StorageEvent &dispatched)
                                                { dispatched.InitStorageEvent(DOMStringAtom(u8"other")); });
      CHECK_FALSE(element->DispatchEvent(storage).HasException());
      CHECK(storage.Type() == u8"storage");
    }
  }

  TEST_CASE("Document's encoding", "[DOM][Document]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("A new document's is UTF-8")
    {
      CHECK(document->CharacterSet() == u8"UTF-8");
      CHECK(document->Charset() == u8"UTF-8");
      CHECK(document->InputEncoding() == u8"UTF-8");
    }

    SECTION("The one its loader gives it, which a clone keeps")
    {
      auto *windows1252 = EncodingRegistry().Find(u8"windows-1252");
      REQUIRE(windows1252);
      document->SetEncoding(*windows1252);
      CHECK(document->CharacterSet() == u8"windows-1252");
      CHECK(document->InputEncoding() == u8"windows-1252");

      auto clone = document->CloneNode();
      REQUIRE_FALSE(clone.HasException());
      CHECK(Downcast<Document>(*clone.Value()).CharacterSet() == u8"windows-1252");
    }
  }
}
