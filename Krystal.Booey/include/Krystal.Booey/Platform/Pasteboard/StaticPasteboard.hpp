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

#pragma once

#include "Krystal.Booey/Platform/Pasteboard/Pasteboard.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  /// @brief A pasteboard in memory, holding what a page writes while copying or starting a drag until it's
  /// committed to the platform pasteboard.
  class StaticPasteboard final : public Pasteboard
  {
  private:
    PasteboardCustomData _customData;
    HashSet<utf8_string> _nonDefaultDataTypes;
    FileContentState _fileContentState {FileContentState::NoFileOrImageData};

  public:
    StaticPasteboard() noexcept = default;

    KRYS_NODISCARD PasteboardCustomData TakeCustomData() noexcept;

    KRYS_NODISCARD bool HasNonDefaultData() const noexcept;
    KRYS_NODISCARD bool IsStatic() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool HasData() noexcept final;
    KRYS_NODISCARD List<utf8_string> TypesSafeForBindings(utf8_stringview origin) noexcept final;
    KRYS_NODISCARD utf8_string ReadOrigin() noexcept final
    {
      return {};
    }
    KRYS_NODISCARD utf8_string ReadString(utf8_stringview type) noexcept final;
    KRYS_NODISCARD Maybe<utf8_string> ReadStringInCustomData(utf8_stringview type) noexcept final;

    void WriteString(utf8_stringview type, utf8_stringview data) noexcept final;
    void WriteData(utf8_stringview type, Ref<SharedBuffer> &&data) noexcept;
    void WriteStringInCustomData(utf8_stringview type, utf8_stringview data) noexcept;
    void Clear() noexcept final;
    void Clear(utf8_stringview type) noexcept final;

    void Read(PasteboardPlainText &,
              PlainTextURLReadingPolicy = PlainTextURLReadingPolicy::AllowURL) noexcept final
    {
    }
    void Read(PasteboardWebContentReader &,
              WebContentReadingPolicy = WebContentReadingPolicy::AnyType) noexcept final
    {
    }
    using Pasteboard::Read;

    void Write(const PasteboardURL &url) noexcept final;
    void Write(const PasteboardImage &image) noexcept final;
    void Write(const PasteboardWebContent &content) noexcept final;
    void WriteMarkup(utf8_stringview markup) noexcept final;
    void WritePlainText(utf8_stringview text, SmartReplaceOption smartReplaceOption) noexcept final;

    void SetDragImage(DragImage, const IntPoint &) noexcept final
    {
    }

    void WriteCustomData(const List<PasteboardCustomData> &) noexcept final
    {
    }

    KRYS_NODISCARD FileContentState GetFileContentState() noexcept final
    {
      return _fileContentState;
    }
    KRYS_NODISCARD bool CanSmartReplaceContents() noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::StaticPasteboard)
  KRYS_NODISCARD static bool IsType(const krys::boo::Pasteboard &pasteboard) noexcept
  {
    return pasteboard.IsStatic();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
