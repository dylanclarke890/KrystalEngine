/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE INC.
 * OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Xml/XMLErrors.hpp"
#include "Krystal.Booey/Dom/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/Dom/Internals/ElementFactory.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include <cstring>

namespace krys::boo::xml
{
  namespace
  {
    constexpr int32 MaxErrors = 25;

    KRYS_NODISCARD Ref<Element> CreateHTMLElement(Document &document, utf8_stringview localName,
                                                  utf8_stringview style = {}) noexcept
    {
      auto element = dom::ElementFactory::Create(
        document, QualifiedName(infra::namespaces::HTML, null, DOMStringAtom(localName)));
      if (!style.empty())
      {
        dom::ElementAlgorithms::SetAttributeValue(*element, u8"style", DOMString(style));
      }
      return element;
    }

    void AppendChild(ContainerNode &parent, Node &child) noexcept
    {
      (void)dom::MutationAlgorithms::Append(child, parent);
    }

    void AppendText(ContainerNode &parent, DOMString text) noexcept
    {
      auto textNode = CreateRef<Text>(parent.NodeDocument(), krys::move(text));
      AppendChild(parent, *textNode);
    }

    KRYS_NODISCARD Ref<Element> CreateXHTMLParserErrorHeader(Document &document,
                                                             DOMString errorMessages) noexcept
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

  XMLErrors::XMLErrors(Document &document) noexcept : _document(CreateWeakPtr(&document))
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
    RefPtr<Document> document = ShareRefPtr(_document.get());
    if (!document)
    {
      return;
    }
    RefPtr<Element> documentElement = document->DocumentElement();
    if (!documentElement)
    {
      auto rootElement = CreateHTMLElement(*document, u8"html");
      auto body = CreateHTMLElement(*document, u8"body");
      AppendChild(*rootElement, *body);
      AppendChild(*document, *rootElement);
      documentElement = krys::move(body);
    }
    else if (documentElement->NamespaceURI() == infra::namespaces::SVG)
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

    if (RefPtr<Node> firstChild = ShareRefPtr(documentElement->FirstChild()))
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
