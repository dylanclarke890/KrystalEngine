/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/Pasteboard/StaticPasteboard.hpp"
#include <utility>

namespace krys::boo
{
  bool StaticPasteboard::HasData() noexcept
  {
    return _customData.HasData();
  }

  List<utf8_string> StaticPasteboard::TypesSafeForBindings(utf8_stringview) noexcept
  {
    return _customData.OrderedTypes();
  }

  utf8_string StaticPasteboard::ReadString(utf8_stringview type) noexcept
  {
    return _customData.ReadString(type);
  }

  Maybe<utf8_string> StaticPasteboard::ReadStringInCustomData(utf8_stringview type) noexcept
  {
    return _customData.ReadStringInCustomData(type);
  }

  bool StaticPasteboard::HasNonDefaultData() const noexcept
  {
    return !_nonDefaultDataTypes.empty();
  }

  void StaticPasteboard::WriteString(utf8_stringview type, utf8_stringview value) noexcept
  {
    _nonDefaultDataTypes.emplace(type);
    _customData.WriteString(type, value);
  }

  void StaticPasteboard::WriteData(utf8_stringview type, Ref<SharedBuffer> &&data) noexcept
  {
    _nonDefaultDataTypes.emplace(type);
    _customData.WriteData(type, krys::move(data));
  }

  void StaticPasteboard::WriteStringInCustomData(utf8_stringview type, utf8_stringview value) noexcept
  {
    _nonDefaultDataTypes.emplace(type);
    _customData.WriteStringInCustomData(type, value);
  }

  void StaticPasteboard::Clear() noexcept
  {
    _nonDefaultDataTypes.clear();
    _fileContentState = FileContentState::NoFileOrImageData;
    _customData.Clear();
  }

  void StaticPasteboard::Clear(utf8_stringview type) noexcept
  {
    _nonDefaultDataTypes.erase(utf8_string(type));
    _customData.Clear(type);
  }

  PasteboardCustomData StaticPasteboard::TakeCustomData() noexcept
  {
    return std::exchange(_customData, {});
  }

  void StaticPasteboard::WriteMarkup(utf8_stringview markup) noexcept
  {
    _customData.WriteString(u8"text/html", markup);
  }

  void StaticPasteboard::WritePlainText(utf8_stringview text, SmartReplaceOption) noexcept
  {
    _customData.WriteString(u8"text/plain", text);
  }

  void StaticPasteboard::Write(const PasteboardURL &url) noexcept
  {
    _customData.WriteString(u8"text/uri-list", url.Url.String());
  }

  void StaticPasteboard::Write(const PasteboardImage &image) noexcept
  {
    // FIXME: This should ideally remember the image data, so that when this StaticPasteboard
    // is committed to the native pasteboard, we'll preserve the image as well. For now, stick
    // with our existing behavior, which prevents image data from being copied in the case where
    // any non-default data was written by the page.
    _fileContentState = FileContentState::InMemoryImage;

    if (CanExposeURLToDOMWhenPasteboardContainsFiles(image.Url.Url.String()))
    {
      Write(image.Url);
    }
  }

  void StaticPasteboard::Write(const PasteboardWebContent &content) noexcept
  {
    if (!content.Markup.empty())
    {
      WriteMarkup(content.Markup);
    }

    if (!content.Text.empty())
    {
      WritePlainText(content.Text, SmartReplaceOption::CannotSmartReplace);
    }
  }
}
