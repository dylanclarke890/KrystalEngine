#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/CustomElementTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD std::string MarkupText(DOMStringView text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    // A document with a body, and a div in it.
    struct MarkupDocument
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      RefPtr<Element> Body;
      RefPtr<Element> Div;

      MarkupDocument() noexcept
      {
        auto html = Document->CreateElement(DOMStringAtom(u8"html")).ReleaseValue();
        (void)Document->AppendChild(*html);
        Body = Document->CreateElement(DOMStringAtom(u8"body")).ReleaseValue();
        (void)html->AppendChild(*Body);
        Div = Document->CreateElement(DOMStringAtom(u8"div")).ReleaseValue();
        (void)Body->AppendChild(*Div);
      }
    };
  }

  TEST_CASE("Element: innerHTML", "[DOM][Element][DOMParsing][html:the-innerhtml-property][html:dom-element-"
                                  "innerhtml][html:fragment-serializing-algorithm-steps]")
  {
    MarkupDocument data;
    auto &div = *data.Div;

    SECTION("serialises the children, escaping text and attribute values")
    {
      REQUIRE_FALSE(div.AppendChild(*data.Document->CreateTextNode(u8"a < b & c")).HasException());
      auto span = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      (void)span->SetAttribute(DOMStringAtom(u8"title"), u8"\"q\" & more");
      REQUIRE_FALSE(div.AppendChild(*span).HasException());
      CHECK(MarkupText(div.InnerHTML())
            == "a &lt; b &amp; c<span title=\"&quot;q&quot; &amp; more\"></span>");
    }

    SECTION("replaces the children with the parsed markup")
    {
      REQUIRE_FALSE(div.AppendChild(*data.Document->CreateTextNode(u8"old")).HasException());
      REQUIRE_FALSE(div.InnerHTML(u8"<p id=x>one<b>two</p>three").HasException());
      CHECK(MarkupText(div.InnerHTML()) == "<p id=\"x\">one<b>two</b></p><b>three</b>");
      CHECK(div.FirstChild()->NodeDocument() == *data.Document);
      CHECK(data.Document->GetElementById(u8"x") != nullptr);
    }

    SECTION("an empty string removes the children")
    {
      REQUIRE_FALSE(div.InnerHTML(u8"<i></i><i></i>").HasException());
      REQUIRE_FALSE(div.InnerHTML(u8"").HasException());
      CHECK_FALSE(div.HasChildNodes());
    }

    SECTION("parses in the element's context")
    {
      auto table = data.Document->CreateElement(DOMStringAtom(u8"table")).ReleaseValue();
      REQUIRE_FALSE(table->InnerHTML(u8"<tr><td>1</td></tr>").HasException());
      CHECK(MarkupText(table->InnerHTML()) == "<tbody><tr><td>1</td></tr></tbody>");

      auto textarea = data.Document->CreateElement(DOMStringAtom(u8"textarea")).ReleaseValue();
      REQUIRE_FALSE(textarea->InnerHTML(u8"<b>not bold</b>").HasException());
      REQUIRE(textarea->FirstChild());
      CHECK(Is<Text>(*textarea->FirstChild()));
    }

    SECTION("parses with the context element's attributes")
    {
      // An annotation-xml element is an HTML integration point only with an HTML encoding.
      REQUIRE_FALSE(
        div.InnerHTML(u8"<math><annotation-xml encoding=text/html></annotation-xml></math>").HasException());
      auto annotation = div.QuerySelector(u8"annotation-xml").ReleaseValue();
      REQUIRE(annotation);
      REQUIRE_FALSE(annotation->InnerHTML(u8"<div></div>").HasException());
      auto *child = DynamicDowncast<Element>(annotation->FirstChild());
      REQUIRE(child);
      CHECK(child->NamespaceURI() == infra::Namespaces::HTML);
    }

    SECTION("goes into a template's content")
    {
      auto templateElement = data.Document->CreateElement(DOMStringAtom(u8"template")).ReleaseValue();
      REQUIRE_FALSE(templateElement->InnerHTML(u8"<span>in</span>").HasException());
      CHECK_FALSE(templateElement->HasChildNodes());
      auto content = Downcast<html::HTMLTemplateElement>(*templateElement).Content();
      REQUIRE(content->FirstChild());
      CHECK(MarkupText(templateElement->InnerHTML()) == "<span>in</span>");
    }

    SECTION("doesn't attach declarative shadow roots")
    {
      REQUIRE_FALSE(
        div.InnerHTML(u8"<p><template shadowrootmode=open><b></b></template></p>").HasException());
      auto p = div.FirstElementChild();
      REQUIRE(p);
      CHECK_FALSE(p->ShadowRoot());
      CHECK(p->FirstElementChild());
    }

    SECTION("queues a single mutation record")
    {
      REQUIRE_FALSE(div.InnerHTML(u8"<i></i><i></i>").HasException());
      auto observer =
        CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
      REQUIRE_FALSE(observer->Observe(div, {.ChildList = true}).HasException());
      REQUIRE_FALSE(div.InnerHTML(u8"<b></b><b></b><b></b>").HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->RemovedNodes()->Length() == 2);
      CHECK(records[0]->AddedNodes()->Length() == 3);
    }
  }

  TEST_CASE("Element: innerHTML in an XML document",
            "[DOM][Element][DOMParsing][html:xml-fragment-parsing-algorithm]")
  {
    auto document = CreateRef<XMLDocument>();
    auto root = document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"div"))
                  .ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());

    CHECK_FALSE(root->InnerHTML(u8"<p>well formed</p>").HasException());
    auto p = root->FirstElementChild();
    REQUIRE(p);
    CHECK(p->LocalName() == u8"p");
    CHECK(p->NamespaceURI() == infra::Namespaces::HTML);

    auto result = root->InnerHTML(u8"<p>not closed");
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::SyntaxError);
    CHECK(root->FirstElementChild() == p);
  }

  TEST_CASE("The XML fragment parsing algorithm declares the namespaces in scope on the context",
            "[DOM][Element][DOMParsing][html:xml-fragment-parsing-algorithm]")
  {
    auto document = CreateRef<XMLDocument>();
    auto root =
      document->CreateElementNS(DOMStringAtom(u8"urn:root"), DOMStringAtom(u8"root")).ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());
    REQUIRE_FALSE(
      root->SetAttributeNS(DOMStringAtom(infra::Namespaces::XMLNS), DOMStringAtom(u8"xmlns:p"), u8"urn:outer")
        .HasException());
    auto context =
      document->CreateElementNS(DOMStringAtom(u8"urn:root"), DOMStringAtom(u8"context")).ReleaseValue();
    REQUIRE_FALSE(root->AppendChild(*context).HasException());

    SECTION("the default namespace and an ancestor's prefix")
    {
      REQUIRE_FALSE(context->InnerHTML(u8"<a/><p:b/>").HasException());
      auto a = context->FirstElementChild();
      REQUIRE(a);
      CHECK(a->NamespaceURI() == u8"urn:root");
      auto b = a->NextElementSibling();
      REQUIRE(b);
      CHECK(b->NamespaceURI() == u8"urn:outer");
    }

    SECTION("a prefix declared nearer the context wins")
    {
      REQUIRE_FALSE(
        context
          ->SetAttributeNS(DOMStringAtom(infra::Namespaces::XMLNS), DOMStringAtom(u8"xmlns:p"), u8"urn:inner")
          .HasException());
      REQUIRE_FALSE(context->InnerHTML(u8"<p:b/>").HasException());
      auto b = context->FirstElementChild();
      REQUIRE(b);
      CHECK(b->NamespaceURI() == u8"urn:inner");
    }
  }

  TEST_CASE(
    "The XML fragment serialization algorithm",
    "[DOM][Element][DOMParsing][html:xml-fragment-serialisation-algorithm][html:relevant-child-nodes]")
  {
    auto document = CreateRef<XMLDocument>();
    auto root =
      document->CreateElementNS(DOMStringAtom(u8"urn:root"), DOMStringAtom(u8"root")).ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());

    SECTION("declares the empty default namespace for elements in no namespace")
    {
      auto child = document->CreateElementNS(DOMStringAtom::Null(), DOMStringAtom(u8"c")).ReleaseValue();
      REQUIRE_FALSE(root->AppendChild(*child).HasException());
      CHECK(MarkupText(root->InnerHTML()) == "<c xmlns=\"\"/>");
    }

    SECTION("serialises a template's contents")
    {
      auto templateElement =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"template"))
          .ReleaseValue();
      REQUIRE_FALSE(root->AppendChild(*templateElement).HasException());
      REQUIRE_FALSE(templateElement->InnerHTML(u8"<b/>").HasException());
      CHECK(templateElement->FirstChild() == nullptr);
      CHECK(MarkupText(templateElement->InnerHTML()) == "<b xmlns=\"http://www.w3.org/1999/xhtml\"></b>");
    }
  }

  TEST_CASE("The XML fragment serialization algorithm refuses a DOM that isn't well-formed",
            "[DOM][Element][DOMParsing][html:xml-fragment-serialisation-algorithm][!shouldfail]")
  {
    // FIXME: Serializing XML doesn't check for the error cases, so it writes markup that isn't well-formed
    // where it should throw an InvalidStateError. See KNOWN-ISSUES.md.
    auto document = CreateRef<XMLDocument>();
    auto root =
      document->CreateElementNS(DOMStringAtom(u8"urn:root"), DOMStringAtom(u8"root")).ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());
    REQUIRE_FALSE(root->AppendChild(*document->CreateComment(u8"a--b")).HasException());
    CHECK(MarkupText(root->InnerHTML()).find("a--b") == std::string::npos);
  }

  TEST_CASE("The XML fragment parsing algorithm parses a script's or style's markup like any other",
            "[DOM][Element][DOMParsing][html:xml-fragment-parsing-algorithm]")
  {
    auto document = CreateRef<XMLDocument>();
    auto root = document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"div"))
                  .ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());

    for (auto name : {u8"script", u8"style"})
    {
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(name)).ReleaseValue();
      REQUIRE_FALSE(root->AppendChild(*element).HasException());

      REQUIRE_FALSE(element->InnerHTML(u8"<b/>").HasException());
      auto b = element->FirstElementChild();
      REQUIRE(b);
      CHECK(b->LocalName() == u8"b");

      auto result = element->InnerHTML(u8"a < b");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::SyntaxError);
    }
  }

  TEST_CASE("Void elements are serialised without end tags or children",
            "[DOM][Element][DOMParsing][html:elements-2][html:void-elements][html:serializes-as-void]")
  {
    MarkupDocument data;
    auto &div = *data.Div;

    REQUIRE_FALSE(div.InnerHTML(u8"<area><base><br><embed><hr><img><input><link><meta><source><track><wbr>")
                    .HasException());
    CHECK(MarkupText(div.InnerHTML())
          == "<area><base><br><embed><hr><img><input><link><meta><source><track><wbr>");

    // Elements made by the DOM serialise as void even with children: col (which the parser only keeps in a
    // table) and the obsolete elements that serialise as void.
    for (auto name : {u8"col", u8"basefont", u8"bgsound", u8"frame", u8"keygen", u8"param"})
    {
      auto element = data.Document->CreateElement(DOMStringAtom(name)).ReleaseValue();
      REQUIRE_FALSE(element->AppendChild(*data.Document->CreateTextNode(u8"text")).HasException());
      CHECK(MarkupText(element->OuterHTML()) == "<" + MarkupText(name) + ">");
    }
  }

  TEST_CASE("The HTML fragment serialization algorithm",
            "[DOM][Element][DOMParsing][html:serialising-html-fragments][html:html-fragment-serialisation-"
            "algorithm][html:escapingString][html:attribute's-serialised-name]")
  {
    MarkupDocument data;
    auto &document = *data.Document;
    auto &div = *data.Div;

    auto make = [&](const char8_t *name)
    {
      return document.CreateElement(DOMStringAtom(name)).ReleaseValue();
    };
    auto makeNS = [&](DOMStringView namespaceURI, const char8_t *name)
    {
      return document.CreateElementNS(DOMStringAtom(namespaceURI), DOMStringAtom(name)).ReleaseValue();
    };

    SECTION("a void element serialises its children as the empty string")
    {
      auto br = make(u8"br");
      REQUIRE_FALSE(br->AppendChild(*document.CreateTextNode(u8"text")).HasException());
      CHECK(MarkupText(br->InnerHTML()).empty());
    }

    SECTION("escapes text and attribute values")
    {
      auto span = make(u8"span");
      REQUIRE_FALSE(span->SetAttribute(DOMStringAtom(u8"title"), u8"&\u00A0\"<>").HasException());
      REQUIRE_FALSE(span->AppendChild(*document.CreateTextNode(u8"&\u00A0\"<>")).HasException());
      REQUIRE_FALSE(div.AppendChild(*span).HasException());
      CHECK(MarkupText(div.InnerHTML())
            == "<span title=\"&amp;&nbsp;&quot;&lt;&gt;\">&amp;&nbsp;\"&lt;&gt;</span>");
    }

    SECTION("leaves the text of raw text elements, and of noscript without scripting, as it is")
    {
      for (auto name : {u8"style", u8"script", u8"xmp", u8"iframe", u8"noembed", u8"noframes", u8"plaintext"})
      {
        auto element = make(name);
        REQUIRE_FALSE(element->AppendChild(*document.CreateTextNode(u8"<&>")).HasException());
        CHECK(MarkupText(element->InnerHTML()) == "<&>");
      }

      auto noscript = make(u8"noscript");
      REQUIRE_FALSE(noscript->AppendChild(*document.CreateTextNode(u8"<&>")).HasException());
      CHECK(MarkupText(noscript->InnerHTML()) == "&lt;&amp;&gt;");
    }

    SECTION("serialises attributes with their serialised names")
    {
      auto span = make(u8"span");
      REQUIRE_FALSE(
        span->SetAttributeNS(DOMStringAtom(infra::Namespaces::XML), DOMStringAtom(u8"a:lang"), u8"1")
          .HasException());
      REQUIRE_FALSE(
        span->SetAttributeNS(DOMStringAtom(infra::Namespaces::XLink), DOMStringAtom(u8"b:href"), u8"2")
          .HasException());
      REQUIRE_FALSE(
        span->SetAttributeNS(DOMStringAtom(infra::Namespaces::XMLNS), DOMStringAtom(u8"xmlns"), u8"3")
          .HasException());
      REQUIRE_FALSE(
        span->SetAttributeNS(DOMStringAtom(infra::Namespaces::XMLNS), DOMStringAtom(u8"xmlns:c"), u8"4")
          .HasException());
      REQUIRE_FALSE(
        span->SetAttributeNS(DOMStringAtom(u8"urn:x"), DOMStringAtom(u8"d:e"), u8"5").HasException());
      CHECK(MarkupText(span->OuterHTML())
            == "<span xml:lang=\"1\" xlink:href=\"2\" xmlns=\"3\" xmlns:c=\"4\" d:e=\"5\"></span>");
    }

    SECTION("uses the local name of HTML, SVG and MathML elements, and the qualified name of others")
    {
      CHECK(MarkupText(makeNS(infra::Namespaces::HTML, u8"a:div")->OuterHTML()) == "<div></div>");
      CHECK(MarkupText(makeNS(infra::Namespaces::SVG, u8"b:svg")->OuterHTML()) == "<svg></svg>");
      CHECK(MarkupText(makeNS(infra::Namespaces::MathML, u8"c:math")->OuterHTML()) == "<math></math>");
      CHECK(MarkupText(makeNS(u8"urn:x", u8"d:e")->OuterHTML()) == "<d:e></d:e>");
    }

    SECTION("adds an is value that isn't in the attributes")
    {
      auto withIs =
        document.CreateElement(DOMStringAtom(u8"p"), ElementCreationOptions {.Is = DOMStringAtom(u8"x-a")})
          .ReleaseValue();
      CHECK(MarkupText(withIs->OuterHTML()) == "<p is=\"x-a\"></p>");

      REQUIRE_FALSE(withIs->SetAttribute(DOMStringAtom(u8"is"), u8"x-b").HasException());
      CHECK(MarkupText(withIs->OuterHTML()) == "<p is=\"x-b\"></p>");
    }

    SECTION("serialises comments and processing instructions")
    {
      REQUIRE_FALSE(div.AppendChild(*document.CreateComment(u8"c")).HasException());
      REQUIRE_FALSE(
        div.AppendChild(*document.CreateProcessingInstruction(u8"t", u8"d").ReleaseValue()).HasException());
      CHECK(MarkupText(div.InnerHTML()) == "<!--c--><?t d>");
    }

    SECTION("serialises a shadow root's options, in order")
    {
      auto host = make(u8"p");
      auto shadow = host
                      ->AttachShadow({.Mode = ShadowRootMode::Closed,
                                      .DelegatesFocus = DelegatesFocus(true),
                                      .SlotAssignment = SlotAssignmentMode::Manual,
                                      .Clonable = Clonable(true),
                                      .Serialisable = Serialisable(true)})
                      .ReleaseValue();
      CHECK(
        MarkupText(host->GetHTML({.SerializableShadowRoots = true}))
        == "<template shadowrootmode=\"closed\" shadowrootdelegatesfocus=\"\" shadowrootserializable=\"\" "
           "shadowrootslotassignment=\"manual\" shadowrootclonable=\"\"></template>");
    }
  }

  TEST_CASE("Element: outerHTML",
            "[DOM][Element][DOMParsing][html:the-outerhtml-property][html:dom-element-outerhtml]")
  {
    MarkupDocument data;
    auto &div = *data.Div;

    SECTION("serialises the element")
    {
      (void)div.SetAttribute(DOMStringAtom(u8"id"), u8"d");
      REQUIRE_FALSE(div.InnerHTML(u8"<br>").HasException());
      CHECK(MarkupText(div.OuterHTML()) == "<div id=\"d\"><br></div>");
    }

    SECTION("replaces the element, parsed in its parent's context")
    {
      REQUIRE_FALSE(div.OuterHTML(u8"<p>1</p><p>2</p>").HasException());
      CHECK_FALSE(div.ParentNode());
      CHECK(MarkupText(data.Body->InnerHTML()) == "<p>1</p><p>2</p>");
    }

    SECTION("merges the text nodes it leaves next to each other")
    {
      REQUIRE_FALSE(data.Body->InnerHTML(u8"a<span></span>d").HasException());
      auto span = data.Body->FirstElementChild();
      REQUIRE_FALSE(span->OuterHTML(u8"b c").HasException());
      REQUIRE(data.Body->FirstChild());
      CHECK(data.Body->FirstChild() == data.Body->LastChild());
      CHECK(MarkupText(data.Body->InnerHTML()) == "ab cd");
    }

    SECTION("does nothing without a parent, and throws when the parent is the document")
    {
      auto orphan = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      CHECK_FALSE(orphan->OuterHTML(u8"<b></b>").HasException());

      auto result = data.Document->DocumentElement()->OuterHTML(u8"<html></html>");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NoModificationAllowedError);
    }

    SECTION("throws when the parent is a document fragment")
    {
      auto fragment = data.Document->CreateDocumentFragment();
      auto span = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      REQUIRE_FALSE(fragment->AppendChild(*span).HasException());
      auto result = span->OuterHTML(u8"<i></i>");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NoModificationAllowedError);
      CHECK(fragment->FirstChild() == span.get());
    }
  }

  TEST_CASE(
    "Element: insertAdjacentHTML",
    "[DOM][Element][DOMParsing][html:the-insertadjacenthtml()-method][html:dom-element-insertadjacenthtml]")
  {
    MarkupDocument data;
    auto &div = *data.Div;
    REQUIRE_FALSE(div.InnerHTML(u8"<i></i>").HasException());

    SECTION("inserts at each position")
    {
      REQUIRE_FALSE(div.InsertAdjacentHTML(InsertAdjacentWhere::BeforeBegin, u8"<a></a>").HasException());
      REQUIRE_FALSE(div.InsertAdjacentHTML(InsertAdjacentWhere::AfterBegin, u8"<b></b>").HasException());
      REQUIRE_FALSE(div.InsertAdjacentHTML(InsertAdjacentWhere::BeforeEnd, u8"<u></u>").HasException());
      REQUIRE_FALSE(div.InsertAdjacentHTML(InsertAdjacentWhere::AfterEnd, u8"<s></s>").HasException());
      CHECK(MarkupText(data.Body->InnerHTML()) == "<a></a><div><b></b><i></i><u></u></div><s></s>");
    }

    SECTION("parses in the parent's context before and after the element")
    {
      auto table = data.Document->CreateElement(DOMStringAtom(u8"table")).ReleaseValue();
      REQUIRE_FALSE(div.AppendChild(*table).HasException());
      REQUIRE_FALSE(
        table->InsertAdjacentHTML(InsertAdjacentWhere::AfterBegin, u8"<td>1</td>").HasException());
      CHECK(MarkupText(table->InnerHTML()) == "<tbody><tr><td>1</td></tr></tbody>");

      // Before the table, the context is the div, where a td start tag is ignored.
      REQUIRE_FALSE(
        table->InsertAdjacentHTML(InsertAdjacentWhere::BeforeBegin, u8"<td>2</td>").HasException());
      CHECK(MarkupText(div.InnerHTML()).starts_with("<i></i>2<table>"));
    }

    SECTION("parses as in a body next to the document element")
    {
      auto html = data.Document->DocumentElement();
      REQUIRE_FALSE(
        data.Body->InsertAdjacentHTML(InsertAdjacentWhere::BeforeBegin, u8"<p></p>").HasException());
      auto first = html->FirstElementChild();
      REQUIRE(first);
      CHECK(first->LocalName() == u8"p");
    }

    SECTION("throws without a parent element to insert next to")
    {
      auto html = data.Document->DocumentElement();
      auto result = html->InsertAdjacentHTML(InsertAdjacentWhere::AfterEnd, u8"<p></p>");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NoModificationAllowedError);

      auto orphan = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      result = orphan->InsertAdjacentHTML(InsertAdjacentWhere::BeforeBegin, u8"<p></p>");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NoModificationAllowedError);
      CHECK_FALSE(orphan->InsertAdjacentHTML(InsertAdjacentWhere::BeforeEnd, u8"<p></p>").HasException());
      CHECK(orphan->FirstElementChild());
    }

    SECTION("insertAdjacentElement returns null when there's nowhere to insert")
    {
      auto orphan = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      auto other = data.Document->CreateElement(DOMStringAtom(u8"b")).ReleaseValue();
      auto result = orphan->InsertAdjacentElement(InsertAdjacentWhere::AfterEnd, *other);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == nullptr);
      result = orphan->InsertAdjacentElement(InsertAdjacentWhere::BeforeEnd, *other);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == other.get());
    }
  }

  TEST_CASE("Element and ShadowRoot: setHTMLUnsafe and getHTML",
            "[DOM][Element][ShadowRoot][DOMParsing][html:html-serialization-methods][html:dom-element-"
            "gethtml][html:dom-shadowroot-gethtml][html:dom-shadowroot-innerhtml]")
  {
    MarkupDocument data;
    auto &div = *data.Div;
    REQUIRE_FALSE(div.InnerHTML(u8"<p></p>").HasException());
    auto p = div.FirstElementChild();

    SECTION("getHTML serialises serialisable shadow roots when asked to")
    {
      auto shadow = p->AttachShadow({.Serialisable = Serialisable(true)}).ReleaseValue();
      REQUIRE_FALSE(shadow->InnerHTML(u8"<b></b>").HasException());
      CHECK(MarkupText(div.InnerHTML()) == "<p></p>");
      CHECK(MarkupText(div.GetHTML()) == "<p></p>");
      CHECK(MarkupText(div.GetHTML({.SerializableShadowRoots = true}))
            == "<p><template shadowrootmode=\"open\" shadowrootserializable=\"\"><b></b></template></p>");
    }

    SECTION("getHTML serialises the shadow roots it's given")
    {
      auto shadow = p->AttachShadow({}).ReleaseValue();
      REQUIRE_FALSE(shadow->InnerHTML(u8"<i>x</i>").HasException());
      CHECK(MarkupText(div.GetHTML({.SerializableShadowRoots = true})) == "<p></p>");
      CHECK(MarkupText(div.GetHTML({.ShadowRoots = {shadow}}))
            == "<p><template shadowrootmode=\"open\"><i>x</i></template></p>");
    }

    SECTION("a shadow root's markup replaces its children")
    {
      auto shadow = p->AttachShadow({}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTMLUnsafe(u8"<b>1</b>").HasException());
      CHECK(MarkupText(shadow->GetHTML()) == "<b>1</b>");
      CHECK(MarkupText(shadow->InnerHTML()) == "<b>1</b>");
      REQUIRE_FALSE(shadow->InnerHTML(u8"").HasException());
      CHECK_FALSE(shadow->HasChildNodes());
    }

    SECTION("setHTMLUnsafe replaces the element's children")
    {
      REQUIRE_FALSE(div.SetHTMLUnsafe(u8"<span>1</span>").HasException());
      CHECK(MarkupText(div.InnerHTML()) == "<span>1</span>");
    }
  }

  TEST_CASE("Document: parseHTMLUnsafe", "[DOM][Document][DOMParsing]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto result = Document::ParseHTMLUnsafe(*context, u8"<title>T</title><div id=d>text</div>");
    REQUIRE_FALSE(result.HasException());
    auto &document = *result.Value();
    CHECK(Is<HTMLDocument>(document));
    CHECK(&document.GetSettings() == &context->GetSettings());
    REQUIRE(document.Body());
    CHECK(document.Title() == u8"T");
    CHECK(document.GetElementById(u8"d") != nullptr);
  }

  TEST_CASE("setHTMLUnsafe and parseHTMLUnsafe attach declarative shadow roots",
            "[DOM][Element][Document][DOMParsing]")
  {
    MarkupDocument data;
    auto &div = *data.Div;
    REQUIRE_FALSE(
      div.SetHTMLUnsafe(u8"<p><template shadowrootmode=open><b></b></template></p>").HasException());
    auto p = div.FirstElementChild();
    REQUIRE(p);
    auto shadow = p->ShadowRoot();
    REQUIRE(shadow);
    CHECK(MarkupText(shadow->InnerHTML()) == "<b></b>");
    CHECK_FALSE(p->HasChildNodes());

    auto result = Document::ParseHTMLUnsafe(
      *data.Document, u8"<div id=d><template shadowrootmode=closed><b></b></template></div>");
    REQUIRE_FALSE(result.HasException());
    auto host = result.Value()->GetElementById(u8"d");
    REQUIRE(host);
    CHECK_FALSE(host->HasChildNodes());
    auto *closedShadow = TreeQueries::GetShadowRoot(*host);
    REQUIRE(closedShadow);
    CHECK(closedShadow->Mode() == ShadowRootMode::Closed);
  }

  TEST_CASE("Declarative shadow roots", "[DOM][Element][DOMParsing]")
  {
    MarkupDocument data;
    auto &div = *data.Div;

    SECTION("take their options from the template")
    {
      REQUIRE_FALSE(div
                      .SetHTMLUnsafe(u8"<p><template shadowrootmode=OPEN shadowrootdelegatesfocus "
                                     u8"shadowrootclonable shadowrootserializable "
                                     u8"shadowrootslotassignment=manual></template></p>")
                      .HasException());
      auto shadow = div.FirstElementChild()->ShadowRoot();
      REQUIRE(shadow);
      CHECK(shadow->Mode() == ShadowRootMode::Open);
      CHECK(shadow->DelegatesFocus());
      CHECK(shadow->Clonable());
      CHECK(shadow->Serialisable());
      CHECK(shadow->SlotAssignment() == SlotAssignmentMode::Manual);
      CHECK(shadow->IsAvailableToElementInternals());
    }

    SECTION("a host's second template stays a template")
    {
      REQUIRE_FALSE(div
                      .SetHTMLUnsafe(u8"<p><template shadowrootmode=open><b></b></template>"
                                     u8"<template shadowrootmode=open><i></i></template></p>")
                      .HasException());
      auto p = div.FirstElementChild();
      REQUIRE(p->ShadowRoot());
      CHECK(MarkupText(p->ShadowRoot()->InnerHTML()) == "<b></b>");
      auto *second = DynamicDowncast<html::HTMLTemplateElement>(p->FirstElementChild().get());
      REQUIRE(second);
      CHECK(MarkupText(second->InnerHTML()) == "<i></i>");
    }

    SECTION("an invalid mode leaves a template")
    {
      REQUIRE_FALSE(
        div.SetHTMLUnsafe(u8"<p><template shadowrootmode=none><b></b></template></p>").HasException());
      auto p = div.FirstElementChild();
      CHECK_FALSE(p->ShadowRoot());
      CHECK(Is<html::HTMLTemplateElement>(p->FirstElementChild().get()));
    }

    SECTION("a top-level template doesn't attach to the context element")
    {
      REQUIRE_FALSE(div.SetHTMLUnsafe(u8"<template shadowrootmode=open><b></b></template>").HasException());
      CHECK_FALSE(div.ShadowRoot());
      CHECK(Is<html::HTMLTemplateElement>(div.FirstElementChild().get()));
    }

    SECTION("nest")
    {
      REQUIRE_FALSE(div
                      .SetHTMLUnsafe(u8"<p><template shadowrootmode=open><span><template "
                                     u8"shadowrootmode=open><b></b></template></span></template></p>")
                      .HasException());
      auto outer = div.FirstElementChild()->ShadowRoot();
      REQUIRE(outer);
      auto span = outer->FirstElementChild();
      REQUIRE(span);
      REQUIRE(span->ShadowRoot());
      CHECK(MarkupText(span->ShadowRoot()->InnerHTML()) == "<b></b>");
    }

    SECTION("attach in a document's head and body")
    {
      auto result = Document::ParseHTMLUnsafe(
        *data.Document, u8"<body><div id=d><template shadowrootmode=open>text</template></div>");
      REQUIRE_FALSE(result.HasException());
      auto host = result.Value()->GetElementById(u8"d");
      REQUIRE(host);
      REQUIRE(host->ShadowRoot());
      CHECK(MarkupText(host->ShadowRoot()->InnerHTML()) == "text");
    }
  }
}

