/*
 * Copyright (C) 2010. Adam Barth. All rights reserved.
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
 * 3.  Neither the name of Adam Barth. ("Adam Barth") nor the names of
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

#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class DocumentParser;
  class ImageDocument;
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
  // FIXME: Encodings other than UTF-8 (a TextResourceDecoder), and media documents, once there are media
  // elements.
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
    RefPtr<html::DocumentParser> _parser;
    RefPtr<xml::XMLDocumentParser> _xmlParser;
    /// @brief The image document being written, which takes the bytes as they are rather than a parser.
    RefPtr<html::ImageDocument> _imageDocument;
    List<uint8> _imageData;
    utf8_string _mimeType;
    State _state {State::NotStarted};
    bool _hasReceivedSomeData {false};
    /// @brief The bytes of a UTF-8 sequence split between two chunks.
    utf8_string _pendingBytes;

  public:
    DocumentWriter() noexcept;
    ~DocumentWriter() noexcept;

    void SetFrame(LocalFrame &frame) noexcept;

    void SetMIMEType(utf8_string mimeType) noexcept
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
