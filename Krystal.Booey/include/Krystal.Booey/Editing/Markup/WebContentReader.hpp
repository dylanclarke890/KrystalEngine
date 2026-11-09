/*
 * Copyright (C) 2013-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Platform/Pasteboard/Pasteboard.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <utility>

namespace krys::boo::dom
{
  class DocumentFragment;
}

namespace krys::boo
{
  class LocalFrame;
  enum class MSOListQuirks : bool;

  /// @brief Reads a pasteboard's content for a frame's document.
  class FrameWebContentReader : public PasteboardWebContentReader
  {
  private:
    WeakPtr<LocalFrame> _frame;

  public:
    explicit FrameWebContentReader(LocalFrame &frame) noexcept;

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return *_frame;
    }

  protected:
    /// @brief Whether the content came from another origin than the frame's document, so its markup must be
    /// sanitised.
    KRYS_NODISCARD bool ShouldSanitize() const noexcept;
    KRYS_NODISCARD MSOListQuirks MSOListQuirksForMarkup() const noexcept;
  };

  /// @brief Reads a pasteboard's richest content as a fragment to paste at a range.
  // FIXME: Images (readImage), with blob URLs.
  class WebContentReader final : public FrameWebContentReader
  {
  private:
    const dom::SimpleRange _context;
    const bool _allowPlainText;

    RefPtr<dom::DocumentFragment> _fragment;
    bool _madeFragmentFromPlainText {false};

  public:
    WebContentReader(LocalFrame &frame, const dom::SimpleRange &context, bool allowPlainText) noexcept;
    ~WebContentReader() noexcept;

    void AddFragment(Ref<dom::DocumentFragment> &&fragment) noexcept;
    KRYS_NODISCARD RefPtr<dom::DocumentFragment> TakeFragment() noexcept;
    KRYS_NODISCARD dom::DocumentFragment *Fragment() const noexcept
    {
      return _fragment.get();
    }

    KRYS_NODISCARD bool MadeFragmentFromPlainText() const noexcept
    {
      return _madeFragmentFromPlainText;
    }

  private:
    bool ReadFilePath(utf8_stringview path, utf8_stringview contentType = {}) noexcept override;
    bool ReadFilePaths(const List<utf8_string> &paths) noexcept override;
    bool ReadHTML(utf8_stringview markup) noexcept override;
    bool ReadURL(const url::URL &url, utf8_stringview title) noexcept override;
    bool ReadPlainText(utf8_stringview text) noexcept override;
  };

  /// @brief Reads a pasteboard's markup, sanitised if it came from another origin.
  class WebContentMarkupReader final : public FrameWebContentReader
  {
  private:
    utf8_string _markup;

  public:
    explicit WebContentMarkupReader(LocalFrame &frame) noexcept : FrameWebContentReader(frame)
    {
    }

    KRYS_NODISCARD utf8_string TakeMarkup() noexcept
    {
      return std::exchange(_markup, {});
    }

  private:
    bool ReadFilePath(utf8_stringview, utf8_stringview = {}) noexcept override
    {
      return false;
    }
    bool ReadFilePaths(const List<utf8_string> &) noexcept override
    {
      return false;
    }
    bool ReadHTML(utf8_stringview markup) noexcept override;
    bool ReadURL(const url::URL &, utf8_stringview) noexcept override
    {
      return false;
    }
    bool ReadPlainText(utf8_stringview) noexcept override
    {
      return false;
    }
  };
}