namespace krys::boo::html::tests
{
  TEST_CASE_METHOD(CustomElementFixture, "innerHTML runs custom element reactions before it returns",
                   "[DOM][Element][DOMParsing][CustomElement]")
  {
    Define(u8"x-widget", {u8"value"});
    REQUIRE_FALSE(Body().InnerHTML(u8"<x-widget id=a value=1></x-widget>").HasException());
    CHECK(TakeCalls() == Log {u8"constructed", u8"attribute value null 1", u8"connected a"});

    REQUIRE_FALSE(Body().InnerHTML(u8"").HasException());
    CHECK(TakeCalls() == Log {u8"disconnected a"});
  }

  TEST_CASE_METHOD(CustomElementFixture, "setHTML doesn't construct custom elements it removes",
                   "[DOM][Element][DOMParsing][CustomElement][Sanitizer]")
  {
    Define(u8"x-widget", {u8"value"});
    REQUIRE_FALSE(Body().SetHTML(u8"<x-widget id=a value=1></x-widget>").HasException());
    CHECK_FALSE(Body().HasChildNodes());
    CHECK(TakeCalls().empty());

    html::SanitizerConfig config;
    config.Elements = List<html::SanitizerElementNamespaceWithAttributes> {u8"x-widget"};
    REQUIRE_FALSE(Body()
                    .SetHTML(u8"<x-widget id=a value=1></x-widget>", {.Sanitizer = krys::move(config)})
                    .HasException());
    CHECK(TakeCalls() == Log {u8"constructed", u8"attribute value null 1", u8"connected a"});
  }
}
