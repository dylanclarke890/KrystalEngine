#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::boo::dom
{
  /// @brief A parser that builds a document (or a fragment of one) from its source: the XML parser.
  /// @note It goes from parsing to stopping (the source has ended) to stopped, and is detached from its
  /// document last, which it must be before it's destroyed.
  class DocumentParser : public RefCounted<DocumentParser>, public CanMakeWeakPtr<DocumentParser>
  {
    KRYS_NON_COPY_MOVABLE(DocumentParser);

  private:
    enum class ParserState : uint8
    {
      Parsing,
      Stopping,
      Stopped,
      Detached,
    };

    ParserState _state {ParserState::Parsing};
    /// @brief The document being built; null once the parser is detached.
    WeakPtr<Document> _document;

  public:
    virtual ~DocumentParser() noexcept;

    /// @brief Parses @p source, the next part of the source.
    virtual void Append(DOMString source) noexcept = 0;

    /// @brief Tells the parser there's no more source, so it finishes the document.
    virtual void Finish() noexcept = 0;

    /// @brief The document being built; only while the parser is attached.
    KRYS_NODISCARD boo::Document *Document() const noexcept
    {
      krys_debug_assert(_document);
      return _document.get();
    }

    /// @brief Whether the parser is still taking source.
    KRYS_NODISCARD bool IsParsing() const noexcept
    {
      return _state == ParserState::Parsing;
    }

    /// @brief Whether the parser has stopped (or been detached), so it makes no more nodes.
    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _state >= ParserState::Stopped;
    }

    /// @brief Whether the parser has let go of its document.
    KRYS_NODISCARD bool IsDetached() const noexcept
    {
      return _state == ParserState::Detached;
    }

    /// @brief Puts the parser back to parsing, as it starts.
    virtual void StartParsing() noexcept;

    /// @brief The source has ended: the parser stops once it has parsed what it has.
    virtual void PrepareToStopParsing() noexcept;

    /// @brief Stops parsing at once, as on a fatal error in the source; the parser keeps its document.
    virtual void StopParsing() noexcept;

    /// @brief Lets go of the document. A parse in progress unwinds without making more nodes.
    virtual void Detach() noexcept;

  protected:
    explicit DocumentParser(boo::Document &document) noexcept;
  };
}
