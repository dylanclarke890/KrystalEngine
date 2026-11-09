/*
 * Copyright (C) 2010. Adam Barth. All rights reserved.
 * Copyright (C) 2016-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/Text/EncodingRegistry.hpp"
#include "Krystal.Booey/XML/XMLDocumentParser.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief The length of the UTF-8 sequence @p leadByte starts; 1 for a byte that can't start one.
    KRYS_NODISCARD size_t SequenceLength(char8 leadByte) noexcept
    {
      auto byte = static_cast<uint8>(leadByte);
      if (byte >= 0xF0 && byte <= 0xF4)
      {
        return 4;
      }
      if (byte >= 0xE0)
      {
        return 3;
      }
      if (byte >= 0xC2)
      {
        return 2;
      }
      return 1;
    }

    /// @brief @p text with each byte that isn't part of well-formed UTF-8 replaced by U+FFFD.
    KRYS_NODISCARD utf8_string ReplaceInvalidUTF8(utf8_string text) noexcept
    {
      if (text::IsValidUTF8(text))
      {
        return krys::move(text);
      }
      StringBuilder builder;
      for (text::CodePointIterator it(Span<const char8>(text.data(), text.size())); !it.AtEnd(); ++it)
      {
        builder.Append(*it);
      }
      return krys::move(builder).ToString();
    }
  }

  namespace
  {
    /// @brief Whether a response of the MIME type @p mimeType (an essence) is loaded as a text document: a
    /// JavaScript MIME type, a JSON MIME type that isn't an XML MIME type, `text/css`, `text/plain` or
    /// `text/vtt`.
    /// @see https://html.spec.whatwg.org/#navigate-text
    KRYS_NODISCARD bool IsTextDocumentMIMEType(utf8_stringview mimeType) noexcept
    {
      // https://mimesniff.spec.whatwg.org/#javascript-mime-type
      static constexpr utf8_stringview TextTypes[] = {u8"application/ecmascript",
                                                      u8"application/javascript",
                                                      u8"application/x-ecmascript",
                                                      u8"application/x-javascript",
                                                      u8"text/ecmascript",
                                                      u8"text/javascript",
                                                      u8"text/javascript1.0",
                                                      u8"text/javascript1.1",
                                                      u8"text/javascript1.2",
                                                      u8"text/javascript1.3",
                                                      u8"text/javascript1.4",
                                                      u8"text/javascript1.5",
                                                      u8"text/jscript",
                                                      u8"text/livescript",
                                                      u8"text/x-ecmascript",
                                                      u8"text/x-javascript",
                                                      u8"application/json",
                                                      u8"text/json",
                                                      u8"text/css",
                                                      u8"text/plain",
                                                      u8"text/vtt"};
      for (auto type : TextTypes)
      {
        if (text::IsASCIICaselessEqual(mimeType, type))
        {
          return true;
        }
      }
      // https://mimesniff.spec.whatwg.org/#json-mime-type: a subtype that ends in "+json".
      auto slash = mimeType.find(u8'/');
      return slash != utf8_stringview::npos && mimeType.size() - slash > 5
             && text::IsASCIICaselessEqual(mimeType.substr(mimeType.size() - 5), utf8_stringview(u8"+json"));
    }
  }

  DocumentWriter::DocumentWriter() noexcept = default;
  DocumentWriter::~DocumentWriter() noexcept
  {
    // A parser is detached before its last reference goes.
    if (RefPtr<xml::XMLDocumentParser> parser = krys::move(_xmlParser))
    {
      parser->Detach();
    }
  }

  void DocumentWriter::SetFrame(LocalFrame &frame) noexcept
  {
    _frame = CreateWeakPtr(&frame);
  }

  bool DocumentWriter::Begin(const url::URL &url) noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get());
    if (!frame)
    {
      return false;
    }

    // Create a new document before clearing the frame.
    auto document = dom::DOMImplementation::CreateDocument(_mimeType, url);
    // A document created by navigation allows declarative shadow roots.
    document->SetAllowDeclarativeShadowRoots(true);
    // The writer decodes UTF-8 only.
    document->SetEncoding(*EncodingRegistry().Find(u8"utf-8"));

    if (RefPtr<xml::XMLDocumentParser> parser = krys::move(_xmlParser))
    {
      parser->Detach();
    }
    _pendingBytes.clear();
    _hasReceivedSomeData = false;

    // The view starts over for the new document, its scrollbars staying as they are until its first layout
    // (FrameLoader::clear's LocalFrameView::clear).
    if (auto *view = frame->View())
    {
      view->Clear();
    }

    // Showing the document in the frame makes its window; the host then sets up the window before the
    // document is parsed (its custom element definitions, say).
    // The previous document is kept alive too, so the new window can take over its navigation entries.
    frame->SetDocument(ShareRefPtr<dom::Document>(document.get()));
    if (frame->Document() != document.get())
    {
      return false;
    }
    frame->Loader().DidBeginDocument();

    // The document is loading until its parser finishes.
    document->SetParsing(true);
    dom::DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Loading);
    if (auto *htmlDocument = DynamicDowncast<dom::HTMLDocument>(document.get()))
    {
      _parser = CreateRef<html::DocumentParser>(*htmlDocument);
      if (IsTextDocumentMIMEType(_mimeType))
      {
        _parser->StartPlainText();
      }
    }
    else
    {
      _xmlParser = xml::XMLDocumentParser::Create(*document);
    }
    _state = State::Started;
    return true;
  }

  void DocumentWriter::AddData(Span<const uint8> data) noexcept
  {
    krys_assert(_state != State::NotStarted);
    if (_state == State::Finished || (!_parser && !_xmlParser))
    {
      return;
    }

    utf8_string chunk = krys::move(_pendingBytes);
    chunk.append(reinterpret_cast<const char8 *>(data.data()), data.size());
    _pendingBytes.clear();

    // Decoding UTF-8 removes its byte order mark, which would otherwise be text in the document.
    if (!_hasReceivedSomeData)
    {
      if (chunk.size() < 3 && utf8_stringview(u8"\uFEFF").starts_with(chunk))
      {
        _pendingBytes = krys::move(chunk);
        return;
      }
      if (chunk.starts_with(u8"\uFEFF"))
      {
        chunk.erase(0, 3);
      }
      _hasReceivedSomeData = true;
    }

    // Keep a sequence the chunk ends in the middle of for the next one.
    size_t start = chunk.size();
    while (start > 0 && chunk.size() - start < 4 && (static_cast<uint8>(chunk[start - 1]) & 0xC0) == 0x80)
    {
      --start;
    }
    if (start > 0)
    {
      auto lead = start - 1;
      if (static_cast<uint8>(chunk[lead]) >= 0xC2 && lead + SequenceLength(chunk[lead]) > chunk.size())
      {
        _pendingBytes = chunk.substr(lead);
        chunk.erase(lead);
      }
    }

    if (RefPtr<xml::XMLDocumentParser> xmlParser = _xmlParser)
    {
      xmlParser->Append(ReplaceInvalidUTF8(krys::move(chunk)));
      return;
    }
    _parser->InputStream().Append(ReplaceInvalidUTF8(krys::move(chunk)));
    _parser->PumpTokenizer();
  }

  void DocumentWriter::End() noexcept
  {
    // The parser is released after this point. Begin() would have to be called again before we can start
    // writing more data.
    _state = State::Finished;

    RefPtr<LocalFrame> protectedFrame = ShareRefPtr(_frame.get());
    if (RefPtr<xml::XMLDocumentParser> xmlParser = krys::move(_xmlParser))
    {
      _hasReceivedSomeData = true;
      xmlParser->Append(ReplaceInvalidUTF8(krys::move(_pendingBytes)));
      _pendingBytes.clear();
      xmlParser->Finish();
      xmlParser->Detach();
    }
    else
    {
      RefPtr<html::DocumentParser> parser = krys::move(_parser);
      if (!parser)
      {
        return;
      }

      _hasReceivedSomeData = true;
      parser->InputStream().Append(ReplaceInvalidUTF8(krys::move(_pendingBytes)), IsEOF(true));
      _pendingBytes.clear();
      parser->PumpTokenizer();
    }

    if (protectedFrame)
    {
      if (auto *document = protectedFrame->Document())
      {
        document->StyleScope().FlushPendingUpdate();
      }
    }
  }
}
