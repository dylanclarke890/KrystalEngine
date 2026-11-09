/*
 * Copyright (C) 2019-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Highlights/HighlightRegistry.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD DOMStringAtom AnnotationHighlightKey() noexcept
    {
      return DOMStringAtom(u8"annotationHighlightKey");
    }
  }

  void HighlightRegistry::Set(DOMStringAtom key, Ref<Highlight> &&value) noexcept
  {
    auto [iterator, isNewEntry] = _map.insert_or_assign(key, krys::move(value));
    if (isNewEntry)
    {
      krys_debug_assert(std::ranges::find(_highlightNames, key) == _highlightNames.end());
      _highlightNames.push_back(key);
    }
  }

  Highlight *HighlightRegistry::Get(DOMStringAtom key) const noexcept
  {
    auto found = _map.find(key);
    return found == _map.end() ? nullptr : found->second.get();
  }

  void HighlightRegistry::Clear() noexcept
  {
    _highlightNames.clear();
    auto map = std::exchange(_map, {});
    for (auto &[name, highlight] : map)
    {
      highlight->Clear();
    }
  }

  bool HighlightRegistry::Delete(DOMStringAtom key) noexcept
  {
    if (auto found = std::ranges::find(_highlightNames, key); found != _highlightNames.end())
    {
      _highlightNames.erase(found);
    }
    return _map.erase(key) > 0;
  }

  void HighlightRegistry::AddAnnotationHighlightWithRange(Ref<StaticRange> &&value) noexcept
  {
    if (auto *highlight = Get(AnnotationHighlightKey()))
    {
      (void)highlight->Add(*value);
    }
    else
    {
      List<Ref<AbstractRange>> ranges;
      ranges.push_back(krys::move(value));
      Set(AnnotationHighlightKey(), Highlight::Create(krys::move(ranges)));
    }
  }
}
