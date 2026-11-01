#include "Krystal.Booey/DOM/Document.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/Core/EncodingRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/CustomEvent.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/BeforeUnloadEvent.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HashChangeEvent.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
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

  TEST_CASE("Document.URL and documentURI: the URL, serialised",
            "[DOM][Document][dom:dom-document-url][dom:dom-document-documenturi][dom:concept-document-url]"
            "[html:set-the-url][html:fallback-base-url][html:encoding-parsing-a-url][html:parse-a-url]")
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

  TEST_CASE("Document base URL from base elements",
            "[DOM][Document][dom:get-the-base-url][html:document-base-url][html:respond-to-base-url-changes]")
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

  TEST_CASE("Document: the CSS parser context", "[DOM][Document][CSS]")
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

  TEST_CASE("Document.createEvent(): the interface its table names",
            "[DOM][Document][Event][dom:dom-document-createevent]")
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

  TEST_CASE("Document.createEvent(): names outside its table throw",
            "[DOM][Document][Event][dom:dom-document-createevent]")
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

  TEST_CASE("Document.createEvent(): the event is untrusted and uninitialized, with an empty type",
            "[DOM][Document][Event][dom:dom-document-createevent]")
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

  TEST_CASE("Document.createEvent(): the event is initialised with the legacy init methods",
            "[DOM][Document][Event][dom:dom-document-createevent]")
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

  TEST_CASE(
    "Document.characterSet, charset and inputEncoding: the encoding's name",
    "[DOM][Document][dom:dom-document-characterset][dom:dom-document-charset][dom:dom-document-inputencoding]"
    "[dom:concept-document-encoding]")
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

  TEST_CASE(
    "new Document(): an XML document with the default URL, content type, encoding and mode",
    "[DOM][Document][dom:interface-document][dom:dom-document-document][dom:create-a-document]"
    "[dom:concept-document][dom:concept-document-content-type][dom:concept-document-type]"
    "[dom:concept-document-mode][dom:concept-document-no-quirks][dom:document-allow-declarative-shadow-roots]"
    "[dom:document-custom-element-registry][dom:concept-document-origin][dom:document-base-url-override]"
    "[dom:dom-document-contenttype]")
  {
    auto document = CreateRef<Document>();
    CHECK(&document->NodeDocument() == document.get());
    CHECK(document->URL() == u8"about:blank");
    CHECK(document->ContentType() == u8"application/xml");
    CHECK(document->CharacterSet() == u8"UTF-8");
    CHECK(document->CompatMode() == u8"CSS1Compat");
    CHECK(document->InNoQuirksMode());
    CHECK(document->CustomElementRegistry() == nullptr);
    CHECK_FALSE(Is<HTMLDocument>(*document));

    auto html = CreateRef<HTMLDocument>();
    CHECK(html->ContentType() == u8"text/html");
    CHECK(Is<HTMLDocument>(*html));
  }

  TEST_CASE("Document.implementation: the document's DOMImplementation, the same each time",
            "[DOM][Document][dom:dom-document-implementation]")
  {
    auto document = CreateRef<Document>();
    CHECK(&document->Implementation() == &document->Implementation());
    auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
    CHECK(&doctype->NodeDocument() == document.get());
  }

  TEST_CASE(
    "Document.compatMode: BackCompat in quirks mode, otherwise CSS1Compat",
    "[DOM][Document][dom:dom-document-compatmode][dom:concept-document-mode][dom:concept-document-quirks]"
    "[dom:concept-document-limited-quirks][dom:concept-document-no-quirks]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto parse = [&](DOMStringView markup)
    {
      return Document::ParseHTMLUnsafe(*context, markup).ReleaseValue();
    };

    auto quirks = parse(u8"<p>");
    CHECK(quirks->InQuirksMode());
    CHECK(quirks->CompatMode() == u8"BackCompat");

    auto limited = parse(u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"\"><p>");
    CHECK(limited->InLimitedQuirksMode());
    CHECK(limited->CompatMode() == u8"CSS1Compat");

    auto noQuirks = parse(u8"<!DOCTYPE html><p>");
    CHECK(noQuirks->InNoQuirksMode());
    CHECK(noQuirks->CompatMode() == u8"CSS1Compat");
  }

  TEST_CASE("Document.doctype and documentElement: the doctype child and the element child, or null",
            "[DOM][Document][dom:dom-document-doctype][dom:dom-document-documentelement]")
  {
    auto document = CreateRef<Document>();
    CHECK(document->DocType() == nullptr);
    CHECK(document->DocumentElement() == nullptr);

    auto comment = document->CreateComment(u8"c");
    auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
    auto element = document->CreateElement(u8"root").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*comment).HasException());
    REQUIRE_FALSE(document->AppendChild(*doctype).HasException());
    REQUIRE_FALSE(document->AppendChild(*element).HasException());

    CHECK(document->DocType() == doctype);
    CHECK(document->DocumentElement() == element);
  }

  TEST_CASE("Document.getElementsByTagName, getElementsByTagNameNS and getElementsByClassName: the lists for "
            "the document",
            "[DOM][Document][dom:dom-document-getelementsbytagname][dom:dom-document-getelementsbytagnamens]"
            "[dom:dom-document-getelementsbyclassname]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"html").ReleaseValue();
    auto paragraph = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(paragraph->SetAttribute(u8"class", u8"a b").HasException());
    REQUIRE_FALSE(root->AppendChild(*paragraph).HasException());
    REQUIRE_FALSE(document->AppendChild(*root).HasException());

    auto byTagName = document->GetElementsByTagName(u8"P");
    REQUIRE(byTagName->Length() == 1u);
    CHECK(byTagName->Item(0) == paragraph);
    CHECK(document->GetElementsByTagName(u8"*")->Length() == 2u);

    auto byNamespace = document->GetElementsByTagNameNS(infra::Namespaces::HTML, u8"p");
    REQUIRE(byNamespace->Length() == 1u);
    CHECK(byNamespace->Item(0) == paragraph);

    auto byClass = document->GetElementsByClassName(u8"b a");
    REQUIRE(byClass->Length() == 1u);
    CHECK(byClass->Item(0) == paragraph);
  }

  TEST_CASE("Document.createElement(): a valid local name, lowercased in an HTML document",
            "[DOM][Document][dom:dom-document-createelement][dom:concept-element-interface]")
  {
    SECTION("1. an invalid name throws")
    {
      auto document = CreateRef<HTMLDocument>();
      for (auto *name : {u8"", u8"1a", u8"a b", u8"a>"})
      {
        auto result = document->CreateElement(name);
        REQUIRE(result.HasException());
        CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }
    }

    SECTION("2. and 4. an HTML document's are lowercase HTML elements")
    {
      auto document = CreateRef<HTMLDocument>();
      auto element = document->CreateElement(u8"DIV").ReleaseValue();
      CHECK(element->LocalName() == u8"div");
      CHECK(element->NamespaceURI() == infra::Namespaces::HTML);
      CHECK(element->Prefix() == DOMStringAtom::Null());
      CHECK(Is<html::HTMLElement>(*element));
    }

    SECTION("4. an XML document's are in no namespace, as given")
    {
      auto document = CreateRef<XMLDocument>();
      auto element = document->CreateElement(u8"DIV").ReleaseValue();
      CHECK(element->LocalName() == u8"DIV");
      CHECK(element->NamespaceURI() == DOMStringAtom::Null());
    }

    SECTION("4. an XHTML document's are HTML elements, as given")
    {
      auto context = CreateRef<HTMLDocument>();
      auto document =
        context->Implementation().CreateDocument(infra::Namespaces::HTML, u8"html", nullptr).ReleaseValue();
      REQUIRE(document->ContentType() == u8"application/xhtml+xml");
      auto element = document->CreateElement(u8"Div").ReleaseValue();
      CHECK(element->LocalName() == u8"Div");
      CHECK(element->NamespaceURI() == infra::Namespaces::HTML);
    }

    SECTION("3. the creation options")
    {
      auto document = CreateRef<HTMLDocument>();
      auto scoped = html::CustomElementRegistry::CreateScoped();
      auto element =
        document->CreateElement(u8"div", ElementCreationOptions {.CustomElementRegistry = scoped})
          .ReleaseValue();
      CHECK(element->CustomElementRegistry() == scoped);
      auto result = document->CreateElement(
        u8"div", ElementCreationOptions {.CustomElementRegistry = scoped, .Is = u8"x-a"});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
    }
  }

  TEST_CASE("Document.createDocumentFragment, createTextNode, createComment: new nodes in the document",
            "[DOM][Document][dom:dom-document-createdocumentfragment][dom:dom-document-createtextnode]"
            "[dom:dom-document-createcomment]")
  {
    auto document = CreateRef<HTMLDocument>();

    auto fragment = document->CreateDocumentFragment();
    CHECK(&fragment->NodeDocument() == document.get());
    CHECK_FALSE(fragment->HasChildNodes());

    auto text = document->CreateTextNode(u8"text");
    CHECK(&text->NodeDocument() == document.get());
    CHECK(text->Data() == u8"text");

    auto comment = document->CreateComment(u8"comment");
    CHECK(&comment->NodeDocument() == document.get());
    CHECK(comment->Data() == u8"comment");
  }

  TEST_CASE("Document.createCDATASection(): not in an HTML document, nor with ]]> in its data",
            "[DOM][Document][dom:dom-document-createcdatasection]")
  {
    auto html = CreateRef<HTMLDocument>();
    auto inHTML = html->CreateCDATASection(u8"data");
    REQUIRE(inHTML.HasException());
    CHECK(inHTML.GetException().Code() == ExceptionCode::NotSupportedError);

    auto document = CreateRef<XMLDocument>();
    auto withEnd = document->CreateCDATASection(u8"a]]>b");
    REQUIRE(withEnd.HasException());
    CHECK(withEnd.GetException().Code() == ExceptionCode::InvalidCharacterError);

    auto section = document->CreateCDATASection(u8"a]]b").ReleaseValue();
    CHECK(section->Data() == u8"a]]b");
    CHECK(&section->NodeDocument() == document.get());
  }

  TEST_CASE("Document.createProcessingInstruction(): a valid target, and no ?> in its data",
            "[DOM][Document][dom:dom-document-createprocessinginstruction]")
  {
    auto document = CreateRef<XMLDocument>();
    auto instruction = document->CreateProcessingInstruction(u8"xml-stylesheet", u8"href=a").ReleaseValue();
    CHECK(instruction->Target() == u8"xml-stylesheet");
    CHECK(instruction->Data() == u8"href=a");
    CHECK(&instruction->NodeDocument() == document.get());

    for (auto [target, data] : {std::pair {u8"1a", u8""}, {u8"a b", u8""}, {u8"a", u8"x?>y"}})
    {
      auto result = document->CreateProcessingInstruction(target, data);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Document.importNode(): a clone in this document, deep unless asked otherwise",
            "[DOM][Document][dom:dom-document-importnode]")
  {
    auto source = CreateRef<HTMLDocument>();
    auto document = CreateRef<HTMLDocument>();
    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    auto element = source->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(element->AppendChild(*source->CreateTextNode(u8"text")).HasException());

    SECTION("1. not a document or shadow root")
    {
      CHECK(document->ImportNode(*source).GetException().Code() == ExceptionCode::NotSupportedError);
      auto shadow = element->AttachShadow({}).ReleaseValue();
      CHECK(document->ImportNode(*shadow).GetException().Code() == ExceptionCode::NotSupportedError);
    }

    SECTION("4. a boolean is whether to clone the subtree")
    {
      auto shallow = document->ImportNode(*element).ReleaseValue();
      CHECK(&shallow->NodeDocument() == document.get());
      CHECK_FALSE(shallow->HasChildNodes());
      auto deep = document->ImportNode(*element, true).ReleaseValue();
      CHECK(deep->IsEqualNode(element.get()));
      CHECK(&deep->FirstChild()->NodeDocument() == document.get());
      CHECK(element->ParentNode() == nullptr);
      CHECK(&element->NodeDocument() == source.get());
    }

    SECTION("5a. options clone the subtree unless selfOnly")
    {
      CHECK(document->ImportNode(*element, ImportNodeOptions {}).ReleaseValue()->HasChildNodes());
      CHECK_FALSE(
        document->ImportNode(*element, ImportNodeOptions {.SelfOnly = true}).ReleaseValue()->HasChildNodes());
    }

    SECTION("5b., 5c. and 6. the registry for elements without one")
    {
      auto withoutRegistry = CreateRef<TestElement>(*source);
      auto scoped = html::CustomElementRegistry::CreateScoped();
      auto withScoped =
        document->ImportNode(*withoutRegistry, ImportNodeOptions {.CustomElementRegistry = scoped})
          .ReleaseValue();
      CHECK(Downcast<Element>(*withScoped).CustomElementRegistry() == scoped);

      auto withGlobal = document->ImportNode(*withoutRegistry).ReleaseValue();
      CHECK(Downcast<Element>(*withGlobal).CustomElementRegistry() == global);

      auto otherGlobal = html::CustomElementRegistry::Create(*source);
      auto result =
        document->ImportNode(*withoutRegistry, ImportNodeOptions {.CustomElementRegistry = otherGlobal});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
    }
  }

  TEST_CASE("Document.adoptNode(): adopts the node into this document and returns it",
            "[DOM][Document][dom:dom-document-adoptnode]")
  {
    auto source = CreateRef<HTMLDocument>();
    auto document = CreateRef<HTMLDocument>();

    SECTION("1. not a document")
    {
      auto result = document->AdoptNode(*source);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
    }

    SECTION("2. not a shadow root")
    {
      auto host = source->CreateElement(u8"div").ReleaseValue();
      auto shadow = host->AttachShadow({}).ReleaseValue();
      auto result = document->AdoptNode(*shadow);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("3. and 4. adopted and returned")
    {
      auto parent = source->CreateElement(u8"div").ReleaseValue();
      auto child = source->CreateElement(u8"p").ReleaseValue();
      REQUIRE_FALSE(parent->AppendChild(*child).HasException());
      auto result = document->AdoptNode(*child).ReleaseValue();
      CHECK(result == child);
      CHECK(child->ParentNode() == nullptr);
      CHECK(&child->NodeDocument() == document.get());
    }

    SECTION("a template's contents are adopted too")
    {
      auto templateElement = source->CreateElement(u8"template").ReleaseValue();
      auto content = Downcast<html::HTMLTemplateElement>(*templateElement).Content();
      REQUIRE_FALSE(content->AppendChild(*source->CreateTextNode(u8"text")).HasException());
      auto result = document->AdoptNode(*content).ReleaseValue();
      CHECK(result == content);
      CHECK(&content->NodeDocument() == document.get());
      CHECK(&content->FirstChild()->NodeDocument() == document.get());
    }
  }

  TEST_CASE("Document.createAttribute(): a valid local name, lowercased in an HTML document",
            "[DOM][Document][dom:dom-document-createattribute]")
  {
    auto html = CreateRef<HTMLDocument>();
    auto attribute = html->CreateAttribute(u8"ID").ReleaseValue();
    CHECK(attribute->LocalName() == u8"id");
    CHECK(attribute->NamespaceURI() == DOMStringAtom::Null());
    CHECK(attribute->Prefix() == DOMStringAtom::Null());
    CHECK(attribute->Value() == u8"");
    CHECK(attribute->OwnerElement() == nullptr);
    CHECK(&attribute->NodeDocument() == html.get());

    CHECK(CreateRef<XMLDocument>()->CreateAttribute(u8"ID").ReleaseValue()->LocalName() == u8"ID");

    for (auto *name : {u8"", u8"a b", u8"a/", u8"a>"})
    {
      auto result = html->CreateAttribute(name);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Document.createAttributeNS(): the name validated and extracted",
            "[DOM][Document][dom:dom-document-createattributens]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto attribute = document->CreateAttributeNS(u8"urn:x", u8"p:Name").ReleaseValue();
    CHECK(attribute->NamespaceURI() == u8"urn:x");
    CHECK(attribute->Prefix() == u8"p");
    CHECK(attribute->LocalName() == u8"Name");

    CHECK(document->CreateAttributeNS(u8"urn:x", u8"a b").GetException().Code()
          == ExceptionCode::InvalidCharacterError);
    CHECK(document->CreateAttributeNS(DOMStringAtom::Null(), u8"p:a").GetException().Code()
          == ExceptionCode::NamespaceError);
    CHECK(document->CreateAttributeNS(u8"urn:x", u8"xmlns").GetException().Code()
          == ExceptionCode::NamespaceError);
  }

  TEST_CASE("Document.createRange(): a live range collapsed at the start of the document",
            "[DOM][Document][dom:dom-document-createrange]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto range = document->CreateRange();
    CHECK(range->StartContainer() == document.get());
    CHECK(range->StartOffset() == 0u);
    CHECK(range->EndContainer() == document.get());
    CHECK(range->EndOffset() == 0u);
    CHECK(range->Collapsed());
  }

  TEST_CASE("Document.createNodeIterator() and createTreeWalker(): rooted at root, with what to show and the "
            "filter",
            "[DOM][Document][dom:dom-document-createnodeiterator][dom:dom-document-createtreewalker]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();

    auto iterator = document->CreateNodeIterator(*root, WhatToShow::SHOW_ELEMENT);
    CHECK(&iterator->Root() == root.get());
    CHECK(&iterator->ReferenceNode() == root.get());
    CHECK(iterator->PointerBeforeReferenceNode());
    CHECK(iterator->WhatToShow() == WhatToShow::SHOW_ELEMENT);
    CHECK(iterator->Filter() == nullptr);

    auto walker = document->CreateTreeWalker(*root, WhatToShow::SHOW_TEXT);
    CHECK(&walker->Root() == root.get());
    CHECK(&walker->CurrentNode() == root.get());
    CHECK(walker->WhatToShow() == WhatToShow::SHOW_TEXT);

    CHECK(document->CreateNodeIterator(*root)->WhatToShow() == WhatToShow::SHOW_ALL);
  }
}
