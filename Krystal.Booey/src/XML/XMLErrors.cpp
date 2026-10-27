#include "Krystal.Booey/XML/XMLErrors.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include <cstring>

namespace krys::boo::xml
{
  namespace
  {
    constexpr int32 MaxErrors = 25;

    KRYS_NODISCARD Ref<dom::Element> CreateHTMLElement(dom::Document &document, utf8_stringview localName,
                                                       utf8_stringview style = {}) noexcept
    {
      auto element = dom::ElementFactory::Create(
        document, dom::QualifiedName(dom::DOMStringAtom(infra::Namespaces::HTML), dom::DOMStringAtom::Null(),
                                     dom::DOMStringAtom(localName)));
      if (!style.empty())
      {
        dom::ElementAlgorithms::SetAttributeValue(*element, dom::DOMStringAtom(u8"style"),
                                                  dom::DOMString(style));
      }
      return element;
    }

    void AppendChild(dom::ContainerNode &parent, dom::Node &child) noexcept
    {
      (void)dom::MutationAlgorithms::Append(child, parent);
    }

    void AppendText(dom::ContainerNode &parent, dom::DOMString &&text) noexcept
    {
      auto textNode = CreateRef<dom::Text>(parent.NodeDocument(), krys::move(text));
      AppendChild(parent, *textNode);
    }

    KRYS_NODISCARD Ref<dom::Element> CreateXHTMLParserErrorHeader(dom::Document &document,
                                                                  dom::DOMString &&errorMessages) noexcept
    {
      auto reportElement = CreateHTMLElement(document, u8"parsererror",
                                             u8"display: block; white-space: pre; border: 2px solid #c77; "
                                             u8"padding: 0 1em 0 1em; margin: 1em; background-color: #fdd; "
                                             u8"color: black");

      auto h3 = CreateHTMLElement(document, u8"h3");
      AppendChild(*reportElement, *h3);
      AppendText(*h3, u8"This page contains the following errors:");

      auto fixed = CreateHTMLElement(document, u8"div", u8"font-family:monospace;font-size:12px");
      AppendChild(*reportElement, *fixed);
      AppendText(*fixed, krys::move(errorMessages));

      h3 = CreateHTMLElement(document, u8"h3");
      AppendChild(*reportElement, *h3);
      AppendText(*h3, u8"Below is a rendering of the page up to the first error.");

      return reportElement;
    }
  }

  XMLErrors::XMLErrors(dom::Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  void XMLErrors::HandleError(Type type, const char *message, SourceLocation position) noexcept
  {
    if (type == Type::Fatal
        || (_errorCount < MaxErrors
            && (!_lastErrorPosition
                || (_lastErrorPosition->Line != position.Line
                    && _lastErrorPosition->Column != position.Column))))
    {
      switch (type)
      {
        case Type::Warning:  AppendErrorMessage(u8"warning", position, message); break;
        case Type::Fatal:
        case Type::NonFatal: AppendErrorMessage(u8"error", position, message);
      }

      _lastErrorPosition = position;
      ++_errorCount;
    }
  }

  void XMLErrors::AppendErrorMessage(utf8_stringview typeString, SourceLocation position,
                                     const char *message) noexcept
  {
    // <typeString> on line <lineNumber> at column <columnNumber>: <message>
    _errorMessages.Append(typeString, u8" on line ", position.Line, u8" at column ", position.Column, u8": ",
                          utf8_stringview(reinterpret_cast<const char8 *>(message), std::strlen(message)));
  }

  void XMLErrors::InsertErrorMessageBlock() noexcept
  {
    // One or more errors occurred during parsing of the code. Display an error block to the user above
    // the normal content (the DOM tree is created manually and includes line/col info regarding
    // where the errors are located)

    // Create elements for display
    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document)
    {
      return;
    }
    RefPtr<dom::Element> documentElement = document->DocumentElement();
    if (!documentElement)
    {
      auto rootElement = CreateHTMLElement(*document, u8"html");
      auto body = CreateHTMLElement(*document, u8"body");
      AppendChild(*rootElement, *body);
      AppendChild(*document, *rootElement);
      documentElement = krys::move(body);
    }
    else if (documentElement->NamespaceURI() == infra::Namespaces::SVG)
    {
      auto rootElement = CreateHTMLElement(*document, u8"html");
      auto head = CreateHTMLElement(*document, u8"head");
      auto style = CreateHTMLElement(*document, u8"style");
      AppendChild(*head, *style);
      AppendText(*style, u8"html, body { height: 100% } parsererror + svg { width: 100%; height: 100% }");
      AppendChild(*rootElement, *head);
      auto body = CreateHTMLElement(*document, u8"body");
      AppendChild(*rootElement, *body);

      documentElement->Remove();
      if (!documentElement->ParentNode())
      {
        AppendChild(*body, *documentElement);
      }

      AppendChild(*document, *rootElement);

      documentElement = krys::move(body);
    }

    auto reportElement = CreateXHTMLParserErrorHeader(*document, krys::move(_errorMessages).ToString());

    if (RefPtr<dom::Node> firstChild = ShareRefPtr(documentElement->FirstChild()))
    {
      (void)dom::MutationAlgorithms::PreInsert(*reportElement, *documentElement, firstChild.get());
    }
    else
    {
      AppendChild(*documentElement, *reportElement);
    }

    document->UpdateStyleIfNeeded();
  }
}
