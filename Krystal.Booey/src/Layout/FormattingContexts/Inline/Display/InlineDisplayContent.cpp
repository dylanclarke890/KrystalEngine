/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"

namespace krys::boo::inline_display
{
  namespace
  {
    template <typename T>
    void InsertList(List<T> &list, size_t index, List<T> &&other) noexcept
    {
      list.insert(list.begin() + static_cast<ptrdiff_t>(index), std::make_move_iterator(other.begin()),
                  std::make_move_iterator(other.end()));
    }

    template <typename T>
    void RemoveAt(List<T> &list, size_t index, size_t count) noexcept
    {
      auto first = list.begin() + static_cast<ptrdiff_t>(index);
      list.erase(first, first + static_cast<ptrdiff_t>(count));
    }
  }

  void Content::Clear() noexcept
  {
    LineList.clear();
    BoxList.clear();
  }

  void Content::Set(Content &&newContent) noexcept
  {
    LineList = krys::move(newContent.LineList);
    BoxList = krys::move(newContent.BoxList);
  }

  void Content::Append(Content &&newContent) noexcept
  {
    InsertList(LineList, LineList.size(), krys::move(newContent.LineList));
    InsertList(BoxList, BoxList.size(), krys::move(newContent.BoxList));
  }

  void Content::Insert(Content &&newContent, size_t lineIndex, size_t boxIndex) noexcept
  {
    InsertList(LineList, lineIndex, krys::move(newContent.LineList));
    InsertList(BoxList, boxIndex, krys::move(newContent.BoxList));
  }

  void Content::Remove(size_t firstLineIndex, size_t numberOfLines, size_t firstBoxIndex,
                       size_t numberOfBoxes) noexcept
  {
    RemoveAt(LineList, firstLineIndex, numberOfLines);
    RemoveAt(BoxList, firstBoxIndex, numberOfBoxes);
  }
}
