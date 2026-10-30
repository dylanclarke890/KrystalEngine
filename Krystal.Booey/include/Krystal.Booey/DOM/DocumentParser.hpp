#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A parser that builds a document (or a fragment of one) from its source.
  class DocumentParser : public RefCounted<DocumentParser>, public CanMakeWeakPtr<DocumentParser>
  {
    KRYS_NON_COPY_MOVABLE(DocumentParser);

  private:
    enum class ParserState : uint8
    {
      Parsing,
      Stopping,
      Stopped,
      Detached
    };

    ParserState _state {ParserState::Parsing};
    // Every DocumentParser needs a pointer back to the document.
    // _document will be null after the parser is detached.
    WeakPtr<Document> _document;

  public:
    virtual ~DocumentParser() noexcept;

    virtual void Append(DOMString &&source) noexcept = 0;
    virtual void Finish() noexcept = 0;

    // Document() will return null after Detach() is called.
    KRYS_NODISCARD dom::Document *Document() const noexcept
    {
      krys_debug_assert(_document);
      return _document.get();
    }

    KRYS_NODISCARD bool IsParsing() const noexcept
    {
      return _state == ParserState::Parsing;
    }
    KRYS_NODISCARD bool IsStopping() const noexcept
    {
      return _state == ParserState::Stopping;
    }
    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _state >= ParserState::Stopped;
    }
    KRYS_NODISCARD bool IsDetached() const noexcept
    {
      return _state == ParserState::Detached;
    }

    virtual void StartParsing() noexcept;

    // PrepareToStopParsing() is used when the end of the input is reached and parsing is to be stopped
    // normally.
    virtual void PrepareToStopParsing() noexcept;

    // StopParsing() is used when a load is canceled/stopped.
    // StopParsing() is currently different from Detach(), but shouldn't be.
    // It should NOT be ok to call any methods on DocumentParser after either Detach() or StopParsing() but
    // right now only Detach() will assert.
    virtual void StopParsing() noexcept;

    // The parser is expected to be detached before the last reference to it goes.
    // After detach, _document is cleared. The parser will unwind its callstacks, but not produce any more
    // nodes.
    virtual void Detach() noexcept;

  protected:
    explicit DocumentParser(dom::Document &document) noexcept;
  };
}
