#include "Krystal.Booey/DOM/DocumentParser.hpp"

namespace krys::boo::dom
{
  DocumentParser::DocumentParser(dom::Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  DocumentParser::~DocumentParser() noexcept
  {
    // The parser is expected to be detached before the last reference to it goes.
    krys_debug_assert(!_document);
  }

  void DocumentParser::StartParsing() noexcept
  {
    _state = ParserState::Parsing;
  }

  void DocumentParser::PrepareToStopParsing() noexcept
  {
    krys_debug_assert(_state == ParserState::Parsing);
    _state = ParserState::Stopping;
  }

  void DocumentParser::StopParsing() noexcept
  {
    _state = ParserState::Stopped;
  }

  void DocumentParser::Detach() noexcept
  {
    _state = ParserState::Detached;
    _document = nullptr;
  }
}
