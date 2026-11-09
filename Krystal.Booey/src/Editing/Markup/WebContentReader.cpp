/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Markup/WebContentReader.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Editing/Markup/Markup.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo
{
  FrameWebContentReader::FrameWebContentReader(LocalFrame &frame) noexcept : _frame(CreateWeakPtr(&frame))
  {
  }

  bool FrameWebContentReader::ShouldSanitize() const noexcept
  {
    auto *document = _frame->Document();
    krys_debug_assert(document);
    return document->OriginIdentifierForPasteboard() != ContentOrigin();
  }

  MSOListQuirks FrameWebContentReader::MSOListQuirksForMarkup() const noexcept
  {
    return ContentOrigin().empty() ? MSOListQuirks::CheckIfNeeded : MSOListQuirks::Disabled;
  }

  WebContentReader::WebContentReader(LocalFrame &frame, const dom::SimpleRange &context,
                                     bool allowPlainText) noexcept
      : FrameWebContentReader(frame), _context(context), _allowPlainText(allowPlainText)
  {
  }

  WebContentReader::~WebContentReader() noexcept = default;

  void WebContentReader::AddFragment(Ref<dom::DocumentFragment> &&newFragment) noexcept
  {
    if (!_fragment)
    {
      _fragment = krys::move(newFragment);
    }
    else
    {
      (void)_fragment->AppendChild(*newFragment);
    }
  }

  RefPtr<dom::DocumentFragment> WebContentReader::TakeFragment() noexcept
  {
    return std::exchange(_fragment, nullptr);
  }

  bool WebContentReader::ReadFilePaths(const List<utf8_string> &paths) noexcept
  {
    if (paths.empty() || !Frame().Document())
    {
      return false;
    }

    for (auto &path : paths)
    {
      ReadFilePath(path);
    }

    return true;
  }

  bool WebContentReader::ReadFilePath(utf8_stringview path, utf8_stringview) noexcept
  {
    auto *document = Frame().Document();
    if (path.empty() || !document)
    {
      return false;
    }

    auto markup = URLToMarkup(url::URL::FileURLWithFileSystemPath(path).String(), path);
    AddFragment(CreateFragmentFromMarkup(*document, markup, u8"file://"));

    return true;
  }

  // FIXME: Preferring images' MIME types to their markup (preferMIMETypeForImages), with images on the
  // pasteboard.
  bool WebContentReader::ReadHTML(utf8_stringview string) noexcept
  {
    auto *document = Frame().Document();
    if (!document)
    {
      return false;
    }

    AddFragment(CreateFragmentFromMarkup(*document, string, {}));
    return true;
  }

  bool WebContentReader::ReadPlainText(utf8_stringview text) noexcept
  {
    if (!_allowPlainText)
    {
      return false;
    }

    AddFragment(CreateFragmentFromText(_context, text));

    _madeFragmentFromPlainText = true;
    return true;
  }

  bool WebContentReader::ReadURL(const url::URL &, utf8_stringview) noexcept
  {
    return false;
  }

  // FIXME: Removing the attributes of subresource URLs that aren't HTTP or data URLs
  // (removeSubresourceURLAttributes), with URL attributes.
  bool WebContentMarkupReader::ReadHTML(utf8_stringview string) noexcept
  {
    auto *document = Frame().Document();
    if (!document)
    {
      return false;
    }

    if (ShouldSanitize())
    {
      _markup = SanitizeMarkup(string, document, MSOListQuirks::Disabled).value_or(utf8_string());
    }
    else
    {
      _markup = utf8_string(string);
    }

    return !_markup.empty();
  }
}
