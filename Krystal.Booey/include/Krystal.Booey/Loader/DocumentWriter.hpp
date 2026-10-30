#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLDocumentParser;
}

namespace krys::boo::xml
{
  class XMLDocumentParser;
}

namespace krys::boo
{
  class LocalFrame;

  /// @brief Writes a loaded document into its frame: makes the document (and its window), gives it to the
  /// frame, and feeds its bytes to the parser.
  // FIXME: Encodings other than UTF-8 (a TextResourceDecoder), text, image and media documents, once there
  // are those.
  class DocumentWriter
  {
    KRYS_NON_COPY_MOVABLE(DocumentWriter);

  public:
    enum class State : uint8
    {
      NotStarted,
      Started,
      Finished,
    };

  private:
    WeakPtr<LocalFrame> _frame;
    RefPtr<html::HTMLDocumentParser> _parser;
    RefPtr<xml::XMLDocumentParser> _xmlParser;
    utf8_string _mimeType;
    State _state {State::NotStarted};
    bool _hasReceivedSomeData {false};
    /// @brief The bytes of a UTF-8 sequence split between two chunks.
    utf8_string _pendingBytes;

  public:
    DocumentWriter() noexcept;
    ~DocumentWriter() noexcept;

    void SetFrame(LocalFrame &frame) noexcept;

    void SetMIMEType(utf8_string &&mimeType) noexcept
    {
      _mimeType = krys::move(mimeType);
    }

    /// @brief Makes a document at @p url and makes it the frame's, ready for its data.
    /// @return False if the frame lost its view while the old document unloaded.
    bool Begin(const url::URL &url) noexcept;

    /// @brief Gives @p data, more of the document's bytes, to the parser.
    void AddData(Span<const uint8> data) noexcept;

    /// @brief The document's data is all there: the parser finishes.
    void End() noexcept;

    KRYS_NODISCARD State GetState() const noexcept
    {
      return _state;
    }
  };
}
