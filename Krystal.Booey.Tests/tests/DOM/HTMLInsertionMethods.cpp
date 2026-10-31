#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/Sanitizer/Sanitizer.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD std::string InsertedMarkup(DOMStringView text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    // A document with a body, and a div in it.
    struct InsertionDocument
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      RefPtr<Element> Body;
      RefPtr<Element> Div;

      InsertionDocument() noexcept
      {
        auto html = Document->CreateElement(DOMStringAtom(u8"html")).ReleaseValue();
        (void)Document->AppendChild(*html);
        Body = Document->CreateElement(DOMStringAtom(u8"body")).ReleaseValue();
        (void)html->AppendChild(*Body);
        Div = Document->CreateElement(DOMStringAtom(u8"div")).ReleaseValue();
        (void)Body->AppendChild(*Div);
      }

      KRYS_NODISCARD std::string Markup() const noexcept
      {
        return InsertedMarkup(Div->InnerHTML());
      }
    };

    KRYS_NODISCARD html::SetHTMLOptions WithConfig(html::SanitizerConfig &&config) noexcept
    {
      return {.Sanitizer = krys::move(config)};
    }

    KRYS_NODISCARD html::SetHTMLUnsafeOptions UnsafeWithConfig(html::SanitizerConfig &&config) noexcept
    {
      return {.Sanitizer = html::SanitizerInput {krys::move(config)}};
    }
  }

  TEST_CASE("setHTML sanitizes with the default configuration", "[DOM][Element][DOMParsing][Sanitizer]")
  {
    InsertionDocument data;
    auto &div = *data.Div;

    SECTION("removes unsafe elements and their contents")
    {
      REQUIRE_FALSE(div.SetHTML(u8"a<script>alert(1)</script>b<iframe></iframe>c").HasException());
      CHECK(data.Markup() == "abc");
      // The text around removed elements coalesces into one Text node.
      REQUIRE(div.FirstChild());
      CHECK(div.FirstChild() == div.LastChild());
    }

    SECTION("removes elements outside the default configuration")
    {
      REQUIRE_FALSE(div.SetHTML(u8"<p>x<img src=a.png>y</p><custom-thing>z</custom-thing>").HasException());
      CHECK(data.Markup() == "<p>xy</p>");
    }

    SECTION("removes attributes, comments and javascript: URLs")
    {
      REQUIRE_FALSE(div
                      .SetHTML(u8"<a href='javascript:alert(1)' onclick=x id=i lang=en>t</a><!--c-->"
                               u8"<a href='https://example.com'>u</a>")
                      .HasException());
      CHECK(data.Markup() == "<a lang=\"en\">t</a><a href=\"https://example.com\">u</a>");
    }

    SECTION("keeps SVG and MathML in the default configuration")
    {
      REQUIRE_FALSE(div
                      .SetHTML(u8"<svg viewBox='0 0 1 1'><circle r=1 onload=x></circle><use href=#a></use>"
                               u8"</svg><math><mi>x</mi></math>")
                      .HasException());
      CHECK(data.Markup()
            == "<svg viewBox=\"0 0 1 1\"><circle r=\"1\"></circle></svg><math><mi>x</mi></math>");
    }

    SECTION("does nothing on a script element")
    {
      auto script = data.Document->CreateElement(DOMStringAtom(u8"script")).ReleaseValue();
      REQUIRE_FALSE(script->AppendChild(*data.Document->CreateTextNode(u8"keep")).HasException());
      REQUIRE_FALSE(script->SetHTML(u8"<b>x</b>").HasException());
      CHECK(InsertedMarkup(script->InnerHTML()) == "keep");
    }

    SECTION("goes into a template's contents")
    {
      auto templateElement = data.Document->CreateElement(DOMStringAtom(u8"template")).ReleaseValue();
      REQUIRE_FALSE(templateElement->SetHTML(u8"<b>x</b><script></script>").HasException());
      CHECK_FALSE(templateElement->HasChildNodes());
      CHECK(InsertedMarkup(templateElement->InnerHTML()) == "<b>x</b>");
    }

    SECTION("an invalid configuration throws a TypeError")
    {
      html::SanitizerConfig config;
      config.Elements = List<html::SanitizerElementNamespaceWithAttributes> {u8"p"};
      config.RemoveElements = List<html::SanitizerElementNamespace> {u8"b"};
      auto result = div.SetHTML(u8"<p></p>", WithConfig(krys::move(config)));
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::TypeError);
    }
  }

  TEST_CASE("setHTML with a configuration", "[DOM][Element][DOMParsing][Sanitizer]")
  {
    InsertionDocument data;
    auto &div = *data.Div;

    SECTION("replaces elements with their children")
    {
      REQUIRE_FALSE(
        div
          .SetHTML(u8"a<b>b<i>c</i>d</b>e",
                   WithConfig({.ReplaceWithChildrenElements = List<html::SanitizerElementNamespace> {u8"b"}}))
          .HasException());
      CHECK(data.Markup() == "ab<i>c</i>de");
    }

    SECTION("a replaced element's children go where it would have been, around a table")
    {
      REQUIRE_FALSE(div
                      .SetHTML(u8"<table><tr><td><span>x</span></td></tr></table>",
                               WithConfig({.ReplaceWithChildrenElements =
                                             List<html::SanitizerElementNamespace> {u8"tbody", u8"tr"}}))
                      .HasException());
      CHECK(data.Markup() == "<table><td><span>x</span></td></table>");
    }

    SECTION("safe parsing still removes unsafe content a configuration allows")
    {
      REQUIRE_FALSE(div.SetHTML(u8"<script>x</script><b onclick=y>z</b>", WithConfig({})).HasException());
      CHECK(data.Markup() == "<b>z</b>");
    }

    SECTION("an allow-list with local attributes and data attributes")
    {
      html::SanitizerConfig config;
      config.Elements = List<html::SanitizerElementNamespaceWithAttributes> {u8"p", u8"span"};
      config.Elements->back().Attributes = List<html::SanitizerAttributeNamespace> {u8"title"};
      config.Attributes = List<html::SanitizerAttributeNamespace> {u8"id"};
      config.DataAttributes = true;
      config.Comments = true;
      REQUIRE_FALSE(div
                      .SetHTML(u8"<p id=a title=t data-x=1>1</p><span id=b title=u class=c>2</span><!--k-->"
                               u8"<em>3</em>",
                               WithConfig(krys::move(config)))
                      .HasException());
      CHECK(data.Markup() == "<p id=\"a\" data-x=\"1\">1</p><span id=\"b\" title=\"u\">2</span><!--k-->");
    }

    SECTION("misnested formatting elements the adoption agency rebuilds")
    {
      REQUIRE_FALSE(
        div
          .SetHTML(u8"<b>1<p>2</b>3</p>",
                   WithConfig({.ReplaceWithChildrenElements = List<html::SanitizerElementNamespace> {u8"b"}}))
          .HasException());
      CHECK(data.Markup() == "1<p>23</p>");

      REQUIRE_FALSE(div
                      .SetHTML(u8"<b>1<p>2</b>3</p>",
                               WithConfig({.RemoveElements = List<html::SanitizerElementNamespace> {u8"b"}}))
                      .HasException());
      // The b reconstructed in p is removed too, but the adoption agency's next pass pops it before the 3.
      CHECK(data.Markup() == "<p>3</p>");
    }

    SECTION("a removed element's declarative shadow root is dropped")
    {
      REQUIRE_FALSE(
        div
          .SetHTMLUnsafe(u8"<p><template shadowrootmode=open><b>s</b></template>t</p>",
                         UnsafeWithConfig({.RemoveElements = List<html::SanitizerElementNamespace> {u8"p"}}))
          .HasException());
      CHECK(data.Markup().empty());
    }

    SECTION("a replaced host keeps its template as a template")
    {
      REQUIRE_FALSE(div
                      .SetHTMLUnsafe(u8"<p><template shadowrootmode=open><b>s</b></template>t</p>",
                                     UnsafeWithConfig({.ReplaceWithChildrenElements =
                                                         List<html::SanitizerElementNamespace> {u8"p"}}))
                      .HasException());
      auto *templateElement = DynamicDowncast<html::HTMLTemplateElement>(div.FirstChild());
      REQUIRE(templateElement);
      CHECK(InsertedMarkup(templateElement->InnerHTML()) == "<b>s</b>");
    }

    SECTION("the default configuration removes declarative shadow roots, as it removes templates")
    {
      REQUIRE_FALSE(div.SetHTML(u8"<p><template shadowrootmode=open><b>s</b></template></p>").HasException());
      CHECK_FALSE(TreeQueries::GetShadowRoot(*div.FirstElementChild()));
      CHECK(data.Markup() == "<p></p>");
    }

    SECTION("declarative shadow roots are sanitized")
    {
      html::SanitizerConfig config;
      config.Elements = List<html::SanitizerElementNamespaceWithAttributes> {u8"p", u8"b", u8"template"};
      config.Elements->back().Attributes = List<html::SanitizerAttributeNamespace> {u8"shadowrootmode"};
      REQUIRE_FALSE(div
                      .SetHTML(u8"<p><template shadowrootmode=open><b>s</b><script></script></template></p>",
                               WithConfig(krys::move(config)))
                      .HasException());
      auto *shadow = TreeQueries::GetShadowRoot(*div.FirstElementChild());
      REQUIRE(shadow);
      CHECK(InsertedMarkup(shadow->InnerHTML()) == "<b>s</b>");
    }
  }

  TEST_CASE("setHTMLUnsafe keeps everything unless given a sanitizer",
            "[DOM][Element][DOMParsing][Sanitizer]")
  {
    InsertionDocument data;
    auto &div = *data.Div;
    REQUIRE_FALSE(div.SetHTMLUnsafe(u8"<b onclick=x>1</b><!--c-->").HasException());
    CHECK(data.Markup() == "<b onclick=\"x\">1</b><!--c-->");

    REQUIRE_FALSE(
      div
        .SetHTMLUnsafe(u8"<b onclick=x>1</b><i>2</i>",
                       UnsafeWithConfig({.RemoveElements = List<html::SanitizerElementNamespace> {u8"i"}}))
        .HasException());
    CHECK(data.Markup() == "<b onclick=\"x\">1</b>");

    auto sanitizer = html::Sanitizer::Create();
    REQUIRE_FALSE(div.SetHTMLUnsafe(u8"<b onclick=x>1</b>", {.Sanitizer = html::SanitizerInput {sanitizer}})
                    .HasException());
    CHECK(data.Markup() == "<b>1</b>");
  }

  TEST_CASE("The HTML insertion methods", "[DOM][Element][DOMParsing]")
  {
    InsertionDocument data;
    auto &div = *data.Div;
    REQUIRE_FALSE(div.SetHTMLUnsafe(u8"<i>mid</i>").HasException());
    auto mid = div.FirstElementChild();

    SECTION("append and prepend")
    {
      REQUIRE_FALSE(div.AppendHTML(u8"<b>end</b>").HasException());
      REQUIRE_FALSE(div.PrependHTMLUnsafe(u8"<u>start</u>").HasException());
      CHECK(data.Markup() == "<u>start</u><i>mid</i><b>end</b>");
    }

    SECTION("before, after and replace with")
    {
      REQUIRE_FALSE(mid->BeforeHTML(u8"<b>1</b>").HasException());
      REQUIRE_FALSE(mid->AfterHTMLUnsafe(u8"<b>3</b>").HasException());
      CHECK(data.Markup() == "<b>1</b><i>mid</i><b>3</b>");
      REQUIRE_FALSE(mid->ReplaceWithHTML(u8"<u>2</u><script></script>").HasException());
      CHECK(data.Markup() == "<b>1</b><u>2</u><b>3</b>");
    }

    SECTION("on character data")
    {
      REQUIRE_FALSE(div.SetHTMLUnsafe(u8"text").HasException());
      auto &text = Downcast<Text>(*div.FirstChild());
      REQUIRE_FALSE(text.BeforeHTML(u8"<b>1</b>").HasException());
      REQUIRE_FALSE(text.ReplaceWithHTMLUnsafe(u8"<i>2</i>").HasException());
      CHECK(data.Markup() == "<b>1</b><i>2</i>");
    }

    SECTION("a node without a parent does nothing")
    {
      auto lone = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      CHECK_FALSE(lone->BeforeHTML(u8"<b></b>").HasException());
      CHECK_FALSE(lone->ParentNode());
    }

    SECTION("parents that can't give a context throw")
    {
      auto isHierarchyError = [](const ExceptionOr<void> &result)
      {
        return result.HasException() && result.GetException().Code() == ExceptionCode::HierarchyRequestError;
      };

      // The html element.
      auto *html = data.Document->DocumentElement().get();
      CHECK(isHierarchyError(html->AppendHTML(u8"<b></b>")));
      CHECK(isHierarchyError(data.Body->BeforeHTML(u8"<b></b>")));

      // The document.
      CHECK(isHierarchyError(html->AfterHTML(u8"<b></b>")));

      // A fragment without a host.
      auto fragment = data.Document->CreateDocumentFragment();
      auto child = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      REQUIRE_FALSE(fragment->AppendChild(*child).HasException());
      CHECK(isHierarchyError(child->ReplaceWithHTML(u8"<b></b>")));

      // A template element (rather than its contents).
      auto templateElement = data.Document->CreateElement(DOMStringAtom(u8"template")).ReleaseValue();
      auto inTemplate = data.Document->CreateElement(DOMStringAtom(u8"span")).ReleaseValue();
      REQUIRE_FALSE(templateElement->AppendChild(*inTemplate).HasException());
      CHECK(isHierarchyError(inTemplate->AfterHTML(u8"<b></b>")));
    }

    SECTION("a child of a template's contents takes the template as its context")
    {
      auto templateElement = data.Document->CreateElement(DOMStringAtom(u8"template")).ReleaseValue();
      REQUIRE_FALSE(templateElement->SetHTMLUnsafe(u8"<span>a</span>").HasException());
      auto &contents = Downcast<html::HTMLTemplateElement>(*templateElement).TemplateContents();
      REQUIRE(contents.Host() == templateElement.get());
      auto span = contents.FirstElementChild();
      REQUIRE_FALSE(span->AfterHTML(u8"<b>b</b>").HasException());
      CHECK(InsertedMarkup(templateElement->InnerHTML()) == "<span>a</span><b>b</b>");
    }

    SECTION("on a shadow root")
    {
      auto shadow = div.AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      REQUIRE_FALSE(shadow->SetHTML(u8"<b>1</b><script></script>").HasException());
      REQUIRE_FALSE(shadow->AppendHTMLUnsafe(u8"<i onclick=x>2</i>").HasException());
      REQUIRE_FALSE(shadow->PrependHTML(u8"<u>0</u>").HasException());
      CHECK(InsertedMarkup(shadow->InnerHTML()) == "<u>0</u><b>1</b><i onclick=\"x\">2</i>");
    }
  }

  TEST_CASE("parseHTML and parseHTMLUnsafe", "[DOM][Document][DOMParsing][Sanitizer]")
  {
    InsertionDocument data;

    auto safe =
      Document::ParseHTML(*data.Document, u8"<title>T</title><p id=a onclick=x>1<script></script></p>");
    REQUIRE_FALSE(safe.HasException());
    auto &document = *safe.Value();
    CHECK(document.Title() == u8"T");
    auto p = document.GetElementById(u8"a");
    CHECK_FALSE(p);
    REQUIRE(document.Body());
    CHECK(InsertedMarkup(document.Body()->InnerHTML()) == "<p>1</p>");

    auto unsafe =
      Document::ParseHTMLUnsafe(*data.Document, u8"<p id=a>1<script></script></p>",
                                {.Sanitizer = html::SanitizerInput {html::SanitizerConfig {
                                   .RemoveElements = List<html::SanitizerElementNamespace> {u8"script"}}}});
    REQUIRE_FALSE(unsafe.HasException());
    REQUIRE(unsafe.Value()->Body());
    CHECK(InsertedMarkup(unsafe.Value()->Body()->InnerHTML()) == "<p id=\"a\">1</p>");
  }
}
