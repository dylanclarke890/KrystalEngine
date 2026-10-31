#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Core/EncodingRegistry.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
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
    KRYS_NODISCARD utf8_string ReplaceInvalidUTF8(utf8_string &&text) noexcept
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
    // FIXME: Reusing the initial empty document's window for a document of the same origin
    // (takeDOMWindowFrom), once there are origins.
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

    // Showing the document in the frame makes its window; the host then sets up the window before the
    // document is parsed (its custom element definitions, say).
    // The previous document is kept alive too, so the new window can take over its navigation entries.
    RefPtr<dom::Document> previousDocument = ShareRefPtr(frame->Document());
    RefPtr<LocalDOMWindow> previousWindow =
      previousDocument ? ShareRefPtr(previousDocument->Window()) : nullptr;
    frame->SetDocument(ShareRefPtr<dom::Document>(document.get()));
    if (frame->Document() != document.get())
    {
      return false;
    }
    frame->Loader().DidBeginDocument(previousWindow.get());

    // The document is loading until its parser finishes.
    document->SetParsing(true);
    dom::DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Loading);
    if (auto *htmlDocument = DynamicDowncast<dom::HTMLDocument>(document.get()))
    {
      _parser = CreateRef<html::HTMLDocumentParser>(*htmlDocument);
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
      RefPtr<html::HTMLDocumentParser> parser = krys::move(_parser);
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
